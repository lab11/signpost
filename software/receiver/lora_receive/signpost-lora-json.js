#!/usr/bin/env node

var iM880 = require('iM880-serial-comm');
var mqtt = require('mqtt');

// set the endpoint ID
const DEVICE_ID = 0x09;
const DEVICE_GROUP = 0x10;
const SERIAL_PORT = '/dev/tty.usbserial-AJ0EB9M7';
const SPREADING_FACTOR = 7;

var mqtt_client = mqtt.connect('mqtt://141.212.11.202');

// call the construction with and endpointID
var device = new iM880(DEVICE_ID, DEVICE_GROUP, SERIAL_PORT, SPREADING_FACTOR);

// wait for config-done message and print endpointID
var msg = new Uint8Array([ 9, 8, 10, 67 ]);

device.on('config-done', function (statusmsg) {
  // Print the result of configuring the dongle
  console.log('Configuration status: ' + statusmsg);
});


function get_meta (src_addr) {
	return {
		received_time: new Date().toISOString(),
		device_id: src_addr,
		receiver: 'lora',
		gateway_id: 'gateway1'
	}
}

function pad (s, len) {
	for (var i=s.length; i<len; i++) {
		s = '0' + s;
	}
	return s;
}

var last_packets = {};

function parse (buf) {
	function check_duplicate (mod, msg_type, seq_no) {
		if (!(mod in last_packets)) {
			last_packets[mod] = {};
		}
		if (!(msg_type in last_packets[mod])) {
			last_packets[mod][msg_type] = seq_no-1;
		}

		var duplicate = true;
		if (seq_no != last_packets[mod][msg_type]) {
			duplicate = false;
		}

		last_packets[mod][msg_type] = seq_no;
		return duplicate;
	}

	// Strip out address
	var addr = '';
	for (var i=0; i<6; i++) {
		addr += pad(buf[i].toString(16), 2);
	}

	// Get the sender module
	var module = buf[6];
	// And the message type
	var message_type = buf[7];
	// And get the sequence number
	var sequence_number = buf[8];

	// TODO
	// DISCARD DUPLICATES BASED ON SEQ NUMBER
	var duplicate = check_duplicate(module, message_type, sequence_number);
	if (duplicate) {
		console.log('[' + module + ':'+ message_type +'] Duplicate (' + sequence_number + ')');
		return undefined;
	}


	if (module == 0x20) {
		// Controller
		if (message_type == 0x01) {
			// Energy
			var energy_module0 = buf.readUInt16BE(9);
			var energy_module1 = buf.readUInt16BE(11);
			var energy_module2 = buf.readUInt16BE(13);
			var energy_controller = buf.readUInt16BE(15);
			var energy_linux   = buf.readUInt16BE(17);
			var energy_module5 = buf.readUInt16BE(19);
			var energy_module6 = buf.readUInt16BE(21);
			var energy_module7 = buf.readUInt16BE(23);

			return {
				device: "signpost_status",
				module0_enabled: true,
				module1_enabled: true,
				module2_enabled: true,
				module5_enabled: true,
				module6_enabled: true,
				module7_enabled: true,
				controller_energy_mAh: energy_controller,
				module0_energy_mAh: energy_module0,
				module1_energy_mAh: energy_module1,
				module2_energy_mAh: energy_module2,
				module5_energy_mAh: energy_module5,
				module6_energy_mAh: energy_module6,
				module7_energy_mAh: energy_module7,
				_meta: get_meta(addr)
			}
		} else if (message_type == 0x02) {
			// GPS
			var day = buf.readUInt8(9);
			var month = buf.readUInt8(10);
			var year = 2000 + buf.readUInt8(11);
			var hours = buf.readUInt8(12);
			var minutes = buf.readUInt8(13);
			var seconds = buf.readUInt8(14);
			var latitude = buf.readUInt32BE(15);
			var longitude = buf.readUInt32BE(19);
			var fix = ['', 'No Fix', '2D', '3D'][buf.readUInt8(23)];
			var satellite_count = buf.readUInt8(24);

			return {
				device: 'signpost_gps',
				latitude: latitude,
				latitude_direction: '???',
				longitude: longitude,
				longitude_direction: '???',
				timestamp: 'fill in',
				_meta: get_meta(addr)
			}
		}
	} else if (module == 0x31) {
		if (message_type == 0x01) {
			var chan11 = buf.readInt8(9);
			var chan12 = buf.readInt8(10);
			var chan13 = buf.readInt8(11);
			var chan14 = buf.readInt8(12);
			var chan15 = buf.readInt8(13);
			var chan16 = buf.readInt8(14);
			var chan17 = buf.readInt8(15);
			var chan18 = buf.readInt8(16);
			var chan19 = buf.readInt8(17);
			var chan20 = buf.readInt8(18);
			var chan21 = buf.readInt8(19);
			var chan22 = buf.readInt8(20);
			var chan23 = buf.readInt8(21);
			var chan24 = buf.readInt8(22);
			var chan25 = buf.readInt8(23);
			var chan26 = buf.readInt8(24);

			return {
				device: 'signpost_2.4ghz_spectrum',
				channel_11: chan11,
				channel_12: chan12,
				channel_13: chan13,
				channel_14: chan14,
				channel_15: chan15,
				channel_16: chan16,
				channel_17: chan17,
				channel_18: chan18,
				channel_19: chan19,
				channel_20: chan20,
				channel_21: chan21,
				channel_22: chan22,
				channel_23: chan23,
				channel_24: chan24,
				channel_25: chan25,
				channel_26: chan26,
				_meta: get_meta(addr)
			}
		}
	} else if (module == 0x34) {
		if (message_type == 0x01) {
			var motion = buf.readInt8(9) > 0;
			var speed = buf.readUInt32BE(10) / 1000.0;

			return {
				device: 'signpost_microwave_radar',
				motion: motion,
				'velocity_m/s': speed,
				_meta: get_meta(addr)
			}
		}
	}
}

// listen for new messages and print them
device.on('rx-msg', function(data) {
	// print rx message without slip encoding or checksum
	var buf = new Buffer(data.payload);
	var pkt = parse(buf);
	if (pkt !== undefined) {
		console.log(pkt);
		mqtt_client.publish('gateway-data', JSON.stringify(pkt));
	}
});
