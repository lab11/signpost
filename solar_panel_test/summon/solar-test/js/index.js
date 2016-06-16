
var deviceId = "C0:98:E5:90:00:05";                                                 // while testing, replace with address of a BLE peripheral
var deviceName = "BLE Device";                                                      // while testing, replace with desired name

var timer;

var last_update = 0;

var switch_visibility_console_check = "visible";
var switch_visibility_steadyscan_check = "visible";

// Load the swipe pane
$(document).on('pageinit',function(){
    $("#main_view").on("swipeleft",function(){
        $("#logPanel").panel( "open");
    });
});

var app = {
    // Application Constructor
    initialize: function() {
        app.log("Solar init");

        document.addEventListener("deviceready", app.onAppReady, false);
        document.addEventListener("resume", app.onAppReady, false);
        document.addEventListener("pause", app.onPause, false);
		
        if (typeof window.gateway != "undefined") { // if UI opened through Summon,
        	app.onAppReady();
		}
    },
    // App Ready Event Handler
    onAppReady: function() {
        app.log("onAppReady");

        // Setup update for last data time
        setInterval(app.update_time_ago, 5000);

        if (typeof window.gateway != "undefined") {                               // if UI opened through Summon,
            deviceId = window.gateway.getDeviceId();                                // get device ID from Summon
            deviceName = window.gateway.getDeviceName();                            // get device name from Summon
            app.log("Opened via Summon..");
        }
        document.getElementById("title").innerHTML = String(deviceId);
        app.log("Checking if ble is enabled...");
        bluetooth.isEnabled(app.onEnable);                                                // if BLE enabled, goto: onEnable
        // app.onEnable();
    },
    // App Paused Event Handler
    onPause: function() {
        app.log("on Pause");                                                           // if user leaves app, stop BLE
        bluetooth.stopScan();
    },
    // Bluetooth Enabled Callback
    onEnable: function() {
        app.log("onEnable");
        // app.onPause();                                                              // halt any previously running BLE processes
        bluetooth.startScan([], app.onDiscover, app.onAppReady);                          // start BLE scan; if device discovered, goto: onDiscover
        app.log("Searching for " + deviceName + " (" + deviceId + ").");
    },
    // BLE Device Discovered Callback
    onDiscover: function(device) {
        if (device.id == deviceId) {
            app.log("Found " + deviceName + " (" + deviceId + ")!");
            app.onParseAdvData(device);
        } else {
            //app.log('Not Blink (' + device.id + ')');

            // HACK:
            bluetooth.stopScan();
            bluetooth.startScan([], app.onDiscover, app.onAppReady);
        }
    },
   onParseAdvData: function(device){
        //Parse Advertised Data
        var advertisement = device.advertisement;


        // Check this is something we can parse
        if (advertisement.localName == 'Solar' &&
                advertisement.manufacturerData) { 

			var mandata = advertisement.manufacturerData;
			
            // Save when we got this.
            last_update = Date.now();
			
			//check that it's a data packet
			app.log(mandata);
			if (mandata[0] == 224) {

				var data = new DataView(new Uint8Array(advertisement.manufacturerData.subarray(3)).buffer);
				var volt = data.getFloat32(1)
				var curr = data.getFloat32(5)
				var pow = data.getFloat32(9)
				app.log("My Voltage: " + volt.toString());
        		document.getElementById("vVal").innerHTML = volt.toString();
        		document.getElementById("cVal").innerHTML = curr.toString();
        		document.getElementById("pVal").innerHTML = pow.toString();
			}


            app.update_time_ago();

        } else {
            app.log('Advertisement was not Solar.');
        }

    },
    update_time_ago: function () {
        if (last_update > 0) {
            // Only do something after we've gotten a packet
            // Default output
            var out = 'Haven\'t gotten a packet in a while...';

            var now = Date.now();
            var diff = now - last_update;
            if (diff < 60000) {
                // less than a minute
                var seconds = Math.round(diff/1000);
                out = 'Last updated ' + seconds + ' second';
                if (seconds != 1) {
                    out += 's';
                }
                out += ' ago';

            } else if (diff < 120000) {
                out = 'Last updated about a minute ago';
            }

            document.querySelector("#data_update").innerHTML = out;
        }
    },
    // Function to Log Text to Screen
    log: function(string) {
        document.querySelector("#console").innerHTML += (new Date()).toLocaleTimeString() + " : " + string + "<br />";
        document.querySelector("#console").scrollTop = document.querySelector("#console").scrollHeight;
    }
};

app.initialize();
