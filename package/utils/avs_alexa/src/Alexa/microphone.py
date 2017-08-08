#coding=utf-8
"""
 Copyright (c) 2016 Seeed Technology Limited.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 Copyrigh 2017 NXP
"""


import os
import wave
import types
import collections
import random
import string
import logging
from threading import Thread, Event
from pocketsphinx.pocketsphinx import Decoder

try: # Python 2
    import Queue
except: # Python 3
    import queue as Queue

import pyaudio

from vad import vad


logger = logging.getLogger('mic')

class Microphone:
    sample_rate = 16000
    frames_per_buffer = 512
    listening_mask = (1 << 0)
    detecting_mask = (1 << 1)
    recording_mask = (1 << 2)

    def __init__(self, pyaudio_instance=None, quit_event=None, decoder=None, language='zh'):
        self.pyaudio_instance = pyaudio_instance if pyaudio_instance else pyaudio.PyAudio()

        self.device_index = None
        for i in range(self.pyaudio_instance.get_device_count()):
            dev = self.pyaudio_instance.get_device_info_by_index(i)
            name = dev['name'].encode('utf-8')
            if name.lower().find(b'Jabra') >= 0 and dev['maxInputChannels'] > 0:
                logger.info('Use audio device:{}'.format(name))
                self.device_index = i
                break

        self.stream = self.pyaudio_instance.open(
            input=True,
            start=False,
            format=pyaudio.paInt16,
            channels=1,
            rate=self.sample_rate,
            frames_per_buffer=self.frames_per_buffer,
            stream_callback=self._callback,
            input_device_index=self.device_index,
        )

        self.quit_event = quit_event if quit_event else Event()

        self.listen_queue = Queue.Queue()
        self.detect_queue = Queue.Queue()

        self.language = language
        self.decoder = decoder if decoder else self.create_decoder()

        self.status = 0
        self.active = False

        self.listen_history = collections.deque(maxlen=8)

        self.wav = None
        self.record_countdown = None
        self.listen_countdown = [0, 0]
        self.buffers_per_sec = self.sample_rate / self.frames_per_buffer + 1

    def create_decoder(self):
        path = os.path.dirname(os.path.realpath(__file__))
        pocketsphinx_data = os.getenv('POCKETSPHINX_DATA', os.path.join(path, 'modules-' + self.language))
        hmm = os.getenv('POCKETSPHINX_HMM', os.path.join(pocketsphinx_data, 'hmm'))
        dict = os.getenv('POCKETSPHINX_DIC', os.path.join(pocketsphinx_data, 'keywords.dic'))
        kws = os.getenv('POCKETSPHINX_KWS', os.path.join(pocketsphinx_data, 'keywords.kws'))
        lm = os.getenv('POCKETSPHINX_LM', os.path.join(pocketsphinx_data, 'keywords.lm'))
        log = os.getenv('POCKETSPHINX_LOG', os.path.join(pocketsphinx_data, 'log'))

        config = Decoder.default_config()
        config.set_string('-hmm', hmm)
        config.set_string('-lm', lm)
        config.set_string('-dict', dict)
        # config.set_string('-kws', kws)
        # config.set_int('-samprate', SAMPLE_RATE) # uncomment if rate is not 16000. use config.set_float() on ubuntu
        config.set_int('-nfft', 512)
        #config.set_float('-vad_threshold', 2.7)
        config.set_string('-logfn', log)

        return Decoder(config)

    def recognize(self, data):
        self.decoder.start_utt()

        if not data:
            return ''

        if isinstance(data, types.GeneratorType):
            for d in data:
                self.decoder.process_raw(d, False, False)
        else:
            self.decoder.process_raw(data, False, True)

        hypothesis = self.decoder.hyp()
        self.decoder.end_utt()
        if hypothesis:
            logger.info('Recognized {}'.format(hypothesis.hypstr))
            return hypothesis.hypstr

        return ''

    def detect(self, keywords=None, duration=5, timeout=10):
        timeout_count = timeout * self.buffers_per_sec
        duration_count = duration * self.buffers_per_sec
        self.decoder.start_utt()

        self.detect_queue.queue.clear()
        self.status |= self.detecting_mask
        self.stream.start_stream()

        result = None
        self.in_speech = False
        logger.info('Start detecting')
        while not self.quit_event.is_set():
            data = self.detect_queue.get()
            self.decoder.process_raw(data, False, False)
            hypothesis = self.decoder.hyp()

            if keywords and hypothesis:
                res = hypothesis.hypstr
                for key in keywords:
                    if key in res:
                        self.decoder.end_utt()
                        result = key
                        self.status &= ~self.detecting_mask
                        self.stop()
                        return res

            if self.in_speech != self.decoder.get_in_speech():
                self.in_speech = self.decoder.get_in_speech()
                if not self.in_speech and hypothesis:
                    self.decoder.end_utt()
                    result = hypothesis.hypstr
                    break
            else:
                if not self.in_speech:
                    timeout_count -= 1
                else:
                    duration_count -= 1
            if timeout_count <= 0 or duration_count <= 0:
                if hypothesis:
                     result = hypothesis.hypstr
                self.decoder.end_utt()
                logger.info('Detecting timeout {} {}'.format(timeout_count,duration_count))
                break

        self.status &= ~self.detecting_mask
        self.stop()

        return result

    wakeup = detect

    def listen(self, duration=4, timeout=1):
        vad.reset()

        self.listen_countdown[0] = duration * self.buffers_per_sec
        self.listen_countdown[1] = timeout * self.buffers_per_sec

        self.listen_queue.queue.clear()
        self.status |= self.listening_mask
        self.start()

        logger.info('Start listening')
        self.start_speeking = 0

        def _listen():
            try:
                data = self.listen_queue.get(timeout=3)
                while data and not self.quit_event.is_set():
                    yield data
                    data = self.listen_queue.get(timeout=3)
            except Queue.Empty:
                pass

            self.stop()

        return _listen()

    def record(self, file_name, seconds=1800):
        self.wav = wave.open(file_name, 'wb')
        self.wav.setsampwidth(2)
        self.wav.setnchannels(1)
        self.wav.setframerate(self.sample_rate)
        self.record_countdown = seconds * self.buffers_per_sec
        self.status |= self.recording_mask
        self.start()

    def quit(self):
        self.status = 0
        self.quit_event.set()
        self.listen_queue.put('')
        if self.wav:
            self.wav.close()
            self.wav = None

    def start(self):
        if self.stream.is_stopped():
            self.stream.start_stream()

    def stop(self):
        if not self.status and self.stream.is_active():
            self.stream.stop_stream()

    def close(self):
        self.quit()
        self.stream.close()

    def _callback(self, in_data, frame_count, time_info, status):
        if self.status & self.recording_mask:
            pass

        if self.status & self.detecting_mask:
            self.detect_queue.put(in_data)

        if self.status & self.listening_mask:
            active = vad.is_speech(in_data)
            if active:
                if not self.active:
                    for d in self.listen_history:
                        self.listen_queue.put(d)
                        self.listen_countdown[0] -= 1

                    self.listen_history.clear()

                self.listen_queue.put(in_data)
                self.listen_countdown[0] -= 1
                self.start_speeking += 1
            else:
                if self.active:
                    self.listen_queue.put(in_data)
                else:
                    self.listen_history.append(in_data)

                if self.start_speeking >= 5:
                    self.listen_countdown[1] -= 1

            if self.listen_countdown[0] <= 0 or self.listen_countdown[1] <= 0:
                self.listen_queue.put('')
                self.status &= ~self.listening_mask
                logger.info('Stop listening')

            self.active = active

        return None, pyaudio.paContinue


def task(quit_event):
    mic = Microphone(quit_event=quit_event, language='en')
    print("START")
    while not quit_event.is_set():
        text = mic.wakeup(keywords=['ALEXA', 'GREEBLE'])
        if text:
            print('Recognized %s' % text)


def main():
    import time

    logging.basicConfig(level=logging.DEBUG)

    q = Event()
    t = Thread(target=task, args=(q,))
    t.start()
    while True:
        try:
            time.sleep(1)
        except KeyboardInterrupt:
            print('Quit')
            q.set()
            break
    t.join()

if __name__ == '__main__':
    main()
