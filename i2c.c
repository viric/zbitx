/*
 * i2cBitBangingBus.cpp
 *
 *  Created on: 06.03.2015
 *      Author: "Marek Wyborski"
 */

#include <fcntl.h> 
#include <stdio.h>
#include <inttypes.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <assert.h>
#include "i2cbb.h"

static int debug = 1;

static int i2c_fd = -1;
void i2cbb_init(uint8_t pin_number_sda, uint8_t pin_number_scl)
{
}

void ensure_open()
{
	if (i2c_fd == -1)
	{
		i2c_fd = open("/dev/i2c-3", O_RDWR);
		if (i2c_fd == -1)
		{
			fprintf(stderr, "Failed to open I2C 3\n");
			abort();
		}
	}
}

// KERNEL-LIKE I2C METHODS

// This executes the SMBus “write byte” protocol, returning negative errno else zero on success.
int32_t i2cbb_write_byte_data(uint8_t i2c_address, uint8_t command, uint8_t value) {

	ensure_open();

	struct i2c_smbus_ioctl_data d;
	d.read_write = I2C_SMBUS_WRITE;
	d.command = command;
	d.size = I2C_SMBUS_BYTE;
	union i2c_smbus_data busdata;
	busdata.byte = value;

	int res = ioctl(i2c_fd, I2C_SMBUS, &d);
  if (res < 0) {
			fprintf(stderr, "Failed to ioctl I2C_SMBUS %d\n", i2c_address);
			return -1;
  };
	if (debug)
	  fprintf(stderr, "i2c_write_byte_data ioctl() res %d\n", res);
	return 0;
}

int32_t i2cbb_write_i2c_block_data(uint8_t i2c_address, uint8_t command, 
	uint8_t length, const uint8_t * values)
{
	ensure_open();

	uint8_t buf[256];

  assert(length < 256);
	buf[0] = command;
	buf[1] = length;
	memcpy(&buf[2], values, length);

	// Write command and all
	struct i2c_msg msg;
	msg.addr = i2c_address;
	msg.flags = 0;
	msg.len = 1;
	msg.buf = buf;

	struct i2c_rdwr_ioctl_data idata;
	idata.msgs = &msg;
	idata.nmsgs = 1;

	int res = ioctl(i2c_fd, I2C_RDWR, &idata);
  if (res < 0) {
			fprintf(stderr, "Failed to ioctl I2C_RDWR write i2c block data %x\n", i2c_address);
			return -1;
  };
	if (debug)
	  fprintf(stderr, "i2c_write_i2c_block_data ioctl() length %d res %d\n", length, res);
}

// This executes the SMBus “block read” protocol, returning negative errno else the number
// of data bytes in the slave's response.
int32_t i2cbb_read_i2c_block_data(uint8_t i2c_address, uint8_t command, uint8_t length,
        uint8_t* values) {
	ensure_open();

	struct i2c_smbus_ioctl_data d;
	d.read_write = I2C_SMBUS_READ;
	d.command = command;
	d.size = I2C_SMBUS_I2C_BLOCK_DATA;

	union i2c_smbus_data busdata;
	busdata.block[0] = length;
	d.data = &busdata;
	assert(length < 32);

	int res = ioctl(i2c_fd, I2C_SMBUS, &d);
  if (res < 0) {
			fprintf(stderr, "Failed to ioctl I2C_SMBUS read i2c block data %x\n", i2c_address);
			return -1;
  };
	if (debug)
	  fprintf(stderr, "i2c_read_i2c_block_data ioctl() length %d res %d\n", length, res);

	memcpy(values, &busdata.block[1], length);
  return length;
}

int32_t i2cbb_read_rll(uint8_t i2c_address, uint8_t* values) {
	ensure_open();

	uint8_t buf[256];

	struct i2c_msg msg;
	msg.addr = i2c_address;
	msg.flags = I2C_M_RD;
	msg.len = 255;
	msg.buf = buf;

	struct i2c_rdwr_ioctl_data idata;
	idata.msgs = &msg;
	idata.nmsgs = 1;

	int res = ioctl(i2c_fd, I2C_RDWR, &idata);
  if (res < 0) {
			fprintf(stderr, "Failed to ioctl I2C_SMBUS read rll %x\n", i2c_address);
			return -1;
  };

	int rlength = buf[0];
	memcpy(values, &buf[1], rlength);

	if (debug)
	  fprintf(stderr, "i2c_read_rll ioctl() length %d res %d\n", rlength, res);
  return rlength;
}
