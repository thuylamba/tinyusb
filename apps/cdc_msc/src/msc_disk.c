/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include <inttypes.h>
#include <stdlib.h>
#include "bsp/board.h"
#include "tusb.h"
#include "printf.h"
#include "f1c100s/reg-ccu.h"
#include "sys-clock.h"
#include "f1c100s-gpio.h"
#include "f1c100s-sdc.h"
#include "sdcard.h"

#if CFG_TUD_MSC

// #define CFG_EXAMPLE_MSC_READONLY
#define SDCARD_LUN_0_MAX_BLOCK 500000

// whether host does safe-eject
static bool ejected = false;
static bool initial = false;

enum {
	SDCARD_FATFS = 0,
	SDCARD_SECFS = 1,
	SDCARD_MAX
};
static sdcard_t sdcard[SDCARD_MAX] = {
	{
		.sdc_base = F1C100S_SDC0_BASE,
		.voltage = MMC_VDD_27_36,
		.width = MMC_BUS_WIDTH_1,
		.clock = 48000000,
		.version = MMC_VERSION_SD_2
	}, 
	{
		.sdc_base = F1C100S_SDC0_BASE,
    .voltage = MMC_VDD_27_36,
    .width = MMC_BUS_WIDTH_1,
		.clock = 48000000,
		.version = MMC_VERSION_SD_2	
	}
};

static void
SD_IO_Init(void) {
	gpio_init(GPIOF, PIN0 | PIN1 | PIN2 | PIN3 | PIN4 | PIN5, GPIO_MODE_AF2, GPIO_PULL_NONE, GPIO_DRV_3);
}
bool disk_initialize(uint8_t lun)
{
  clk_enable(CCU_BUS_CLK_GATE0, 8);
  clk_enable(CCU_BUS_SOFT_RST0, 8);
  //
	SD_IO_Init();
  //
  if (sdcard_detect(&sdcard[0]) == 0)
		return false;

	memcpy(&sdcard[1], &sdcard[0], sizeof(sdcard_t));
	// init sdcard[0]
	if(sdcard[0].blk_cnt >= SDCARD_LUN_0_MAX_BLOCK) 
	{
		sdcard[0].blk_cnt = SDCARD_LUN_0_MAX_BLOCK;
		sdcard[0].capacity = sdcard[0].read_bl_len * SDCARD_LUN_0_MAX_BLOCK;
	}
	// init sdcard[1]
	sdcard[1].blk_cnt -= sdcard[0].blk_cnt;
	sdcard[1].capacity -= sdcard[0].capacity;
	for(int i = 0; i < sizeof(sdcard) / sizeof(sdcard_t); i++) 
	{
		lprintf("sdcard[%d].read_bl_len = [%d]\n", i, sdcard[i].read_bl_len);
		lprintf("sdcard[%d].write_bl_len = [%d]\n", i, sdcard[i].write_bl_len);
		lprintf("sdcard[%d].blk_cnt = %" PRId64 "\n", i, sdcard[i].blk_cnt);
		lprintf("sdcard[%d].capacity = %" PRId64 "\n", i, sdcard[i].capacity);
		lprintf("sdcard[%d].high_capacity = %d\n", i, sdcard[i].high_capacity);
		lprintf("sdcard[%d].tran_speed = %d\n", i, sdcard[i].tran_speed);
	}
	return true;
}
// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
  (void) lun;

  const char vid[] = "TinyUSB";
  const char pid[] = "Mass Storage";
  const char rev[] = "1.0";

  memcpy(vendor_id  , vid, strlen(vid));
  memcpy(product_id , pid, strlen(pid));
  memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
  (void) lun;

  // RAM disk is ready until ejected
  if (ejected) {
    // Additional Sense 3A-00 is NOT_FOUND
    tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
    return false;
  }

  return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
	if(initial == false)
	{
		if(disk_initialize(lun))
		{
			initial = true;
			lprintf("Init sdcard [%d] is done...\n", lun);
		}
	}
	*block_count = sdcard[lun].blk_cnt;
	*block_size = sdcard[lun].read_bl_len;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
  (void) lun;
  (void) power_condition;

  if ( load_eject )
  {
    if (start)
    {
      // load disk storage
    }else
    {
      // unload disk storage
      ejected = true;
    }
  }

  return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
  // out of ramdisk
  if ( lba >= sdcard[lun].blk_cnt ) return -1;
	// read
	uint64_t count = bufsize / sdcard[lun].read_bl_len;
	uint8_t *buff = (uint8_t *)malloc(bufsize * sizeof(uint8_t));
	uint64_t result = sdcard_read(&sdcard[lun], buff, lba, count);
	if(result != count) {
		free(buff);
		return -1;
	}
	memcpy(buffer, buff + offset, bufsize);
	free(buff);
  return (int32_t) bufsize;
}

bool tud_msc_is_writable_cb (uint8_t lun)
{
  (void) lun;

#ifdef CFG_EXAMPLE_MSC_READONLY
  return false;
#else
  return true;
#endif
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
  // out of ramdisk
	if ( lba >= sdcard[lun].blk_cnt ) return -1;
	
#ifndef CFG_EXAMPLE_MSC_READONLY
	// write
	uint64_t count = bufsize / sdcard[lun].write_bl_len;
	uint64_t result = sdcard_write(&sdcard[lun], (uint8_t*)buffer, lba, count);
	if(result != count) 
	{
		lprintf("[%s][%d] Write sdcard is failed..\n", __func__, __LINE__);
	}
#else
  (void) lba; (void) offset; (void) buffer;
#endif

  return (int32_t) bufsize;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
  // read10 & write10 has their own callback and MUST not be handled here

  void const* response = NULL;
  int32_t resplen = 0;

  // most scsi handled is input
  bool in_xfer = true;

  switch (scsi_cmd[0])
  {
		case SCSI_CMD_TEST_UNIT_READY:
			break;
		case SCSI_CMD_INQUIRY:
			break;
		case SCSI_CMD_MODE_SELECT_6:
			break;
		case SCSI_CMD_MODE_SENSE_6:
			break;
		case SCSI_CMD_START_STOP_UNIT:
			break;
		case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
			break;
		case SCSI_CMD_READ_CAPACITY_10:
			break;
		case SCSI_CMD_REQUEST_SENSE:
			break;
		case SCSI_CMD_READ_FORMAT_CAPACITY:
			break;
		case SCSI_CMD_READ_10:
			break;
		case SCSI_CMD_WRITE_10:
			break;
    default:
      // Set Sense = Invalid Command Operation
      tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

      // negative means error -> tinyusb could stall and/or response with failed status
      resplen = -1;
    break;
  }

  // return resplen must not larger than bufsize
  if ( resplen > (int32_t)bufsize ) resplen = bufsize;

  if ( response && (resplen > 0) )
  {
    if(in_xfer)
    {
      memcpy(buffer, response, (size_t) resplen);
    }else
    {
      // SCSI output
    }
  }

  return (int32_t) resplen;
}

#endif
