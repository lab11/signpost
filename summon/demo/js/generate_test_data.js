// #!/usr/bin/env node

// var mqtt = require('mqtt')

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
	function get_velocity (beg, end) {
		return get_value(beg*1000, end*1000)/1000.0
	}

	m = get_value(0, 1)
	if (m == 1) {
		v = get_velocity(0, 4)
	} else {
		v = 0.0
	}

	return {
		device: 'signpost_microwave_radar',
		motion: m == 1,
		'velocity_m/s': v,
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
		latitude_direction: 'N',
		longitude: get_long(),
		longitude_direction: 'W',
		timestamp: new Date().toISOString(),
		_meta: get_meta()
	}
}

var cont_energy = 0
var mod0_energy = 0
var mod1_energy = 0
var mod2_energy = 0
var mod5_energy = 0
var mod6_energy = 0
var mod7_energy = 0

function generate_status_packet () {
	function increase_mah () {
		return get_value(0, 1000)/500.0
	}
	function enabled () {
		return get_value(0, 100) > 25
	}

	cont_energy += increase_mah()
	mod0_energy += increase_mah()
	mod1_energy += increase_mah()
	mod2_energy += increase_mah()
	mod5_energy += increase_mah()
	mod6_energy += increase_mah()
	mod7_energy += increase_mah()

	return {
		device: 'signpost_status',
		module0_enabled: enabled(),
		module1_enabled: enabled(),
		module2_enabled: enabled(),
		module5_enabled: enabled(),
		module6_enabled: enabled(),
		module7_enabled: enabled(),
		controller_energy_mAh: cont_energy,
		module0_energy_mAh: mod0_energy,
		module1_energy_mAh: mod1_energy,
		module2_energy_mAh: mod2_energy,
		module5_energy_mAh: mod5_energy,
		module6_energy_mAh: mod6_energy,
		module7_energy_mAh: mod7_energy,
		_meta: get_meta()
	}
}

var ucsd_aq_CO2 = 500;
var ucsd_aq_VOC_PID = 4000;
var ucsd_aq_VOC_IAQ = 120;
var ucsd_aq_barometric = 990;
var ucsd_aq_humidity = 40;

function generate_ucsd_aq_packet () {
	ucsd_aq_CO2 += get_value(-20, 20);
	ucsd_aq_VOC_PID += get_value(-50, 50);
	ucsd_aq_VOC_IAQ += get_value(-5, 5);
	ucsd_aq_barometric += get_value(-1, 1);
	ucsd_aq_humidity += get_value(3, 3);

	return {
		device: 'signpost_ucsd_air_quality',
		co2_ppm: ucsd_aq_CO2,
		VOC_PID_ppb: ucsd_aq_VOC_PID,
		VOC_IAQ_ppb: ucsd_aq_VOC_IAQ,
		barometric_millibar: ucsd_aq_barometric,
		humidity_percent: ucsd_aq_humidity,
		_meta: get_meta()
	}
}


function publish (f) {
	var rand = get_value(0, 100)

	// Drop 20% of packets
	if (rand >= 20) {
		var pkt = f()
		document.dispatchEvent(new CustomEvent("data",{detail:pkt}));
		// console.log(pkt)
		// mqtt_client.publish('gateway-data', JSON.stringify(pkt))
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

function status_generate () {
	publish(generate_status_packet)
}

function ucsd_aq_generate () {
	publish(generate_ucsd_aq_packet)
}

// var mqtt_client = mqtt.connect('mqtt://localhost')



// Generate each packet at varying rates.
function simulatePackets() {
	setInterval(audio_frequency_generate, 1000)
	setInterval(microwave_radar_generate, 3000)
	setInterval(ambient_generate, 5000)
	setInterval(spectrum_generate, 2500)
	setInterval(gps_generate, 7000)
	setInterval(status_generate, 10000)
	setInterval(ucsd_aq_generate, 700);
}