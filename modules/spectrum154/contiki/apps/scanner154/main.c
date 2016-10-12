
#include <stdio.h>

#include "contiki.h"
#include "sys/etimer.h"
#include "dev/leds.h"

#include "net/netstack.h"
#include "dev/i2c.h"
#include "cc2538-rf.h"


static struct etimer periodic_timer_blue;

/*---------------------------------------------------------------------------*/
PROCESS(scanner154_process, "Scanner 15.4");
AUTOSTART_PROCESSES(&scanner154_process);
/*---------------------------------------------------------------------------*/

int8_t out_buf[18] = {0};
int channel_rssi[16] = {0};
int current_channel = CC2538_RF_CHANNEL_MIN;
int sample_index = 0;



PROCESS_THREAD(scanner154_process, ev, data) {

	PROCESS_BEGIN();

	// Setup buffer for signpost network
	out_buf[0] = 0x31; // my i2c address
	out_buf[1] = 1; // message type

	i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, 400000);

	// etimer_set(&periodic_timer_blue, CLOCK_SECOND/200);
	etimer_set(&periodic_timer_blue, CLOCK_SECOND/100);

	NETSTACK_RADIO.on();


	while(1) {
		PROCESS_YIELD();

		if (etimer_expired(&periodic_timer_blue)) {
			leds_toggle(LEDS_BLUE);


			if (sample_index == 20) {
				// calculate average
				channel_rssi[current_channel-CC2538_RF_CHANNEL_MIN] /= (20*20);

				out_buf[(current_channel-CC2538_RF_CHANNEL_MIN)+2] = (int8_t) channel_rssi[current_channel-CC2538_RF_CHANNEL_MIN];

				// Next!
				sample_index = 0;

				if (current_channel == CC2538_RF_CHANNEL_MAX) {
					// print em
					// for (int i=0; i<16; i++) {
					// 	printf("Channel %i, RSSI %i\n", i+CC2538_RF_CHANNEL_MIN, channel_rssi[i]);
					// }

					// send to radio
					// uint8_t ret = i2c_burst_send(0x22, (uint8_t*) out_buf, 18);
					uint8_t ret = i2c_burst_send(0x20, (uint8_t*) out_buf, 18);
					if (ret != I2C_MASTER_ERR_NONE) {
						i2c_master_command(I2C_MASTER_CMD_BURST_SEND_FINISH);
					}


					current_channel = CC2538_RF_CHANNEL_MIN;
				} else {
					current_channel++;
				}
				NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, current_channel);

			} else {
				radio_value_t val;

				for (int i=0; i<20; i++) {
					NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &val);
					channel_rssi[current_channel-CC2538_RF_CHANNEL_MIN] += val;
				}
				sample_index++;
			}

			etimer_restart(&periodic_timer_blue);
		}
	}

	PROCESS_END();
}
