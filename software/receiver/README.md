Signpost Data Schemas
=====================


Generate Test Data
------------------

    npm install mqtt
    ./generate_test_data.js

Make sure you have an MQTT broker running on `localhost` (or change the path
in the script).



Schemas
-------


### Common

All data packets include a `_meta` section like the following:

```
{
	"_meta": {
		"received_time": <time in ISO-8601 format>,
		"device_id":     <signpost id>,
		"receiver":      "lora",
		"gateway_id":    <gateway_id>
	}
}
```



### Audio Frequency Module

```
{
	"device": "signpost_audio_frequency",
	"63Hz":    <0 - 4095>,
	"160Hz":   <0 - 4095>,
	"400Hz":   <0 - 4095>,
	"1000Hz":  <0 - 4095>,
	"2500Hz":  <0 - 4095>,
	"6250Hz":  <0 - 4095>,
	"16000Hz": <0 - 4095>
}

```


### Microwave Radar Module

```
{
	"device": "signpost_microwave_radar",
	???
}
```


### Ambient Sensing Module

```
{
	"device": "signpost_ambient",
	"temperature_c":    <float>,
	"humidity":         <float>,
	"light_lux":        <float>,
	"pressure_pascals": <float>
}
```


### 2.4GHz RF Spectrum Sensing Module

```
{
	"device": "signpost_2.4ghz_spectrum",
	"channel_11": <int>,
	"channel_12": <int>,
	"channel_13": <int>,
	"channel_14": <int>,
	"channel_15": <int>,
	"channel_16": <int>,
	"channel_17": <int>,
	"channel_18": <int>,
	"channel_19": <int>,
	"channel_20": <int>,
	"channel_21": <int>,
	"channel_22": <int>,
	"channel_23": <int>,
	"channel_24": <int>,
	"channel_25": <int>,
	"channel_26": <int>
}
```

### GPS Data

```
{
	"device": "signpost_gps",
	"latitude":  <float>,
	"longitude": <float>,
	"timestamp": <ISO time>
}
```
