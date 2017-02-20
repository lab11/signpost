#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "i2c_master_slave.h"
#include "signbus_io_interface.h"
#include "tock.h"

#pragma GCC diagnostic ignored "-Wstack-usage="

#define I2C_MAX_LEN 255

static uint8_t master_write_buf[I2C_MAX_LEN];
static uint8_t slave_write_buf[I2C_MAX_LEN];
//static uint8_t master_read_buf[I2C_MAX_LEN];
static uint8_t slave_read_buf[I2C_MAX_LEN];
static uint8_t packet_buf[I2C_MAX_LEN];

typedef struct __attribute__((packed)) signbus_network_flags {
    unsigned int is_fragment   : 1;
    unsigned int rsv_wire_bit6 : 1;
    unsigned int rsv_wire_bit5 : 1;
    unsigned int rsv_wire_bit4 : 1;
    unsigned int version       : 4;
} signbus_network_flags_t;
_Static_assert(sizeof(signbus_network_flags_t) == 1, "network flags size");

typedef struct __attribute__((packed)) signbus_network_header {
    union {
        uint8_t flags_storage;
        signbus_network_flags_t flags;
    };
    uint8_t src;
    uint16_t sequence_number;
    uint16_t length;
    uint16_t fragment_offset;
} signbus_network_header_t;
_Static_assert(sizeof(signbus_network_header_t) == 8, "network header size");

#define MAX_DATA_LEN (I2C_MAX_LEN-sizeof(signbus_network_header_t))

typedef struct {
    signbus_network_header_t header;
    uint8_t data[MAX_DATA_LEN];
} __attribute__((__packed__)) Packet;

typedef struct {
    bool new;
    uint8_t len;
} new_packet;

static uint8_t this_device_address;
static uint16_t sequence_number = 0;
static new_packet np = { .new = false };


__attribute__((const))
static uint16_t htons(uint16_t in) {
    return (((in & 0x00FF) << 8) | ((in & 0xFF00) >> 8));
}


/***************************************************************************
 * Tock I2C Interface
 *
 * The underlying I2C mechanism imposes a 255 byte MTU.
 * On the sending side, this means we simply fragment until complete.
 * Receiving is a little more tricky. We pass a 255 byte buffer that we own to
 * the I2C driver which is always re-used to get individual messages from the
 * I2C bus. When an upper layer wishes to recieve a message, it passes a buffer
 * to copy into, which we copy I2C data into until we get a complete message.
 *
 ***************************************************************************/

// Internal helper that copies data from the I2C driver buffer to the packet
// buffer. Forward declaration here so callback can use it.
static int get_message(
        uint8_t* recv_buf,
        size_t recv_buflen,
        uint8_t* src_address
        );

// Internal helper for supporting slave reads. Forward declaration here so
// callback can use it.
static void signbus_iterate_slave_read(void);

// State for an active async event
bool                    async_active = false;
uint8_t*                async_recv_buf;
size_t                  async_recv_buflen;
uint8_t*                async_src_address;
signbus_app_callback_t* async_callback = NULL;

// flag to indicate if callback is for async operation
static bool async = false;

// Kernel callback for I2C events
// n.b. currently only sync send is implemented, so only recv callbacks here
static void i2c_master_slave_callback(
        int callback_type,
        int length,
        int unused __attribute__ ((unused)),
        __attribute__ ((unused)) void* callback_args) {
    SIGNBUS_DEBUG("type %d-%s length %d cb args %p\n",
            callback_type,
            (callback_type == TOCK_I2C_CB_SLAVE_WRITE) ? "SLAVE_WRITE" :
            (callback_type == TOCK_I2C_CB_SLAVE_READ_COMPLETE) ? "SLAVE_READ_COMPLETE" :
            "UNKNOWN",
            length, callback_args);

    if(callback_type == TOCK_I2C_CB_SLAVE_WRITE) {
        memcpy(packet_buf, slave_write_buf, length);
        new_packet* packet = (new_packet*) &np;
        packet->new = true;
        packet->len = length;
        if (async_active) {
            get_message(async_recv_buf, async_recv_buflen, async_src_address);
        }
    } else if (callback_type == TOCK_I2C_CB_SLAVE_READ_COMPLETE) {
        signbus_iterate_slave_read();
    }
}


/// Initialization routine that sets up buffers used to read and write to
/// the underlying I2C device. Also sets the slave address for this device.
///
/// MUST be called before any other methods.
void signbus_io_init(uint8_t address) {
    i2c_master_slave_set_slave_write_buffer(slave_write_buf, I2C_MAX_LEN);
    i2c_master_slave_set_master_write_buffer(master_write_buf, I2C_MAX_LEN);
    i2c_master_slave_set_slave_read_buffer(slave_read_buf, I2C_MAX_LEN);
    i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
    i2c_master_slave_set_slave_address(address);
    this_device_address = address;
}


