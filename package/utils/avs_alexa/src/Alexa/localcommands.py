#coding=utf-8


keyWordCommands = {
"打开卧室电灯":[("control_light", "on")],
"关闭卧室电灯":[("control_light", "off")],
"打开客厅电灯":[("control_color", "on")],
"关闭客厅电灯":[("control_color", "off")],
"打开所有电灯":[("control_color", "on"),("control_light", "on")],
"关闭所有电灯":[("control_color", "off"),("control_light", "off")],
"打开空调":[("control_power", "on")],
"关闭空调":[("control_power", "off")],
"打开风扇":[("control_power", "on")],
"关闭风扇":[("control_power", "off")],
"打开总电源":[("control_light", "on"),("control_color", "on"),("control_power", "on")],
"关闭总电源":[("control_light", "off"),("control_color", "off"),("control_power", "off")],
"打开总开关":[("control_light", "on"),("control_color", "on"),("control_power", "on")],
"关闭总开关":[("control_light", "off"),("control_color", "off"),("control_power", "off")],
}


def localcommands(str, localBroker):
    for key in keyWordCommands:
        if key in str:
            for topic,cmd in keyWordCommands[key]:
                print('Publish message %s %s' % (topic, cmd))
                localBroker.pubMessage(topic, cmd)
            return 0
    return -1

if __name__ == '__main__':
    localcommands('关闭风扇')
    localcommands('打开总电源')
