#ifndef MESSAGE_H
#define MESSAGE_H

//this is the first i2c messaging library!
//
//The MTU of the i2c bus is 256Bytes.
//We need packets
//Here's the proposed packet structure
//1:ver,2:total len, 3bit flag, remaind fragment offset, src



//initialize
void message_init(uint8_t src);

//synchronous send
//returns size
uint32_t message_send(uint8_t dest, uint8_t* data, uint32_t len);

//synchronous receive
uint32_t message_recv(uint8_t* data, uint32_t len, uint8_t* src);

//set the read buffer
uint32_t message_set_read_buffer(uint8_t* data, uint32_t len);

#endif
