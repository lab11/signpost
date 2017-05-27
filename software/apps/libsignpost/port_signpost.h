#pragma once

/* This is a port interface for the signpost API and networking stack!
The goal is to export the minimal interface such that platforms
can integrate with the signpost.

From a high level you will need to provide:
    - Blocking I2C write function (currently at least 255 bytes at a time)
    - Asynchronous I2C slave function (listen on an address and send 
                                        back written bytes asynchronously)
    - Set and clear of the MOD_OUT pin
    - Asynchronous callback for falling edge of the Mod-in pin
    - Currently the signpost libraries assume printf functionality
        - This is assumed to be provided through <stdio.h>
    - A way to wait on a variable reference
        -This could be as simple as looping until it changes or 
        -Could be more complex like going to sleep
        -It is assumed the variable will change due to an interrupt context

To provide this functionality please implement the following interface
for each platform*/

#define PORT_SIGNPOST_ERROR -1
#define PORT_SIGNPOST_I2C_WRITE_ERROR -2

//These are the callback definitions

//This is the typedef of the callback you should call when you get an i2c slave write
//You can either return the (positive) length or an error code
typedef void (*port_signpost_i2c_slave_write_callback)(int len_or_rc);

//This is the typedef of the callback you should call when you see a falling edge on
//the mod in pin
typedef void (*port_signpost_mod_in_falling_edge_callback)(void);

//This function is called upon signpost initialization
//You should use it to set up the i2c interface
void port_signpost_init(uint8_t i2c_address);

//This function is a blocking i2c send call
//it should return the length of the message successfully sent on the bus
//If the bus returns an error, use the appropriate error code
//defined in this file
int port_signpost_i2c_master_write(uint8_t addr, uint8_t* buf, int len);

//This function is sets up the asynchronous i2c receive interface
//When this function is called start listening on the i2c bus for
//The address specified in init
//Place data in the buffer no longer than the max len
void port_signpost_i2c_slave_listen(port_signpost_i2c_slave_write_callback cb, uint8_t* buf, int max_len);

//These functions are used to control the mod_out pin
void port_signpost_mod_out_set(void);
void port_signpost_mod_out_clear(void);

//This function is used to get the input interrupt for the falling edge of
//mod-in
void port_signpost_mod_in_falling_edge_listen(port_signpost_mod_in_falling_edge_callback cb);

//This is a way to wait on a variable in a platform specific way
void port_signpost_wait_for(void* wait_on_true);


