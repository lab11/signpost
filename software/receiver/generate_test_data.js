#!/usr/bin/env node

var mqtt = require('mqtt')

function get_meta () {
	return {
		received_time: new Date().toISOString(),
		device_id: 'signpost_1',
		receiver: 'lora',
		gateway_id: 'gateway1'
	}
}

function get_value (beg, end) {
	return Math.floor(Math.random()*(end-beg+1)+beg)
}

function generate_audio_frequency_packet () {
	function get_audio_value () {
		return get_value(0, 4095)
	}

	return {
		device: 'signpost_audio_frequency',
		'63Hz':    get_audio_value(),
		'160Hz':   get_audio_value(),
		'400Hz':   get_audio_value(),
		'1000Hz':  get_audio_value(),
		'2500Hz':  get_audio_value(),
		'6250Hz':  get_audio_value(),
		'16000Hz': get_audio_value(),
		_meta: get_meta()
	}
}

function generate_microwave_radar_packet () {
	return {
		device: 'signpost_microwave_radar',
		_meta: get_meta()
	}
}

function generate_ambient_sensing_packet () {
	function get_measurement (beg, end) {
		return get_value(beg*1000, end*1000)/1000.0
	}

	return {
		device: 'signpost_ambient',
		temperature_c: get_measurement(18, 27),
		_meta: get_meta()
	}
}

function generate_spectrum_sensing_packet () {
	function get_measurement (beg, end) {
		return get_value(beg*1000, end*1000)/1000.0
	}

	return {
		device: 'signpost_2.4ghz_spectrum',
		channel_11: get_value(0, -100),
		channel_12: get_value(0, -100),
		channel_13: get_value(0, -100),
		channel_14: get_value(0, -100),
		channel_15: get_value(0, -100),
		channel_16: get_value(0, -100),
		channel_17: get_value(0, -100),
		channel_18: get_value(0, -100),
		channel_19: get_value(0, -100),
		channel_20: get_value(0, -100),
		channel_21: get_value(0, -100),
		channel_22: get_value(0, -100),
		channel_23: get_value(0, -100),
		channel_24: get_value(0, -100),
		channel_25: get_value(0, -100),
		channel_26: get_value(0, -100),
		_meta: get_meta()
	}
}

function generate_gps_packet () {
	function get_lat () {
		return get_value(420000, 425000)/10000.0
	}
	function get_long () {
		return get_value(830000, 842000)/10000.0
	}

	return {
		device: 'signpost_gps',
		latitude: get_lat(),
		longitude: get_long(),
		timestamp: new Date().toISOString(),
		_meta: get_meta()
	}
}


function publish (f) {
	var rand = get_value(0, 100)

	// Drop 20% of packets
	if (rand >= 20) {
		var pkt = f()

		console.log(pkt)
		mqtt_client.publish('gateway-data', JSON.stringify(pkt))
	}
}


function audio_frequency_generate () {
	publish(generate_audio_frequency_packet)
}

function microwave_radar_generate () {
	publish(generate_microwave_radar_packet)
}

function ambient_generate () {
	publish(generate_ambient_sensing_packet)
}

function spectrum_generate () {
	publish(generate_spectrum_sensing_packet)
}

function gps_generate () {
	publish(generate_gps_packet)
}


var mqtt_client = mqtt.connect('mqtt://localhost')



// Generate each packet at varying rates.
setInterval(audio_frequency_generate, 1000)
setInterval(microwave_radar_generate, 3000)
setInterval(ambient_generate, 5000)
setInterval(spectrum_generate, 2500)
setInterval(gps_generate, 7000)
