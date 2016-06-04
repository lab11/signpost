#! /usr/bin/env nodejs

var SerialPort = require("serialport");

var module0_port = new SerialPort("/dev/ttyO1", {baudRate: 230400});
var module1_port = new SerialPort("/dev/ttyO2", {baudRate: 230400});

module0_port.on('data', function(data) {
    console.log('Module0 Data: ' + data);
});


