#include <stdio.h>
#include "tock.h"
#include "simple_post.h"
#include "signpost_api.h"

int simple_octetstream_post(const char* url, uint8_t* buf, uint16_t buf_len) {

      http_request r;
      r.num_headers = 1;
      http_header h;
      h.header = "content-type";
      h.value = "application/octet-stream";
      r.headers = &h;
      r.body_len = buf_len;
      r.body = buf;

      //prep a place to put responses
      http_response r2;
      r2.num_headers = 0;
      r2.reason_len = 0;
      r2.body_len = 0;
      r2.headers = NULL;

      int result = signpost_networking_post(url, r, &r2);
      if(result != 0) {
          printf("WARN: Simple Octet Stream Post error %d\n",result);
          return result;
      } else {
          return r2.status;
      }
}
