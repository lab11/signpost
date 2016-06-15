/* Parse Squall PIR advertisements */

var parse_advertisement = function (advertisement, cb) {

    if (advertisement.localName === 'squall+PIR') {
        if (advertisement.manufacturerData) {
            // Need at least 3 bytes. Two for manufacturer identifier and
            // one for the service ID.
            if (advertisement.manufacturerData.length >= 3) {
                // Check that manufacturer ID and service byte are correct
                var manufacturer_id = advertisement.manufacturerData.readUIntLE(0, 2);
                var service_id = advertisement.manufacturerData.readUInt8(2);
                if (manufacturer_id == 0x02E0 && service_id == 0x13) {
                    // OK! This looks like a PIR packet
                    if (advertisement.manufacturerData.length == 6) {
                        var pir = advertisement.manufacturerData.slice(3);

                        var current_motion        = pir.readUInt8(0); // 1 if the PIR is currently detecting motion, 0 otherwise
                        var motion_since_last_adv = pir.readUInt8(1); // 1 if the PIR detected motion at any point since the last adv was transmitted
                        var motion_last_minute    = pir.readUInt8(2); // 1 if the PIR detected motion at any point in the last minute

                        var out = {
                            device: 'Blink',
                            current_motion:        current_motion==1,
                            motion_since_last_adv: motion_since_last_adv==1,
                            motion_last_minute:    motion_last_minute==1
                        };

                        cb(out);
                        return;
                    }
                }
            }
        }
    }

    cb(null);
}


module.exports = {
    parseAdvertisement: parse_advertisement
};
