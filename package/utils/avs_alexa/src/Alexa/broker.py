"""
 Copyrigh 2017 NXP
"""

import paho.mqtt.client as mqtt

class ControllerBroker:
    def __init__(self, ip='localhost', port=1883):
        self.handlers = {}
        self.client = mqtt.Client()
        self.client.on_connect = self.__onMqttConnect
        self.client.on_message = self.__onMqttMessage
        self.client.connect(ip, port, 60)

    def __onMqttMessage(self, client, userdata, msg):
        self.handlers[msg.topic](msg.payload)

    def __onMqttConnect(self, client, userdata, flags, rc):
        print('Connected to MQTT broker with error code:' + str(rc))

    def pubMessage(self, topic, msg):
        self.client.publish(topic, msg)

    def addHandler(self, topic, callback):
        self.handlers.setdefault(topic, callback)
        self.client.subscribe(topic, qos=0)

    def delHandler(self, topic):
        self.client.unsubscribe(topic)
        self.handlers.pop(topic)

    def loopForever(self):
        self.client.loop_forever()

