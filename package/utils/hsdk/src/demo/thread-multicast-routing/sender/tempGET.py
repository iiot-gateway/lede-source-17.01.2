'''
Created on 08-09-2012

@author: Maciej Wasilak
'''

import socket
import sys

from twisted.internet.defer import Deferred
from twisted.internet.protocol import DatagramProtocol
from twisted.internet import reactor, task
from twisted.python import log

import txthings.coap as coap
import txthings.resource as resource

from ipaddress import ip_address


class Agent():

    def __init__(self, protocol):
        self.protocol = protocol

    def requestResource(self):
        request = coap.Message(code=coap.GET)
        request.opt.uri_path = ('temp',)
        request.opt.observe = 0
        request.remote = (ip_address("ff03::3ead"), coap.COAP_PORT)
        sock = self.protocol.transport.getHandle()
        # increase TTL
        sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_MULTICAST_HOPS, 42)

        d = protocol.request(request, observeCallback=self.printLaterResponse)
        d.addCallback(self.printResponse)
        d.addErrback(self.noResponse)

    def printResponse(self, response):
        print 'First result: ' + response.payload

    def printLaterResponse(self, response):
        print 'Observe result: ' + response.payload

    def noResponse(self, failure):
        print 'Failed to fetch resource:'
        print failure

log.startLogging(sys.stdout)

endpoint = resource.Endpoint(None)
protocol = coap.Coap(endpoint)
client = Agent(protocol)

reactor.listenUDP(61616, protocol, interface="::")
loop = task.LoopingCall(client.requestResource)
loop.start(2.0)
reactor.run()