// synchronous send call
int signbus_io_send(uint8_t dest, uint8_t* data, size_t len) {
    SIGNBUS_DEBUG("dest %02x data %p len %d\n", dest, data, len);

    sequence_number++;
    Packet p = {0};
    size_t toSend = len;

    //calculate the number of packets we will have to send
    uint16_t numPackets;
    if(len % MAX_DATA_LEN) {
        numPackets = (len/MAX_DATA_LEN) + 1;
    } else {
        numPackets = (len/MAX_DATA_LEN);
    }

    //set version
    p.header.flags.version = 0x01;
    //set the source
    p.header.src = this_device_address;
    p.header.sequence_number = htons(sequence_number);

    //set the total length
    p.header.length = htons((numPackets*sizeof(signbus_network_header_t))+len);

    while(toSend > 0) {
        uint8_t morePackets = 0;
        uint16_t offset = 0;

        //calculate moreFragments
        morePackets = (toSend > MAX_DATA_LEN);
        //calculate offset
        offset = (len-toSend);

        //set more fragments bit
        p.header.flags.is_fragment = morePackets;

        //set the fragment offset
        p.header.fragment_offset = htons(offset);

        //set the data field
        //if there are more packets write the whole packet
        if(morePackets) {
            memcpy(p.data,
                    data+offset,
                    MAX_DATA_LEN);
        } else {
            //if not just send the remainder of the data
            memcpy(p.data,
                    data+offset,
                    toSend);
        }

        //copy the packet into the send buffer
        memcpy(master_write_buf,&p,I2C_MAX_LEN);

        //send the packet in syncronous mode
        if(morePackets) {
            i2c_master_slave_write_sync(dest,I2C_MAX_LEN);
            i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
            toSend -= MAX_DATA_LEN;
        } else {
            SIGNBUS_DEBUG_DUMP_BUF(master_write_buf, sizeof(signbus_network_header_t)+toSend);
            i2c_master_slave_write_sync(dest,sizeof(signbus_network_header_t)+toSend);
            i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
            toSend = 0;
        }
    }

    SIGNBUS_DEBUG("dest %02x data %p len %d -- COMPLETE\n", dest, data, len);
    return len;
}

// get_message is called either from a synchronous context, or from the
// callback path of an async request. In either case, this method can
// safely call blocking methods until it is ready to either return or
// call up the callback chain.
//
// This function will return the number of bytes received or < 0 for error.
// For async invocation, the return value is passed as the callback argument.
static int get_message(uint8_t* data, size_t len, uint8_t* src) {
    uint8_t done = 0;
    size_t lengthReceived = 0;
    uint16_t message_sequence_number;
    uint8_t message_source_address;

    // Mark async as inactive so this call stack can block
    async_active = false;

    //loop receiving packets until we get the whole datagram
    while(!done) {
        //wait and receive a packet
        if (!np.new) {
            yield_for(&np.new);
        }
        np.new = 0;

        //a new packet is in the packet buf

        //copy the packet into a header struct
        Packet p;
        memcpy(&p,packet_buf,I2C_MAX_LEN);

        if(lengthReceived == 0) {
            //this is the first packet
            //save the message_sequence_number
            message_sequence_number = p.header.sequence_number;
            message_source_address = p.header.src;
        } else {
            //this is not the first packet
            //is this the same message_sequence_number?
            if(message_sequence_number == p.header.sequence_number
                    && message_source_address == p.header.src) {
                //yes it is - proceed
            } else {
                //we should drop this packet
                continue;
            }
        }

        *src = p.header.src;

        //are there more fragments?
        uint8_t moreFragments = p.header.flags.is_fragment;
        uint16_t fragmentOffset = htons(p.header.fragment_offset);
        if(moreFragments) {
            //is there room to copy into the buffer?
            if(fragmentOffset + MAX_DATA_LEN > len) {
                //this is too long
                //just copy what we can and end
                uint16_t remainder = len - fragmentOffset;
                memcpy(data+fragmentOffset,p.data,remainder);
                lengthReceived += remainder;
                done = 1;
            } else {
                memcpy(data+fragmentOffset,p.data,MAX_DATA_LEN);
                lengthReceived += MAX_DATA_LEN;
            }
        } else {
            //is there room to copy into the buffer?
            if(fragmentOffset + (np.len - sizeof(signbus_network_header_t)) > len) {
                //this is too long
                //just copy what we can and end
                uint16_t remainder = len - fragmentOffset;
                memcpy(data+fragmentOffset,p.data,remainder);
                lengthReceived += remainder;
            } else {
                //copy the rest of the packet
                memcpy(data+fragmentOffset,p.data,(np.len - sizeof(signbus_network_header_t)));
                lengthReceived += np.len - sizeof(signbus_network_header_t);
            }

            //no more fragments end
            done = 1;
        }
    }

    SIGNBUS_DEBUG_DUMP_BUF(data, lengthReceived);

    if (async_callback != NULL) {
        // allow recursion
        signbus_app_callback_t* temp = async_callback;
        async_callback = NULL;
        temp(lengthReceived);
    }

    return lengthReceived;
}

