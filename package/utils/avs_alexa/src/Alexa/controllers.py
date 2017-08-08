"""
 Copyrigh 2017 NXP
"""
from socket import *
import config
import os
from broker import ControllerBroker

class PowerSwitchController:
    def __init__(self, broker, port=80):
        self.sock = socket(AF_INET, SOCK_DGRAM)
        self.addr = (config.powerIp, port)
        self.sock.settimeout(5)
        self.off = config.offFrame
        self.on = config.onFrame
        self.topic = 'control_power'
        self.broker = broker
        self.broker.addHandler(self.topic, self.switch)

    def __del__(self):
        self.broker.delHandler(self.topic)
        self.sock.close()

    def __switchOn(self):
        self.sock.sendto(self.on, self.addr)
        buf, addr = self.sock.recvfrom(1024)

    def __switchOff(self):
        self.sock.sendto(self.off, self.addr)
        buf, addr = self.sock.recvfrom(1024)

    def switch(self, action):
        try:
            if action == 'on':
                self.__switchOn()
            elif action == 'off':
                self.__switchOff()
            self.broker.pubMessage("power_state", action)
        except Exception,e:
            print 'rcv:Switch power to',action,'failed:',e

class FanController:
    def __init__(self):
        self.topic = 'my_exam_080028572e4f'

    def __del__(self):
        self.client.disconnect()

    def setSpeed(self, percentage):
        payload = 'slider,%d' % percentage
        mqttClient.publish(self.topic, payload, qos=0, retain=False)

    def onStateChange(self, state):
        print("Set fan to " + state)

class DimmableLightController:
    def __init__(self, broker):
        self.broker = broker
        self.topic = 'control_light'
        self.broker.addHandler(self.topic, self.switch)

    def __del__(self):
        self.broker.delHandler(self.topic)

    def __switchOn(self):
       os.system('iot_snd set ' + config.lightMac + ' 254')

    def __switchOff(self):
       os.system('iot_snd off ' + config.lightMac)

    def switch(self, action):
        try:
            if action == 'on':
                self.__switchOn()
            elif action == 'off':
                self.__switchOff()
            self.broker.pubMessage("light_state", action)
        except Exception,e:
            print 'rcv:Set light to',action,'failed:',e


class ColorLightController:
    def __init__(self, broker):
        self.broker = broker
        self.topic = 'control_color'
        self.broker.addHandler(self.topic, self.switch)

    def __del__(self):
        self.broker.delHandler(self.topic)

    def __switchOn(self):
        cmd = "coap-client -m post -e on coap://["+config.kw41Ip+"]/led -B 0"
        os.system(cmd)

    def __switchOff(self):
        cmd = "coap-client -m post -e off coap://["+config.kw41Ip+"]/led -B 0"
        os.system(cmd)

    def __switchColor(self, rgb):
        intrgb = int(rgb)
        r = str((intrgb & 0xff0000) >> 16)
        g = str((intrgb & 0x00ff00) >> 8)
        b = str((intrgb & 0x0000ff))
        print("switchColor R:"+r+" G:"+g+" B:"+b)
        cmd = "coap-client -m post -e rgb\ r"+r+"\ g"+g+"\ b"+b+" coap://["+config.kw41Ip+"]/led -B 0"
        os.system(cmd)

    def switch(self, action):
        try:
            if action == 'on':
                self.__switchOn()
            elif action == 'off':
                self.__switchOff()
            elif 'rgb' in action:
                self.__switchColor(action.split(',')[1])
            self.broker.pubMessage("color_state", action)
        except Exception,e:
            print 'rcv:Set color to',action,'failed:',e


controllerBroker = ControllerBroker()
powerController = PowerSwitchController(controllerBroker)
lampController = DimmableLightController(controllerBroker)
colorController = ColorLightController(controllerBroker)

def gyroscopeMsg(msg):
    msglist = msg.split(';')
    slide = int((msglist[1].split(','))[1])
    x = int((msglist[2].split(','))[1])
    y = int((msglist[3].split(','))[1])
    z = int((msglist[4].split(','))[1])
    if (slide > 5):
        os.system('iot_snd set ' + config.lightMac + ' ' + str(slide/2 - 5))
        controllerBroker.pubMessage("light_state", "on")
    if (y < -25 and z < -25):
        powerController.switch('on')
    if (y > 25 and z < -25):
        powerController.switch('off')

def nfcMsg(msg):
    if msg == 'ok':
        os.system('touch /certified;aplay resources/welcom.wav')
    elif msg == 'ng':
        os.system('rm /certified;aplay resources/alarm.wav')

if __name__ == '__main__':
    controllerBroker.addHandler("my_exam_00376d7569bb", gyroscopeMsg)
    controllerBroker.addHandler("nfc_event", nfcMsg)

    while True:
        try:
            print('Listening...')
            controllerBroker.loopForever()
        except Exception as e:
            print("AWS Iot connect failed:" + e.message)
