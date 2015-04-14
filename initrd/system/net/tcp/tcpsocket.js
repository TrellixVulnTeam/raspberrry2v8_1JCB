// Copyright 2014 Runtime.JS project authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

var resources = require('resources.js')();
var netUtils = require('net/utils.js');
var tcpConn = require('net/tcp/tcpconn.js');
var tcp = require('net/tcp/tcp.js');
var intfc = require('interface.js');

"use strict";

/**
 * Maps handle -> socket
 */
var tcpConnections = new Map();

/**
 * Handle pool for connection sockets
 */
var tcpConnectionsSocketPool = intfc.createHandlePool();

/**
 * Note: No listening to IP address support (0.0.0.0 assumed)
 */
var listeningTable = (function() {
  var table = Object.create(null);

  return {
    set: function(port, socket) {
      if (table[port]) {
        throw new Error('ADDR_IN_USE');
      }

      table[port] = socket;
    },
    get: function(port) {
      var socket = table[port];

      if (!socket) {
        return null;
      }

      return socket;
    },
    delete: function(port) {
      delete table[port];
    }
  };
})();

function TCPServerSocket(connPipe) {
  this.connPipe = connPipe;
  this.listeningPort = 0;
  this.isListening = false;
  this.connections = Object.create(null);
}

var pow32 = Math.pow(2, 32);
function connHash(ip, port) {
  return pow32 * port + (((ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | ip[3]) >>> 0);
}

TCPServerSocket.prototype.listen = function(port) {
  if (!(this instanceof TCPServerSocket)) throw new Error('instanceof check failed');

  if (this.isListening) {
    throw new Error('ALREADY_LISTENING');
  }

  this.listeningPort = port;
  this.isListening = true;

  listeningTable.set(port, this);
};

TCPServerSocket.prototype.close = function() {
  if (!(this instanceof TCPServerSocket)) throw new Error('instanceof check failed');

  if (!this.isListening) {
    return;
  }

  listeningTable.delete(this.listeningPort);
  this.isListening = false;
  this.listeningPort = 0;
};

TCPServerSocket.prototype.connGet = function(remoteIP, remotePort) {
  return this.connections[connHash(remoteIP, remotePort)];
};

TCPServerSocket.prototype.connSet = function(remoteIP, remotePort, socket) {
  this.connections[connHash(remoteIP, remotePort)] = socket;
};

TCPServerSocket.prototype.accept = function(intf, remoteIP, remotePort, seqNumber, windowSize) {
  if (!(this instanceof TCPServerSocket)) throw new Error('instanceof check failed');

  if (!this.isListening) {
    return null;
  }

  var socket = new tcpConn.TCPConnectionSocket(intf);
  socket.accept(remoteIP, remotePort, this.listeningPort, seqNumber, windowSize);
  this.connSet(remoteIP, remotePort, socket);
  return socket;
};

function socketInit(connPipe, socket) {
  var writePipe = isolate.createPipe();
  var readPipe = isolate.createPipe();
  var socketHandle = tcpConnectionsSocketPool.createHandle();
  tcpConnections.set(socketHandle, socket);
  socket.writePipe = writePipe;
  socket.readPipe = readPipe;

  function read() {
    writePipe.pull(wpp);
  }

  function wpp(v) {
    if (!v) {
      return socket.close();
    }

    socket.sendData(v);
    read();
  }

  read();
  connPipe.push([socketHandle, writePipe, readPipe]);
}

TCPServerSocket.prototype.recvAccept = function(intf, ip4Header, tcpHeader) {
  var remoteIP = ip4Header.srcIP;
  var remotePort = tcpHeader.srcPort;
  var socket = this.accept(intf, remoteIP, remotePort, tcpHeader.seqNumber, tcpHeader.windowSize);
  if (socket) {
    socketInit(this.connPipe, socket);
  }
}

TCPServerSocket.prototype.recvData = function(ip4Header, tcpHeader, buf, len, dataOffset) {
  var remoteIP = ip4Header.srcIP;
  var remotePort = tcpHeader.srcPort;
  var conn = this.connGet(remoteIP, remotePort);
  if (conn) {
    conn.recv(ip4Header, tcpHeader, buf, len, dataOffset);
  }
}

TCPServerSocket.prototype.recv = function(intf, ip4Header, tcpHeader, buf, len, dataOffset) {
  if (!(this instanceof TCPServerSocket)) throw new Error('instanceof check failed');

  var self = this;

  if (!this.isListening) {
    // TODO: Send reset
    return;
  }

  if (tcpHeader.flags & tcp.flags.SYN) {
    (function() {
      self.recvAccept(intf, ip4Header, tcpHeader);
    })();
  } else {
    (function() {
      self.recvData(ip4Header, tcpHeader, buf, len, dataOffset);
    })();
  }
};

module.exports = {
  TCPServerSocket: TCPServerSocket,
  getListeningSocket: function(port) {
    return listeningTable.get(port);
  },
};
