#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include <firestorm.h>
#include <adc.h>
#include <tock_str.h>

#define UNUSED_PARAMETER(x) (void)(x)

// Global store
uint32_t _channel = 0;
uint32_t _adc_val = 0;
uint32_t _timer = 0;
uint32_t prev_data;
uint32_t curr_data;
uint32_t t1;
uint32_t t2;

// Other global variable
const uint32_t INITIAL_VAL = 0x820;
const uint32_t LOWER_BOUND = 0X800;
const uint32_t UPPER_BOUND = 0x840;
const uint32_t NOISE_OFFSET = 4;

// Some define
#define SAMPLE_TIMES 20
#define ADC_CHANNEL 0
#define is_stable(curr, prev) (curr < prev + NOISE_OFFSET && curr > prev - NOISE_OFFSET)

// used to store the interval of each half period
uint32_t time_intervals[SAMPLE_TIMES];

// Callback when the adc reading is done
void callback (int callback_type, int data, int data2, void* callback_args) {
  UNUSED_PARAMETER(callback_args);
  UNUSED_PARAMETER(callback_type);

  _channel = data;
  _adc_val = data2;

}

void print_timer () {
  char buf[64];
  sprintf(buf, "\tTimer: %d\n\n", _timer);
  putstr(buf);
}

void print_data () {
  char buf[64];
  // the value read from adc register is left alligned to 16 bits
  sprintf(buf, "\tGot: 0x%02x\n\n", _adc_val>>4);
  putstr(buf);
}

void sample_frequency(){
  prev_data = INITIAL_VAL;
  curr_data = _adc_val>>4;

  uint32_t i;

  t2 = timer_read();


  for(i = 0; i < SAMPLE_TIMES; ++i){
  	t1 = t2;
	if(prev_data < curr_data) {
		while((prev_data < curr_data) || is_stable(curr_data, prev_data)){
			adc_single_sample(ADC_CHANNEL);
			prev_data = curr_data;
			curr_data = _adc_val >> 4;
			delay_ms(2);
		}
	} else if(prev_data >= curr_data) {
		while((prev_data >= curr_data) || is_stable(curr_data, prev_data)){
			adc_single_sample(ADC_CHANNEL);
			prev_data = curr_data;
			curr_data = _adc_val >> 4;
			delay_ms(2);
		}
	}

	t2 = timer_read();
	time_intervals[i] = t2 - t1;

  }

  // average clock ticks between pos and neg edge
  uint32_t average = 0;

  // The first value is outlier. Don't account for that
  for(i = 1; i < SAMPLE_TIMES; ++i) {
  	average += time_intervals[i]/(SAMPLE_TIMES - 1);
  }

 // Since we are using timer with 32kHz frequency and prescaler 0
 // One tick means 0.0000625 s
 uint32_t interval = average * 625 * 2;
 uint32_t frequency = 10000000/interval;


  char buf[64];
  sprintf(buf, "\tFrequency is: %d\n\n", frequency);
  putstr(buf);

  delay_ms(500);

}

bool detect_moving(){
    return ((_adc_val>>4) > UPPER_BOUND || (_adc_val>>4) < LOWER_BOUND);
}

int main () {
  putstr("Start Microwave Radar program. Sampling!!!\n");

  adc_set_callback(callback, NULL);
  // initialize adc
  adc_initialize();


  while (1) {
    adc_single_sample(0);
    yield();
    if(detect_moving()){
	sample_frequency();
    }
  }
}
