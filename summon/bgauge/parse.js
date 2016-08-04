/* Get data from edimax smart plugs when we get an advertisement from
 * it.
 */

var http = require('http');

function postRequest (command, options, cb) {
    options.timeout = 20000;
    options.port = 10000;
    options.path = 'smartplug.cgi';
    options.method = 'POST';
    options.headers = {
            'Content-Type': 'application/xml',
            'Content-Length': command.length
        };
    options.username = 'admin';

    options.headers['Authorization'] =
        "Basic " + new Buffer(options.username + ":" + options.password).toString("base64");

    var data = '';
    var postReq = http.request(options, function (response) {
        var error;
        if (response.statusCode >= 300) {
            cb(undefined);
            return;
        }
        var contentLength = response.headers['content-length'];
        if (contentLength === undefined || parseInt(contentLength) === 0) {
            cb(undefined);
            return;
        }

        response.setEncoding('utf8');
        response.on('data', function (result) {
            data += result;
        });
        response.on('end', function () {
            cb(data);
        });
    }).on('error', function (error) {
        cb(undefined);
    }).on('timeout', function () {
        cb(undefined);
    });
    postReq.setTimeout(options.timeout);
    postReq.write(command);
    postReq.end();
}

var parse_advertisement = function (advertisement, cb) {

    if (advertisement.localName === 'sp2101w') {
        if (advertisement.manufacturerData) {
            // Need at least 3 bytes. Two for manufacturer identifier and
            // one for the service ID.
            var bgauge_lvl = undefined;
            var b = advertisement.manufacturerData;

            if (b.length >= 3) {
                // Check that manufacturer ID and service byte are correct
                var manufacturer_id = b.readUIntLE(0, 2);
                var service_id = b.readUInt8(2);
                if (manufacturer_id == 0x02E0 && service_id == 0x1D) {
                    // OK!
                    // Parse it as a float
                    bgauge_lvl = b.readFloatLE(3);
                }
            }

            if (bgauge_lvl != undefined) {
                var out = {
                    device: 'bgauge',
                    energy_mAh: bgauge_lvl
                };

                cb(out);
                return;
            }

        }
    }

    cb(null);
}


module.exports = {
    parseAdvertisement: parse_advertisement
};