// blocking receive call
int signbus_io_recv(
        size_t recv_buflen,
        uint8_t* recv_buf,
        uint8_t* src_address
        ) {

    async = false;

    int rc;
    rc = i2c_master_slave_listen();
    if (rc < 0) return rc;

    return get_message(recv_buf, recv_buflen, src_address);
}

// async receive call
int signbus_io_recv_async(
        signbus_app_callback_t callback,
        size_t recv_buflen,
        uint8_t* recv_buf,
        uint8_t* src
        ) {

    async_callback = callback;
    async_recv_buflen = recv_buflen;
    async_recv_buf = recv_buf;
    async_src_address = src;
    async_active = true;

    return i2c_master_slave_listen();
}



/******************************************************************************
 * Slave Read Section
 *
 * This is the code that's responsible for supporting I2C reads.
 * Currently, reads use the same Network Layer as the rest of signbus, but do
 * not follow any of the higher layers of the protocol.
 ******************************************************************************/

static uint32_t slave_read_data_bytes_remaining = 0;
static uint32_t slave_read_data_len = 0;
static Packet readPacket = {0};
static uint8_t* slave_read_data = NULL;
static signbus_app_callback_t* slave_read_callback = NULL;

static void signbus_iterate_slave_read(void) {

    // check if there is still data to send
    if (slave_read_data_bytes_remaining > 0) {


        // this message is a fragment if the entire data buffer cannot fit
        bool is_fragment = (slave_read_data_bytes_remaining > MAX_DATA_LEN);

        // calculate current offset into the data buffer
        uint16_t offset = slave_read_data_len - slave_read_data_bytes_remaining;

        // set packet header values
        readPacket.header.flags.is_fragment = is_fragment;
        readPacket.header.fragment_offset = htons(offset);

        // set the packet data field
        uint32_t data_len = slave_read_data_bytes_remaining;
        if (is_fragment) {
            // we cannot fit the whole data buffer. Copy as much as we can
            data_len = MAX_DATA_LEN;
        }
        memcpy(readPacket.data, &slave_read_data[offset], data_len);

        // update bytes remaining
        slave_read_data_bytes_remaining -= data_len;

        // copy the packet into the i2c slave read buffer
        memcpy(slave_read_buf, &readPacket, I2C_MAX_LEN);

        // enable the i2c slave read
        i2c_master_slave_enable_slave_read(I2C_MAX_LEN);

    } else {
        // all provided data has been sent! Do something about it
        if (slave_read_callback == NULL && slave_read_data_len > 0) {
            // do repitition of provided buffer... for legacy reasons
            slave_read_data_bytes_remaining = slave_read_data_len;
            signbus_iterate_slave_read();

        } else {
            // perform the callback!
            slave_read_callback(SUCCESS);
        }
    }
}

// provide data for a slave read
int signbus_io_set_read_buffer (uint8_t* data, uint32_t len) {
    // reads work just like writing messages, but only iterate through
    // fragments after someone has read the buffer. A callback will be sent
    // after the entire data buffer has been read if one has been provided.
    // It's up to the application layers to ensure that the master is reading
    // the correct number of bytes from the slave. For legacy code reasons,
    // after the buffer has been read, if no callback has been provided we set
    // the next read to start from the beginning of the buffer again. It's
    // expected that the application layer will call signbus_io_set_read_buffer
    // from the callback to provide new data

    // listen for i2c messages asynchronously
    int err = i2c_master_slave_listen();
    if (err < SUCCESS) {
        return err;
    }

    // sequence number is incremented once per data
    sequence_number++;

    // keep track of data and length
    slave_read_data_len = len;
    slave_read_data = data;

    // still need to send entire buffer
    slave_read_data_bytes_remaining = len;

    //calculate the number of packets we will have to send
    uint16_t numPackets;
    if(len % MAX_DATA_LEN) {
        numPackets = (len/MAX_DATA_LEN) + 1;
    } else {
        numPackets = (len/MAX_DATA_LEN);
    }

    // setup metadata for packets to be read
    readPacket.header.flags.version = 0x01;
    readPacket.header.src = this_device_address;
    readPacket.header.sequence_number = htons(sequence_number);

    //set the total length
    readPacket.header.length = htons((numPackets*sizeof(signbus_network_header_t))+len);

    // actually set up the fragmented read packet
    signbus_iterate_slave_read();
    return SUCCESS;
}

// provide callback to be performed when the slave read has completed all data
// provided (possibly across multiple packets). Callback is not cleared after
// use
void signbus_io_set_read_callback (signbus_app_callback_t* callback) {
    slave_read_callback = callback;
}

