//A userland driver for the UBLOX SARA-U260
//https://www.u-blox.com/en/product/sara-u2-series

#include <stdint.h>
#include <tock.h>

#define SARA_U260_SUCCESS 0
#define SARA_U260_ERROR -1
#define SARA_U260_INVALIDPARAM -2
#define SARA_U260_NO_SERVICE -3

//initializes and turns off command echo
int sara_u260_init(void);

//Attempts to perform an HTTP Post
//Will attempt to join the network and setup a packet switch connect if one does not exist
int sara_u260_basic_http_post(const char* url, const char* path, uint8_t* buf, size_t len);

//Returns the response from the most recent successful post
int sara_u260_get_post_response(uint8_t* buf, size_t max_len);

//Returns part of the response from the most recent successful post
int sara_u260_get_post_partial_response(uint8_t* buf, size_t offset, size_t max_len);
