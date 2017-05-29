/* JavaScript for Signpost Summon UI */

var SIMULATE_PACKETS = false; // if false, uses websocket when opened in a non-Summon browser; else, simulates packets

var MODULES = [
  { name:"2.4GHz Spectrum", dev:"2.4ghz_spectrum", bytes:18, x:[11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26], y:[-128,-60] },
  { name:"Ambient", dev:"ambient", bytes:10 },
  { name:"Radio", dev:"radio_status", bytes:16 },
  { name:"Controller", dev:"gps", bytes:18 },
  { name:"Power Supply", dev:"status", bytes:19 },
  { name:"Audio Frequency", dev:"audio_frequency", bytes:16, x:[63,160,400,1000,2500,6250,16000], y:[0,80] },
  { name:"UCSD Air Quality", dev:"ucsd_air_quality", bytes:16 },
  { name:"Microwave Radar", dev:"microwave_radar", bytes:7 }
]

var app = {
  // Application Constructor
  initialize: function() {
    document.addEventListener("deviceready", app.onAppReady, false);
    document.addEventListener("resume", app.onAppReady, false);
    document.addEventListener("pause", app.onPause, false);
    document.addEventListener("data", app.onData, false);
    document.querySelector("#rect").addEventListener("click", app.onClick);
    for (i=0; i<Math.min(MODULES.length,8); i++) app.createModule(i,MODULES[i].name);
    if (!navigator.userAgent.endsWith("Summon") && typeof gateway == "undefined") {
      if (!SIMULATE_PACKETS) {
        var client = mqtt.connect("ws://signpost.j2x.us:9001/mqtt");
        client.on("connect", function () { client.subscribe('signpost'); });
        client.on("message", function(topic, payload) {
          var data = JSON.parse(payload.toString());
          SIMULATE_PACKETS=true;
          document.dispatchEvent(new CustomEvent("data",{detail:data}));
        });
        setTimeout(function(){if(!SIMULATE_PACKETS)app.simulatePackets()},30000); // simulate if nothing happens in 30s
      } else app.simulatePackets();
    } else app.log("Running Signpost UI on Summon");
  },
  // Module Constructor
  createModule: function(i,name) {
    var svg = d3.select("#mod"+i);
    var module = document.querySelectorAll(".mod")[i];
    var titles = [];
    module.addEventListener("click", app.onClick);
    module.insertBefore(document.createTextNode(name),module.querySelector("svg"));
    switch(name) {
      case "Controller":
        app.projection = d3.geo.equirectangular();
        d3.json("js/map.json",function(e,geodata) {
          if (e) return app.log(e);
          svg.selectAll("path")
            .data(topojson.feature(geodata,geodata.objects.collection).features).enter().append("path")
            .attr("d",d3.geo.path().projection(app.projection.scale(19).translate([120/2,65/2])));
          svg.append("text").text("--.---\xB0 N ---.---\xB0 E").attr({ "x":60, "y":75, "id":"coord" });
          svg.append("text").text("----/--/-- --:--:--.---(UTC)").attr({ "x":60, "y":87, "id":"stamp" });
        });
        break;
      case "2.4GHz Spectrum":
        titles = ["CH","dBm"];
      case "Audio Frequency":
        if (!titles.length) titles = ["Hz","dB"]
        var box = { top:5, right:0, bottom:15, left:20, width:100, height:70 };
        var g = svg.append("g").attr({ "transform":"translate("+box.left+","+box.top+")" });
        app.x[i] = d3.scale.ordinal().rangeRoundBands([0, box.width], 0.1, 0.2).domain(MODULES[i].x);
        app.y[i] = d3.scale.linear().range([box.height, 0]).domain(MODULES[i].y);
        g.append("g").attr({ "class":"x axis", "transform":"translate(0,"+box.height+")" })
          .call(d3.svg.axis().scale(app.x[i]).orient("bottom").tickFormat(d3.format(".2s")));
        g.append("g").attr({ "class":"y axis" })
          .call(d3.svg.axis().scale(app.y[i]).orient("left"));
        svg.append("text").text(titles[1]).attr({ "x":7, "y":83, "style":"font-size:5px" });
        svg.append("text").text(titles[0]).attr({ "x":18, "y":87, "style":"font-size:5px" });
        break;
      case "Microwave Radar":
        svg.append("text").text("0.0").attr({ "x":60, "y":55, "id":"velocity" });
        svg.append("text").text("m/s").attr({ "x":60, "y":75 });
        break;
      case "Ambient":
        svg.append("image").attr({ "x":20, "y":10, "height":20, "width":20, "xlink:href":"img/temperature.svg" });
        svg.append("text").attr({ "x":30, "y":40, "id":"temp"}).text("-- \xB0C");
        svg.append("image").attr({ "x":80, "y":10, "height":20, "width":20, "xlink:href":"img/humidity.svg" });
        svg.append("text").attr({ "x":90, "y":40, "id":"hum"}).text("-- %");
        svg.append("image").attr({ "x":20, "y":55, "height":20, "width":20, "xlink:href":"img/light.svg" });
        svg.append("text").attr({ "x":30, "y":85, "id":"lux"}).text("-- lx");
        svg.append("image").attr({ "x":80, "y":55, "height":20, "width":20, "xlink:href":"img/pressure.svg" });
        svg.append("text").attr({ "x":90, "y":85, "id":"pres"}).text("-- kPa");
        break;
      case "UCSD Air Quality":
        svg.append("text").attr({ "x":60, "y":14, "class":"unit"}).text("CO\u2082");
        svg.append("text").attr({ "x":60, "y":32, "id":"co2"}).text("---");
        svg.append("text").attr({ "x":60, "y":41}).text("ppm");
        svg.append("text").attr({ "x":30, "y":57, "class":"unit"}).text("VOC(PID)");
        svg.append("text").attr({ "x":30, "y":75, "id":"vocp"}).text("---");
        svg.append("text").attr({ "x":30, "y":84}).text("ppb");
        svg.append("text").attr({ "x":90, "y":57, "class":"unit"}).text("VOC(IAQ)");
        svg.append("text").attr({ "x":90, "y":75, "id":"voci"}).text("---");
        svg.append("text").attr({ "x":90, "y":84}).text("ppb");
        break;
      case "Radio":
        var table = svg.append("foreignObject").attr({ "width":"100%", "height":"100%"}).append("xhtml:table");
        table.append("tr").html("<td>RADIO</td><td class='pkt'>PACKETS</td><td class='byt'>BYTES</td>");
        table.append("tr").html("<td>LoRa </td><td class='lora pkt'>0</td><td class='lora byt'>0</td>");
        table.append("tr").html("<td>BLE  </td><td class='ble  pkt'>0</td><td class='ble  byt'>0</td>");
        table.append("tr").html("<td>Cell </td><td class='cell pkt'>0</td><td class='cell byt'>0</td>");
        break;
      case "Power Supply":
        var table = svg.append("foreignObject").attr({ "width":"100%", "height":"100%"}).append("xhtml:table");
        table.append("tr").html("<td>MODULE</td><td>STATE</td><td class='mah'>ENERGY[mAh]</td>");
        for (i=0; i<8; i=[1,2,3,5,8,6,7,8][i])
          table.append("tr").html("<td>"+i+"</td><td class='m"+i+" state'>-</td><td class='m"+i+" mah'>-</td>");
        break;
    }
  },
  // Module Data Updater
  updateModule: function(i,data) {
    var svg = d3.select("#mod"+i);
    var mod = MODULES[i];
    var values = [];
    switch(mod.name) {
      case "Controller":
        var lat = data.latitude * (data.latitude_direction=="N"?1:-1);
        var lon = data.longitude * (data.longitude_direction=="E"?1:-1);
        svg.selectAll("line").remove();
        svg.selectAll("line")
          .data([[[lon,-90],[lon,90]],[[-180,lat],[180,lat]]]).enter().append("line")
          .attr("x1", function(d) { return app.projection(d[0])[0]; })
          .attr("y1", function(d) { return app.projection(d[0])[1]; })
          .attr("x2", function(d) { return app.projection(d[1])[0]; })
          .attr("y2", function(d) { return app.projection(d[1])[1]; });
        svg.select("#coord").text(
          data.latitude.toFixed(3) + "\xB0 " + data.latitude_direction + " " +
          data.longitude.toFixed(3)+ "\xB0 " + data.longitude_direction );
        svg.select("#stamp").text(data.timestamp.replace('T',' ').replace('Z','')+"(UTC)");
        break;
      case "2.4GHz Spectrum":
        for (n in mod.x) values.push({ x:mod.x[n], y:data["channel_"+mod.x[n]] });
      case "Audio Frequency":
        if (!values.length) for (n in mod.x) values.push({ x:mod.x[n], y:data[mod.x[n]+"Hz"] });
        svg.select("g").selectAll(".bar").remove();
        svg.select("g").selectAll(".bar").data(values).enter().append("rect")
          .attr("class", "bar")
          .attr("x", function(d) { return app.x[i](d.x); })
          .attr("width", app.x[i].rangeBand())
          .attr("y", function(d) { return app.y[i](d.y); })
          .attr("height", function(d) { return 70 - app.y[i](d.y); });
        break;
      case "Microwave Radar":
        svg.select("text").text( (data["motion"] * data["velocity_m/s"]).toFixed(1) );
        break;
      case "Ambient":
        svg.select("#temp").text(data.temperature_c.toFixed(1) + " \xB0C");
        svg.select("#hum" ).text(data.humidity.toFixed(1) + " %");
        svg.select("#lux" ).text(data.light_lux.toFixed(1) + " lx");
        svg.select("#pres").text((data.pressure_pascals/1000).toFixed(1) + " kPa");
        break;
      case "UCSD Air Quality":
        svg.select("#co2" ).text(data.co2_ppm);
        svg.select("#vocp").text(data.VOC_PID_ppb);
        svg.select("#voci").text(data.VOC_IAQ_ppb);
        break;
      case "Radio":
        var values = { lora:{pkts:0,byts:0}, ble:{pkts:0,byts:0} }
        for (n in data) {
          var i = MODULES.findIndex(function(x){return x.dev.startsWith(n.split("_")[0].toLowerCase());});
          if (n.endsWith("ble_packets_sent")) {
            values.ble.pkts += data[n];
            values.ble.byts += data[n]*MODULES[i].bytes;
          } else if (n.endsWith("lora_packets_sent")) {
            values.lora.pkts += data[n];
            values.lora.byts += data[n]*MODULES[i].bytes;
          }
        }
        svg.select(".ble.pkt").text(values.ble.pkts);
        svg.select(".ble.byt").text(values.ble.byts.toFixed(0));
        svg.select(".lora.pkt").text(values.lora.pkts);
        svg.select(".lora.byt").text(values.lora.byts.toFixed(0));
        break;
      case "Power Supply":
        for (i=0; i<8; i=[1,2,3,5,8,6,7,8][i]) {
          svg.selectAll(".state.m"+i).text(i==3||data["module"+i+"_enabled"]?"On":"Off");
          svg.selectAll(".mah.m"+i).text(data[(i==3?"controller":"module"+i)+"_energy_mAh"].toFixed(0));
        }
        break;
    }
  },
  // App Ready Event Handler
  onAppReady: function() {
    app.log("onAppReady");
    summon.bluetooth.isEnabled(app.onEnable);
  },
  // App Paused Event Handler
  onPause: function() {
    app.log("onPause");
    summon.bluetooth.stopScan();
  },
  // New Data Received Event Handeler
  onData: function(data) {
    app.updateModule(MODULES.findIndex(function(x){return x.dev==data.detail.device.substr(9)}),data.detail);
  },
  // Bluetooth Enabled Callback
  onEnable: function() {
    app.onPause();
    summon.bluetooth.startScan([], app.onDiscover, app.onAppReady);
    app.log("BLE Scanning");
  },
  // BLE Device Discovered Callback
  onDiscover: function(device) {
    var advertisement = device.advertisement;
    if (device.id == gateway.getDeviceId() && advertisement.manufacturerData.length) {
      var data = null;
      var md = new Uint8Array(advertisement.manufacturerData.buffer.slice(3));
      app.log("Found AD: "+Array.prototype.map.call(md,function(m){return ("0"+m.toString(16)).substr(-2);}).join(''));
      switch (advertisement.manufacturerData[2] * 0x100 + advertisement.manufacturerData[3]) {
        case 0x2001: // Power Supply
          data = {
            device: "signpost_status",
            module0_energy_mAh: md[2] * 0x100 + md[3],
            module1_energy_mAh: md[4] * 0x100 + md[5],
            module2_energy_mAh: md[6] * 0x100 + md[7],
            controller_energy_mAh: md[8] * 0x100 +md[9],
            module5_energy_mAh: md[12] * 0x100 + md[13],
            module6_energy_mAh: md[14] * 0x100 + md[15],
            module7_energy_mAh: md[16] * 0x100 + md[17],
            module0_enabled: Boolean(md[18] & 0b10000000),
            module1_enabled: Boolean(md[18] & 0b01000000),
            module2_enabled: Boolean(md[18] & 0b00100000),
            module5_enabled: Boolean(md[18] & 0b00000100),
            module6_enabled: Boolean(md[18] & 0b00000010),
            module7_enabled: Boolean(md[18] & 0b00000001),
          }; break;
        case 0x2002: // Controller
          data = {
            device: "signpost_gps",
            latitude: Math.abs((md[8]*0x1000000+md[9]*0x10000+md[10]*0x100+md[11])/(10000*100.0)),
            latitude_direction: md[8]&0b10000000 ? "S" : "N",
            longitude: Math.abs((md[12]*0x1000000+md[13]*0x10000+md[14]*0x100+md[15])/(10000*100.0)),
            longitude_direction: md[12]&0b10000000 ? "W" : "E",
            timestamp: new Date(Date.UTC(md[4]+2000, md[3]-1, md[2], md[5], md[6], md[7])).toISOString(),
          }; break;
        case 0x2201: // Radio
          data = {
            device: "signpost_radio_status",
            "status_ble_packets_sent": Number(((md[2]*0x100+md[3])*(7.0/8.0)).toFixed(0)),
            "gps_ble_packets_sent": Number(((md[4]*0x100+md[5])*(7.0/8.0)).toFixed(0)),
            "2.4gHz_spectrum_ble_packets_sent":Number(((md[6]*0x100+md[7])*(7.0/8.0)).toFixed(0)),
            "ambient_sensing_ble_packets_sent":Number(((md[8]*0x100+md[9])*(7.0/8.0)).toFixed(0)),
            "audio_spectrum_ble_packets_sent": Number(((md[10]*0x100+md[11])*(7.0/8.0)).toFixed(0)),
            "microwave_radar_ble_packets_sent":Number(((md[12]*0x100+md[13])*(7.0/8.0)).toFixed(0)),
            "ucsd_air_quality_ble_packets_sent":Number(((md[14]*0x100+md[15])*(7.0/8.0)).toFixed(0)),
            "status_lora_packets_sent": (md[2]*0x100+md[3]),
            "gps_lora_packets_sent": (md[4]*0x100+md[5]),
            "2.4gHz_spectrum_lora_packets_sent": (md[6]*0x100+md[7]),
            "ambient_sensing_lora_packets_sent": (md[8]*0x100+md[9]),
            "audio_spectrum_lora_packets_sent": (md[10]*0x100+md[11]),
            "microwave_radar_lora_packets_sent": (md[12]*0x100+md[13]),
            "ucsd_air_quality_lora_packets_sent": (md[14]*0x100+md[15]),
            "status_radio_energy_used_mWh": Number((md[2]*0x100+md[3])*(0.000096+0.01)).toFixed(3),
            "gps_radio_energy_used_mWh": Number((md[4]*0x100+md[5])*(0.000096+0.01)).toFixed(3),
            "2.4gHz_spectrum_radio_energy_used_mWh": Number((md[6]*0x100+md[7])*(0.000096+0.01)).toFixed(3),
            "ambient_sensing_radio_energy_used_mWh": Number((md[8]*0x100+md[9])*(0.000096+0.01)).toFixed(3),
            "audio_spectrum_radio_energy_used_mWh": Number((md[10]*0x100+md[11])*(0.000096+0.01)).toFixed(3),
            "microwave_radar_radio_energy_used_mWh": Number((md[12]*0x100+md[13])*(0.000096+0.01)).toFixed(3),
            "ucsd_air_quality_radio_energy_used_mWh": Number((md[14]*0x100+md[15])*(0.000096+0.01)).toFixed(3),
          }; break;
        case 0x3101: // 2.4GHz Spectrum
          var md8 = new Int8Array(md.buffer.slice(2,18));
          data = {
            device: 'signpost_2.4ghz_spectrum',
            channel_11: md8[0],
            channel_12: md8[1],
            channel_13: md8[2],
            channel_14: md8[3],
            channel_15: md8[4],
            channel_16: md8[5],
            channel_17: md8[6],
            channel_18: md8[7],
            channel_19: md8[8],
            channel_20: md8[9],
            channel_21: md8[10],
            channel_22: md8[11],
            channel_23: md8[12],
            channel_24: md8[13],
            channel_25: md8[14],
            channel_26: md8[15],
          }; break;
        case 0x3201: // Ambient
          data = {
            device: 'signpost_ambient',
            temperature_c: (md[2] * 0x100 + md[3]) / 100.0,
            humidity: (md[4] * 0x100 + md[5]) / 100.0,
            light_lux: md[6] * 0x100 + md[7],
            pressure_pascals: md[8] * 0x100 +md[9],
          }; break;
        case 0x3301: // Audio Frequency
          data = {
            device: 'signpost_audio_frequency',
            "63Hz": Number(((Math.log10((md[2]*0x100+md[3])/43.75)*20)+35.5).toFixed(0)),
            "160Hz": Number(((Math.log10((md[4]*0x100+md[5])/43.75)*20)+35.5).toFixed(0)),
            "400Hz": Number(((Math.log10((md[6]*0x100+md[7])/43.75)*20)+35.5).toFixed(0)),
            "1000Hz": Number(((Math.log10((md[8]*0x100+md[9])/43.75)*20)+35.5).toFixed(0)),
            "2500Hz": Number(((Math.log10((md[10]*0x100+md[11])/43.75)*20)+35.5).toFixed(0)),
            "6250Hz": Number(((Math.log10((md[12]*0x100+md[13])/43.75)*20)+35.5).toFixed(0)),
            "16000Hz": Number(((Math.log10((md[14]*0x100+md[15])/43.75)*20)+35.5).toFixed(0)),
          }; break;
        case 0x3401: // Microwave Radar
          data = {
            device: 'signpost_microwave_radar',
            motion: !(md[2] & 0b10000000),
            'velocity_m/s': (md[3]*0x1000000+md[4]*0x10000+md[5]*0x100+md[6]) / 1000.0,
          }; break;
        case 0x3501: // Air Quality
          data = {
            device: 'signpost_ucsd_air_quality',
            co2_ppm: md[2] * 0x100 + md[3],
            VOC_PID_ppb: md[4]*0x1000000+md[5]*0x10000+md[6]*0x100+md[7],
            VOC_IAQ_ppb: md[8]*0x1000000+md[9]*0x10000+md[10]*0x100+md[11],
            barometric_millibar: md[12] * 0x100 + md[13],
            humidity_percent: md[14] * 0x100 + md[15],
          }; break;
      }
      if (data) document.dispatchEvent(new CustomEvent("data",{detail:data}));
      app.log(JSON.stringify(data));
    }
    if (summon.device.platform=="Android") {
      bluetooth.stopScan();
      bluetooth.startScan([], app.onDiscover, app.onAppReady);
    }
  },
  // Module Click Event Handler
  onClick: function() {
    if (this.id!="rect" && document.body.style.transform=="") {
      el = this.getBoundingClientRect();
      x = window.innerWidth / 2 - el.width / 2 - el.left;
      y = window.innerHeight / 2 - el.height / 2 - el.top;
      s = Math.min(window.innerHeight,window.innerWidth) / (el.width + 20);
      document.body.style.transform = "scale("+s+") translate("+x+"px,"+y+"px)";
    } else document.body.style.transform = "";
  },
  // Function to simulate data packets
  simulatePackets: function() {
    d3.select("#sim").attr("class","flash")
    simulatePackets();
  },
  // Function to Log Text to Screen
  log: function(string) {
    console.log(string);
  },
  x:{},
  y:{}
};

app.initialize();
