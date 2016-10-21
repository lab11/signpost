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
	"motion": <boolean>,
	"velocity_m/s": <float>,
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
	"latitude_direction": "N"|"S",
	"longitude": <float>,
	"longitude_direction": "E"|"W",
	"timestamp": <ISO time>
}
```

### Signpost Status

```
{
	"device": "signpost_status",
	"module0_enabled": <boolean>,
	"module1_enabled": <boolean>,
	"module2_enabled": <boolean>,
	"module5_enabled": <boolean>,
	"module6_enabled": <boolean>,
	"module7_enabled": <boolean>,
	"controller_energy_mAh": <float>,
	"module0_energy_mAh": <float>,
	"module1_energy_mAh": <float>,
	"module2_energy_mAh": <float>,
	"module5_energy_mAh": <float>,
	"module6_energy_mAh": <float>,
	"module7_energy_mAh": <float>
}
```

### UCSD Air Quality

```
{
	device:              'signpost_ucsd_air_quality',
	co2_ppm:             <uint>,
	VOC_PID_ppb:         <uint>,
	VOC_IAQ_ppb:         <uint>,
	barometric_millibar: <uint>,
	humidity_percent:    <uint>,
}
```

I2C Message Structure
---------------------

### 2.4GHz RF Spectrum Sensing Module

```
18 bytes:

u8 : 0x31
u8 : 0x01
i8 : Channel 11 RSSI
i8 : Channel 12 RSSI
i8 : Channel 13 RSSI
i8 : Channel 14 RSSI
i8 : Channel 15 RSSI
i8 : Channel 16 RSSI
i8 : Channel 17 RSSI
i8 : Channel 18 RSSI
i8 : Channel 19 RSSI
i8 : Channel 20 RSSI
i8 : Channel 21 RSSI
i8 : Channel 22 RSSI
i8 : Channel 23 RSSI
i8 : Channel 24 RSSI
i8 : Channel 25 RSSI
i8 : Channel 26 RSSI
```

### Ambient Sensing Module

```
10 bytes:

u8  : 0x32
u8  : 0x01
u16 : temperature (1/100 degree c)
u16 : humidity (1/100 %)
u16 : light (lux)
u16 : pressure
```


### Controller


Energy & status
```
19 bytes:

u8  : 0x20
u8  : 0x01
u16 : Module0 Energy (mAh)
u16 : Module1 Energy (mAh)
u16 : Module2 Energy (mAh)
u16 : Controller/Backplane Energy (mAh)
u16 : Linux Energy (mAh)
u16 : Module5 Energy (mAh)
u16 : Module6 Energy (mAh)
u16 : Module7 Energy (mAh)
u1  : Module0 Enabled
u1  : Module1 Enabled
u1  : Module2 Enabled
u1  : reserved
u1  : Linux Enabled
u1  : Module5 Enabled
u1  : Module6 Enabled
u1  : Module7 Enabled
```

GPS
```
18 bytes:

u8  : 0x20
u8  : 0x02
u8  : Day
u8  : Month
u8  : Year (Last two digits)
u8  : Hours
u8  : Minutes
u8  : Seconds
u32 : Latitude
u32 : Longitude
u8  : Fix (1=No Fix, 2=2D, 3=3D)
u8  : Satellite Count (Satellites used in fix)
```


### Microwave Radar Module

```
7 bytes:

u8  : 0x32
u8  : 0x01
u8  : motion since last transmission (boolean)
u32 : max speed measured since last transmission (mm/s)
```

### UCSD Air Quality

```
16 bytes:

u8  : 0x35
u8  : 0x01
u16 : CO2 ppm
u32 : VOC from the PID sensor
u32 : VOC from the IAQ sensor
u16 : Barometric pressure
u16 : Percent Humidity
```

