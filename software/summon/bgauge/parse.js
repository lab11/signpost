/* Get data from battery gauge when we get an advertisement from
 * it.
 */

var parse_advertisement = function (advertisement, cb) {

    if (advertisement.localName === 'bgauge') {
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
