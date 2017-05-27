#include <stdint.h>
#include <tock.h>

#define SARA_U260_SUCCESS 0
#define SARA_U260_ERROR -1
#define SARA_U260_INVALIDPARAM -2
#define SARA_U260_NO_SERVICE -3

int sara_u260_init(void);
int sara_u260_basic_http_post(const char* url, const char* path, uint8_t* buf, size_t len);
int sara_u260_authenticated_http_post(const char* url, const char* path, uint8_t* buf,
                                        size_t len, const char* username, const char* password);

int sara_u260_get_post_response(uint8_t* buf, size_t max_len);
