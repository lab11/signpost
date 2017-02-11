#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "adc.h"
#include "console.h"
#include "timer.h"
#include "tock.h"

#define UNUSED_PARAMETER(x) (void)(x)

// Global store
uint32_t t1;
uint32_t t2;
uint32_t sample_index;
uint32_t max_sample;
uint32_t min_sample;
bool active_mode = false;

#define RISING 2
#define FALLING 3
#define FINISH_RISING 5
#define FINISH_FALLING 6
uint32_t sample_state;

// Other global variable
const uint32_t INITIAL_VAL = 0x820;
//const uint32_t LOWER_BOUND = 0X800;
//const uint32_t UPPER_BOUND = 0x840;
const uint32_t LOWER_BOUND = 0X750;
const uint32_t UPPER_BOUND = 0x890;
//const uint32_t NOISE_OFFSET = 4;
const uint32_t NOISE_OFFSET = 10;

// Some define
#define SAMPLE_TIMES 20
#define ADC_CHANNEL 0

// used to store the interval of each half period
uint32_t time_intervals[SAMPLE_TIMES];

static bool detect_motion (uint32_t sample) {
    return (sample > UPPER_BOUND) || (sample < LOWER_BOUND);
}

static uint32_t calculate_sample_frequency (uint32_t curr_data) {
    // check if data signifies movement
    if (!active_mode && detect_motion(curr_data)) {
        // initialize active mode
        active_mode = true;
        sample_index = 0;

        // starting this period sample
        t1 = timer_read();
        if (INITIAL_VAL < curr_data) {
            sample_state = RISING;
            max_sample = curr_data;
            min_sample = curr_data;
        } else {
            sample_state = FALLING;
            min_sample = curr_data;
            max_sample = curr_data;
        }
    }

    // sample frequencies
    if (active_mode) {

        // keep sampling until signals peaks or bottoms
        if (sample_state == RISING) {
            if (curr_data <= (max_sample - NOISE_OFFSET)) {
                // data falling again
                sample_state = FINISH_RISING;
            } else {
                if (curr_data > max_sample) {
                    max_sample = curr_data;
                }
                sample_state = RISING;
            }
        } else if (sample_state == FALLING) {
            if (curr_data >= (min_sample + NOISE_OFFSET)) {
                // data rising again
                sample_state = FINISH_FALLING;
            } else {
                if (curr_data < min_sample) {
                    min_sample = curr_data;
                }
                sample_state = FALLING;
            }
        }

        // calculate time for that period
        if (sample_state == FINISH_RISING || sample_state == FINISH_FALLING) {
            t2 = timer_read();
            time_intervals[sample_index] = t2-t1;
            sample_index++;

            t1 = t2;
            if (sample_state == FINISH_FALLING) {
                sample_state = RISING;
                max_sample = curr_data;
                min_sample = curr_data;
            } else if (sample_state == FINISH_RISING) {
                sample_state = FALLING;
                min_sample = curr_data;
                max_sample = curr_data;
            }

            if (sample_index >= SAMPLE_TIMES) {
                // average clock ticks between pos and neg edge
                uint32_t average = 0;

                // The first value is outlier. Don't account for that
                for(int i = 1; i < SAMPLE_TIMES; i++) {
                    average += time_intervals[i]/(SAMPLE_TIMES - 1);
                }

                // Since we are using timer with 32kHz frequency and prescaler 0
                // One tick means 0.0000625 s
                uint32_t interval = average * 625 * 2;
                uint32_t frequency = 10000000/interval;
                active_mode = false;

                return frequency;
            }
        }
    }

    return 0;
}

static uint32_t calculate_radar_speed (uint32_t freq) {
    // Note: this speed is not really meaningful because it calculates
    // the corresponding speed when object moving perpendicular to sensor
    uint32_t speed_fph = (freq * 5280)/31;
    uint32_t speed_fps = speed_fph*1000/3600;

    printf("Freq: %ld\tSpeed: %ld (milli-fps)\n", freq, speed_fps);

    return speed_fps;
}


// Callback when the adc reading is done
static void adc_callback (int callback_type, int channel, int sample, void* callback_args) {
    UNUSED_PARAMETER(callback_type);
    UNUSED_PARAMETER(channel);
    UNUSED_PARAMETER(callback_args);

    bool motion = detect_motion(sample);
    uint32_t speed_fps = 0;

    // determine mircowave radar frequency
    uint32_t freq = calculate_sample_frequency(sample);
    if (freq != 0) {
        speed_fps = calculate_radar_speed(freq);
    }

    // get new sample
    adc_single_sample(ADC_CHANNEL);
}


int main (void) {
    putstr("Start Microwave Radar program. Sampling!!!\n");

    // initialize adc
    adc_set_callback(adc_callback, NULL);
    adc_initialize();

    // start getting samples
    adc_single_sample(ADC_CHANNEL);
}

