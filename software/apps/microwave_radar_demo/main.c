#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include <tock.h>
#include <firestorm.h>
#include <adc.h>
#include <console.h>

#include "i2c_master_slave.h"

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
const uint32_t INITIAL_VAL = 0x820 << 4; // average value of ADC
const uint32_t LOWER_BOUND = 0X800 << 4; // when adc is smaller than this, something moves
const uint32_t UPPER_BOUND = 0x840 << 4; // when adc is greater than this, something moves
const uint32_t NOISE_OFFSET = 4 << 4;	// Dont account for edge if change of adc is less than noise

// Some define
#define SAMPLE_TIMES 20
#define ADC_CHANNEL 0
#define is_stable(curr, prev) (curr < prev + NOISE_OFFSET && curr > prev - NOISE_OFFSET)
#define BUFFER_SIZE 20
#define I2C_ADDR_INDEX 0
#define I2C_TEXT_TYPE_INDEX 1
#define I2C_FREQ_START_INDEX 2
#define I2C_SPEED_START_INDEX 6

// used to store the interval of each half period
uint32_t time_intervals[SAMPLE_TIMES];

// i2c buffers
uint8_t slave_write_buf[BUFFER_SIZE];
uint8_t slave_read_buf[BUFFER_SIZE];
uint8_t master_write_buf[BUFFER_SIZE];
uint8_t master_read_buf[BUFFER_SIZE];

static void i2c_master_slave_callback (
	int callback_type __attribute__ ((unused)),
	int length __attribute__ ((unused)),
	int unused __attribute__ ((unused)),
	void * callback_args __attribute__ ((unused))) {
	return;
}

void initialize_i2c(){
	// low configure i2c slave to listen
	i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
	i2c_master_slave_set_slave_address(0x34);

	i2c_master_slave_set_master_read_buffer(master_read_buf, BUFFER_SIZE);
	i2c_master_slave_set_master_write_buffer(master_write_buf, BUFFER_SIZE);
	i2c_master_slave_set_slave_write_buffer(slave_write_buf, BUFFER_SIZE);
	i2c_master_slave_set_slave_read_buffer(slave_read_buf, BUFFER_SIZE);

	// listen
	i2c_master_slave_listen();

	master_write_buf[I2C_ADDR_INDEX] = 0x34;
	master_write_buf[I2C_TEXT_TYPE_INDEX] = 0x00;
}

// Callback when the adc reading is done
void callback (int callback_type, int data, int data2, void* callback_args) {
  UNUSED_PARAMETER(callback_args);
  UNUSED_PARAMETER(callback_type);

  _channel = data;
  _adc_val = data2;

}

void print_timer () {
  char buf[64];
  sprintf(buf, "\tTimer: %u\n\n", _timer);
  putstr(buf);
}

void print_data () {
  char buf[64];
  // the value read from adc register is left alligned to 16 bits
  sprintf(buf, "\tGot: 0x%02x\n\n", _adc_val);
  putstr(buf);
}

void sample_frequency(){
  prev_data = INITIAL_VAL;
  curr_data = _adc_val;

  uint32_t i;

  t2 = timer_read();


  for(i = 0; i < SAMPLE_TIMES; ++i){
  	t1 = t2;
	if(prev_data < curr_data) {
		while((prev_data < curr_data) || is_stable(curr_data, prev_data)){
			adc_single_sample(ADC_CHANNEL);
			prev_data = curr_data;
			curr_data = _adc_val;
			delay_ms(1);
		}
	} else if(prev_data >= curr_data) {
		while((prev_data >= curr_data) || is_stable(curr_data, prev_data)){
			adc_single_sample(ADC_CHANNEL);
			prev_data = curr_data;
			curr_data = _adc_val;
			delay_ms(1);
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

 // Note: this speed is not really meaningful because it calculates
 // the corresponding speed when object moving perpendicular to sensor
 uint32_t speed_fph = (frequency * 5280)/31;

  char buf[64];
  sprintf(buf, "\tFrequency is: %u, Speed is: %u feet per hour\n\n", frequency, speed_fph);
  putstr(buf);

  for(i = I2C_FREQ_START_INDEX; i < I2C_FREQ_START_INDEX + sizeof(uint32_t); ++i){
  	master_write_buf[i] = frequency & 0xff;
	frequency = frequency >> 8;
  }
  for(i = I2C_SPEED_START_INDEX; i < I2C_SPEED_START_INDEX + sizeof(uint32_t); ++i){
  	master_write_buf[i] = speed_fph & 0xff;
	speed_fph = speed_fph >> 8;
  }

  i2c_master_slave_write(0x22, 16);

  delay_ms(500);
}

bool detect_moving(){
    return ((_adc_val) > UPPER_BOUND || (_adc_val) < LOWER_BOUND);
}

int main () {
  putstr("Start Microwave Radar program. Sampling!!!\n");

  // initialize i2c
  initialize_i2c();

  // set adc callback function to read adc value
  adc_set_callback(callback, NULL);

  // initialize adc
  adc_initialize();

  while (1) {
    adc_single_sample(0);
    yield();
    //print_data();
    if(detect_moving()){
	sample_frequency();
    }
  }
}
