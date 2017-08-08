"""
 Copyrigh 2017 NXP
"""
#!/usr/bin/env python
import os
import json
import time
from abc import ABCMeta, abstractmethod
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTShadowClient
from broker import ControllerBroker

class DeviceShadow:
    __metaclass__ = ABCMeta

    def __init__(self, iot, name, stateDic):
        self.name = name
        self.stateDic = stateDic

        self.shadow = iot.createShadowHandlerWithName(self.name, True)
        self.shadow.shadowRegisterDeltaCallback(self.__shadowDeltaCallback)

    def setState(self, stateDic):
        for key in stateDic.keys():
            if key not in self.stateDic:
                 raise KeyError(key)

        self.shadow.shadowUpdate(json.dumps({
            'state': { 'reported': stateDic, 'desired' : stateDic}
            }), None, 5)

    def __shadowDeltaCallback(self, payload, responseStatus, token):
        newState = json.loads(payload)['state']
        self.controlDevice(newState)

    @abstractmethod
    def controlDevice(self, stateDic):
        pass

    @abstractmethod
    def onStateChange(self, state):
        pass

class AirConditioner(DeviceShadow):
    def __init__(self, iot, broker):
        self.topic = "power_state"
        self.broker = broker 
        self.broker.addHandler(self.topic, self.onStateChange)
        DeviceShadow.__init__(self, iot, "air-conditioner", {'temperature':30, 'power':'on'})

    def controlDevice(self, stateDic):
        if 'temperature' in stateDic:
            print("%s:Set temperature to %s" % (self.name, stateDic['temperature']))
        if 'power' in stateDic:
            print("%s:Set power to %s" % (self.name,stateDic['power']))
            self.broker.pubMessage("control_power", stateDic['power'])

    def onStateChange(self, state):
        if state not in ['on', 'off']:
            raise KeyError(state)
        newState = {'power':state}
        self.setState(newState)

class TableLamp(DeviceShadow):
    def __init__(self, iot, broker):
        self.topic = "light_state"
        self.broker = broker 
        self.broker.addHandler(self.topic, self.onStateChange)
        DeviceShadow.__init__(self, iot, "table-lamp", {'power':'on'})

    def controlDevice(self, stateDic):
        if 'power' in stateDic:
            print("%s:Set power to %s" % (self.name,stateDic['power']))
            self.broker.pubMessage("control_light", stateDic['power'])

    def onStateChange(self, state):
        if state not in ['on', 'off']:
            raise KeyError(state)
        newState = {'power':state}
        self.setState(newState)

class ColorLamp(DeviceShadow):
    def __init__(self, iot, broker):
        self.topic = "color_state"
        self.broker = broker 
        self.broker.addHandler(self.topic, self.onStateChange)
        DeviceShadow.__init__(self, iot, "color-lamp", {'power':'on'})

    def controlDevice(self, stateDic):
        if 'power' in stateDic:
            print("%s:Set power to %s" % (self.name,stateDic['power']))
            self.broker.pubMessage("control_color", stateDic['power'])

    def onStateChange(self, state):
        if state not in ['on', 'off']:
            raise KeyError(state)
        newState = {'power':state}
        self.setState(newState)

def createAWSIoT():
    iot = AWSIoTMQTTShadowClient('AWSHome', useWebsocket=True)
    iot.configureEndpoint('a2t27hxmmljsck.iot.us-east-1.amazonaws.com', 443)
    iot.configureCredentials(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'root-CA.pem'))
    iot.configureConnectDisconnectTimeout(10)  # 10 sec
    iot.configureMQTTOperationTimeout(5)  # 5 sec
    iot.connect()
    return iot

if __name__ == "__main__":
    try:
        print('Connecting to AWS iot cloud...')
        iot = createAWSIoT()
        broker = ControllerBroker()
        airConditioner = AirConditioner(iot, broker)
        colorLamp = ColorLamp(iot, broker)
        tableLamp = TableLamp(iot, broker)

        print('Connect to AWS iot cloud successfully...')
        broker.loopForever()
    except Exception as e:
        print("AWS Iot connect failed:" + e.message)
