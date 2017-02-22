Getting Started with the Signpost API
=====================================

The signpost API runs on signpost modules and handles initialization while
providing modules with access to shared services such as networking,
storage, processing, energy, time and location. This prevents the module
developer from needing to interact with the shared I2C bus directly, or
knowing the message formats. This document is a simple guide to using 
the APIs. To use these APIs you must 

```c
#include "signpost_api.h"
```

at the top of your application.

##Contents
1. [Initialization](#initialization)
2. [Storage](#storage)
3. [Networking](#networking)
4. [Simple Networking](#simple networking)
5. [Posting to GDP](#posting to gdp)
6. [Energy](#energy)
7. [Time](#time)
8. [Location](#location)
9. [Processing](#processing)

##Initialization

Initialization registers a signpost module with the controller and sets up
shared symmetric keys with other modules. For now, this method could take a few seconds to run
because it must wait for the controller to perform key
exchange. If you are providing a service to other signpost modules, you would also
declare that service here.

The prototype of the initialization function is:

```c
int signpost_initialization_module_init(uint8_t i2c_address, api_handler_t** api);
```

In practice, most modules will simply call:

```c
int result =  signpost_initialization_module_init(0x30, NULL);
```

Where 0x30 should be replaced by an I2C address not currently being used
by other modules. Addresses 0x20,0x21, and 0x22 are reserved. In the future
we plan to add automatic address resolution so conflicting is not an issue.

This is the first Signpost API function every module should call.

##Storage

Modules can storage data on an SD card that is on the control module.

They can access this SD card through the storage API. Currently modules
can only write to the SD card in an append only log. This would be good for
storing long term data, and allowing the Intel Edison to access and 
process that data. In the future we plan to allow modules to
read data from the SD card, and make the SD card format the data in
a filesystem readable by your computer, but these are still being implemented.

To store data to the SD card:

```c
uint8_t data = {0x01, 0x02, 0x03};
Storage_Record_t record;
int result = signpost_storage_write(data, 3, &record);
```

where record could then be used by the Intel Edison to access the data, 
or in the future by your module to read the data.

##Networking

Currently the signpost API provides an http post abstraction. The prototype
of this function is

```c
int signpost_networking_post(char* url, http_request request, http_response* response);
```

where http_request is a structure containing the fields for a post and
http_response is a structure which you provide and will be populated upon
completion of the post. Note that API will fill in
content-length for you. A simple call to http_post would look like:

```c
uint8_t test[200];

const char* url = "httpbin.org/post";
http_request request;
http_header h;
h.header = "content-type";
h.value = "application/octet-stream";
r.num_headers = 1;
r.headers = &h;
r.body_len = 20;
r.body = test;

http_response response;
r2.num_headers = 0;
r2.headers = NULL;
r2.reason_len = 0;
r2.body_len = 200;
r2.body = test;

int result = signpost_networking_post(url, request, &response);
```

The result is 0 on success, and negative on error.

Note that the response will only populate as much as you provide (i.e. to get
a response with multiple headers, you would need to declare multiple 
http_response_header structures and initialize them with buffers and lengths;
to get a longer post body, you would need to declare a longer buffer to put that post
body).

###Simple Networking

Because the above code is both longer and more flexible than most people need,
we created a simple wrapper around it called simple_post. To use it:

```c
#include "simple_post.h"
```

Simple post only posts binary data with application/octet-stream
content-type, which should be appropriate for most sensor data. 
To use simple_post:

```c
uint8_t data = {0x01, 0x02, 0x03};
int status = simple_octetstream_post("httpbin.org/post", data, 3);
```

Where the status is the http status returned by the website, or the error
code if negative.

###Posting to GDP
While GDP does not currently have a rest API, your 
[signpost-debug-radio](../receiver/debug_radio/)
will append to gdp if you post to the url `gdp.lab11.eecs.umich.edu/gdp/v1/<log_name>/append`
where `<log_name>` is the name of the log. This log will be created if it does
not already exist. Therefore, posting sensor data to GDP would look like:

```c
uint8_t data = {0x01, 0x02, 0x03};
int status = simple_octetstream_post("gdp.lab11.eecs.umich.edu/gdp/v1/
                                      edu.umich.eecs.lab11/fake-data/append", data, 3);
```

##Time

The time API returns current time. If you care about time synchronization, 
this _should_ be correlated with the pulse per second (PPS) line routed to your module. 
To get time synchronization we recommend the following procedure:

    1. Listen for PPS
    2. Start timer  
    3. Perform time request
    4. Wait for response
    5. If response happens in <1s, the next PPS will be response_time + 1s, otherwise, retry.

This should be able to provide every module with global time sync with error 
around the delay it takes for tock to propagate the PPS signal up to your app (which
we believe to be much greater than error from GPS or propagation delay). Tock is
not an RTOS. 

To use the time API declare a time structure, and call the time API function:

```c
signpost_timelocation_time_t time;
int result_code = signpost_timelocation_get_time(&time);
```

The time structure provides the fields:

```c
typedef struct __attribute__((packed)) {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t satellite_count;
} signpost_timelocation_time_t;
```

The time will be valid if satellite_count > 0.

##Location

The location API provides location from the GPS. To use the location API:

```c
signpost_timelocation_location_t location;
int result_code = signpost_timelocation_get_location(&location);
```

The location structure provides the fields:

```c
typedef struct __attribute__((packed)) {
    float latitude;
    float longitude;
    uint8_t satellite_count;
} signpost_timelocation_location_t;
```

Location is valid if satellite_count >= 4.

##Energy

This API is not implemented yet (it returns junk data).

Here is our API proposal:

```c
signpost_energy_information_t energy;
int result = signpost_energy_query(&energy);
```

where the energy structure is:

```c
typedef struct __attribute__((packed)) energy_information {
    uint32_t    energy_limit_24h_mJ;
    uint32_t    energy_used_24h_mJ;
    uint16_t    current_average_mJDay;
    uint8_t     energy_limit_warning_threshold;
    uint8_t     energy_limit_critical_threshold;
} signpost_energy_information_t;
```

The goal of this is to tell a module if they are using too much energy. The controller
will occasionally update energy_limit_24h_mJ based on incoming energy from
the solar panel. A module can see how much energy it has used
over the past 24h window and the current_average_mJDay should allow a
module to easily compare its current usage to the limit and see if it
will exceed the limit.

##Processing

The processing API allows a module to use the Intel Edison Linux computer
on the signpost controller. Modules can access the Intel Edison
through an RPC Abstraction. It consists of two parts:

**Note that this is not completely working, but we are close.**

**Initialization**

```c
int result = signpost_processing_init("/path/to/RPC")
```

where path to RPC is the location of a python module that implements
your RPC functions on the Intel Edison (we eventually plan to allow this to also be a remote 
path to a server or git repo).

**RPC Calls**

In module code, using the RPC interface is as simple as including
your RPC header and calling the RPC. The RPC library handles getting
this call from your module to the Intel Edison and returning a result.
Writing the RPC definitions
and implementing the python module to execute the RPC is more
than we will cover in this document. Please see the [RPC documentation](./RPCApi.md)
for more information.



