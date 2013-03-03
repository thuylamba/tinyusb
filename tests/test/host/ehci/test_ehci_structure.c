/*
 * test_ehci.c
 *
 *  Created on: Feb 27, 2013
 *      Author: hathach
 */

/*
 * Software License Agreement (BSD License)
 * Copyright (c) 2012, hathach (tinyusb.net)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the tiny usb stack.
 */

#include "unity.h"
#include "tusb_option.h"
#include "errors.h"
#include "binary.h"

#include "hal.h"
#include "mock_osal.h"
#include "ehci.h"

extern ehci_data_t ehci_data;

LPC_USB0_Type lpc_usb0;
LPC_USB1_Type lpc_usb1;

//--------------------------------------------------------------------+
// Setup/Teardown + helper declare
//--------------------------------------------------------------------+
int8_t first_pos_of_high_bit(uint32_t value);
uint8_t number_of_high_bits(uint32_t value);

#define BITFIELD_OFFSET_OF_MEMBER(struct_type, member, bitfield_member) \
  ({\
    uint32_t value=0;\
    struct_type str;\
    memclr_((void*)&str, sizeof(struct_type));\
    str.member.bitfield_member = 1;\
    memcpy(&value, (void*)&str.member, sizeof(str.member));\
    first_pos_of_high_bit( value );\
  })

#define BITFIELD_OFFSET_OF_UINT32(struct_type, offset, bitfield_member) \
  ({\
    struct_type str;\
    memclr_(&str, sizeof(struct_type));\
    str.bitfield_member = 1;\
    first_pos_of_high_bit( ((uint32_t*) &str)[offset] );\
  })

void setUp(void)
{
  memclr_(&lpc_usb0, sizeof(LPC_USB0_Type));
  memclr_(&lpc_usb1, sizeof(LPC_USB1_Type));
}

void tearDown(void)
{
}

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
void test_struct_alignment(void)
{
  TEST_ASSERT_EQUAL( 32, __alignof__(ehci_qhd_t) );
//  TEST_ASSERT_EQUAL( 32, __alignof__(ehci_qtd_t) ); ehci_qtd_t is used to declare overlay variable in qhd --> cannot declare with ATTR_ALIGNED(32)

  TEST_ASSERT_EQUAL( 32, __alignof__(ehci_itd_t) );
  TEST_ASSERT_EQUAL( 32, __alignof__(ehci_sitd_t) );

}

void test_struct_size(void)
{
  TEST_ASSERT_EQUAL( 64, sizeof(ehci_qhd_t) );
  TEST_ASSERT_EQUAL( 32, sizeof(ehci_qtd_t) );

  TEST_ASSERT_EQUAL( 64, sizeof(ehci_itd_t) );
  TEST_ASSERT_EQUAL( 32, sizeof(ehci_sitd_t) );

  TEST_ASSERT_EQUAL( 4, sizeof(ehci_link_t) );
}

//--------------------------------------------------------------------+
// EHCI Data Structure
//--------------------------------------------------------------------+
void test_qtd_structure(void)
{
  TEST_ASSERT_EQUAL( 0, offsetof(ehci_qtd_t, next));
  TEST_ASSERT_EQUAL( 4, offsetof(ehci_qtd_t, alternate));

  //------------- Word 2 -------------//
  TEST_ASSERT_EQUAL( 0, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, pingstate_err) );
  TEST_ASSERT_EQUAL( 1, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, split_state) );
  TEST_ASSERT_EQUAL( 2, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, missed_uframe));
  TEST_ASSERT_EQUAL( 3, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, xact_err) );
  TEST_ASSERT_EQUAL( 4, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, babble_err) );
  TEST_ASSERT_EQUAL( 5, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, buffer_err) );
  TEST_ASSERT_EQUAL( 6, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, halted) );
  TEST_ASSERT_EQUAL( 7, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, active) );
  TEST_ASSERT_EQUAL( 8, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, pid) );
  TEST_ASSERT_EQUAL( 10, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, cerr) );
  TEST_ASSERT_EQUAL( 12, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, current_page) );
  TEST_ASSERT_EQUAL( 15, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, int_on_complete) );
  TEST_ASSERT_EQUAL( 16, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, total_bytes) );
  TEST_ASSERT_EQUAL( 31, BITFIELD_OFFSET_OF_UINT32(ehci_qtd_t, 2, data_toggle) );

  TEST_ASSERT_EQUAL( 12, offsetof(ehci_qtd_t, buffer));
}

void test_qhd_structure(void)
{
  TEST_ASSERT_EQUAL( 0, offsetof(ehci_qhd_t, next));

  //------------- Word 1 -------------//
  TEST_ASSERT_EQUAL( 0, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 1, device_address) );
  TEST_ASSERT_EQUAL( 7, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 1, inactive_next_xact) );
  TEST_ASSERT_EQUAL( 8, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 1, endpoint_number) );
  TEST_ASSERT_EQUAL( 12, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 1, endpoint_speed) );
  TEST_ASSERT_EQUAL( 14, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 1, data_toggle_control) );
  TEST_ASSERT_EQUAL( 15, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 1, head_list_flag) );
  TEST_ASSERT_EQUAL( 16, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 1, max_package_size) );
  TEST_ASSERT_EQUAL( 27, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 1, non_hs_control_endpoint) );
  TEST_ASSERT_EQUAL( 28, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 1, nak_count_reload) );

  //------------- Word 2 -------------//
  TEST_ASSERT_EQUAL( 0, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 2, smask) );
  TEST_ASSERT_EQUAL( 8, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 2, cmask) );
  TEST_ASSERT_EQUAL( 16, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 2, hub_address) );
  TEST_ASSERT_EQUAL( 23, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 2, port_number) );
  TEST_ASSERT_EQUAL( 30, BITFIELD_OFFSET_OF_UINT32(ehci_qhd_t, 2, mult) );

  TEST_ASSERT_EQUAL( 3*4, offsetof(ehci_qhd_t, qtd_addr));
  TEST_ASSERT_EQUAL( 4*4, offsetof(ehci_qhd_t, qtd_overlay));
}

void test_itd_structure(void)
{
  TEST_ASSERT_EQUAL( 0, offsetof(ehci_itd_t, next));

  // Each Transaction Word
  TEST_ASSERT_EQUAL( 0  , BITFIELD_OFFSET_OF_MEMBER(ehci_itd_t, xact[0], offset) );
  TEST_ASSERT_EQUAL( 12 , BITFIELD_OFFSET_OF_MEMBER(ehci_itd_t, xact[0], page_select) );
  TEST_ASSERT_EQUAL( 15 , BITFIELD_OFFSET_OF_MEMBER(ehci_itd_t, xact[0], int_on_complete) );
  TEST_ASSERT_EQUAL( 16 , BITFIELD_OFFSET_OF_MEMBER(ehci_itd_t, xact[0], length) );
  TEST_ASSERT_EQUAL( 28 , BITFIELD_OFFSET_OF_MEMBER(ehci_itd_t, xact[0], error) );
  TEST_ASSERT_EQUAL( 29 , BITFIELD_OFFSET_OF_MEMBER(ehci_itd_t, xact[0], babble_err) );
  TEST_ASSERT_EQUAL( 30 , BITFIELD_OFFSET_OF_MEMBER(ehci_itd_t, xact[0], buffer_err) );
  TEST_ASSERT_EQUAL( 31 , BITFIELD_OFFSET_OF_MEMBER(ehci_itd_t, xact[0], active) );

  TEST_ASSERT_EQUAL( 9*4, offsetof(ehci_itd_t, BufferPointer));
}

void test_sitd_structure(void)
{
  TEST_ASSERT_EQUAL( 0, offsetof(ehci_sitd_t, next));

  //------------- Word 1 -------------//
  TEST_ASSERT_EQUAL( 0, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 1, device_address) );
  TEST_ASSERT_EQUAL( 8, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 1, endpoint_number) );
  TEST_ASSERT_EQUAL( 16, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 1, hub_address) );
  TEST_ASSERT_EQUAL( 24, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 1, port_number) );
  TEST_ASSERT_EQUAL( 31, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 1, direction) );

  //------------- Word 2 -------------//
  TEST_ASSERT_EQUAL( 4*2, offsetof(ehci_sitd_t, smask));
  TEST_ASSERT_EQUAL( 4*2+1, offsetof(ehci_sitd_t, cmask));

  //------------- Word 3 -------------//
  TEST_ASSERT_EQUAL( 1, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 3, split_state) );
  TEST_ASSERT_EQUAL( 2, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 3, missed_uframe));
  TEST_ASSERT_EQUAL( 3, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 3, xact_err) );
  TEST_ASSERT_EQUAL( 4, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 3, babble_err) );
  TEST_ASSERT_EQUAL( 5, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 3, buffer_err) );
  TEST_ASSERT_EQUAL( 6, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 3, error) );
  TEST_ASSERT_EQUAL( 7, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 3, active) );

  TEST_ASSERT_EQUAL( 8, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 3, cmask_progress) );
  TEST_ASSERT_EQUAL( 16, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 3, total_bytes) );
  TEST_ASSERT_EQUAL( 30, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 3, page_select) );
  TEST_ASSERT_EQUAL( 31, BITFIELD_OFFSET_OF_UINT32(ehci_sitd_t, 3, int_on_complete) );

  //------------- Word 4 -------------//
  TEST_ASSERT_EQUAL( 4*4, offsetof(ehci_sitd_t, buffer));

  TEST_ASSERT_EQUAL( 4*6, offsetof(ehci_sitd_t, back));
}

//--------------------------------------------------------------------+
// EHCI Register Interface
//--------------------------------------------------------------------+
void test_register_offset(void)
{
  TEST_ASSERT_EQUAL( 0x00, offsetof(ehci_registers_t, usb_cmd));
  TEST_ASSERT_EQUAL( 0x04, offsetof(ehci_registers_t, usb_sts));
  TEST_ASSERT_EQUAL( 0x08, offsetof(ehci_registers_t, usb_int_enable));
  TEST_ASSERT_EQUAL( 0x0C, offsetof(ehci_registers_t, frame_index));
  TEST_ASSERT_EQUAL( 0x10, offsetof(ehci_registers_t, ctrl_ds_seg));
  TEST_ASSERT_EQUAL( 0x14, offsetof(ehci_registers_t, periodic_list_base));
  TEST_ASSERT_EQUAL( 0x18, offsetof(ehci_registers_t, async_list_base));
  TEST_ASSERT_EQUAL( 0x1C, offsetof(ehci_registers_t, tt_control)); // NXP specific
  TEST_ASSERT_EQUAL( 0x40, offsetof(ehci_registers_t, config_flag)); // NXP not used
  TEST_ASSERT_EQUAL( 0x44, offsetof(ehci_registers_t, portsc));
}

void test_register_usbcmd(void)
{
  TEST_ASSERT_EQUAL( 0  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_cmd_bit, run_stop) );
  TEST_ASSERT_EQUAL( 1  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_cmd_bit, reset) );
  TEST_ASSERT_EQUAL( 2  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_cmd_bit, framelist_size) );
  TEST_ASSERT_EQUAL( 4  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_cmd_bit, periodic_enable) );
  TEST_ASSERT_EQUAL( 5  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_cmd_bit, async_enable) );
  TEST_ASSERT_EQUAL( 6  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_cmd_bit, advacne_async) );
  TEST_ASSERT_EQUAL( 7  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_cmd_bit, light_reset) );
  TEST_ASSERT_EQUAL( 8  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_cmd_bit, async_park) );
  TEST_ASSERT_EQUAL( 11 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_cmd_bit, async_park_enable) );
  TEST_ASSERT_EQUAL( 15 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_cmd_bit, nxp_framelist_size_msb) );
  TEST_ASSERT_EQUAL( 16 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_cmd_bit, int_threshold) );
}

void test_register_usbsts(void)
{
  TEST_ASSERT_EQUAL( 0  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, usb));
  TEST_ASSERT_EQUAL( 1  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, usb_error));
  TEST_ASSERT_EQUAL( 2  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, port_change_detect));
  TEST_ASSERT_EQUAL( 3  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, framelist_rollover));
  TEST_ASSERT_EQUAL( 4  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, pci_host_system_error));
  TEST_ASSERT_EQUAL( 5  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, async_advance));
  TEST_ASSERT_EQUAL( 7  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, nxp_sof_received));
  TEST_ASSERT_EQUAL( 12 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, hc_halted));
  TEST_ASSERT_EQUAL( 13 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, reclamation));
  TEST_ASSERT_EQUAL( 14 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, period_schedule_status));
  TEST_ASSERT_EQUAL( 15 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, async_schedule_status));
  TEST_ASSERT_EQUAL( 18 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, nxp_int_async));
  TEST_ASSERT_EQUAL( 19 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_sts_bit, nxp_int_period));
}

void test_register_usbint(void)
{
  TEST_ASSERT_EQUAL( 0  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_int_enable_bit, usb));
  TEST_ASSERT_EQUAL( 1  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_int_enable_bit, usb_error));
  TEST_ASSERT_EQUAL( 2  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_int_enable_bit, port_change_detect));
  TEST_ASSERT_EQUAL( 3  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_int_enable_bit, framelist_rollover));
  TEST_ASSERT_EQUAL( 4  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_int_enable_bit, pci_host_system_error));
  TEST_ASSERT_EQUAL( 5  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_int_enable_bit, async_advance));
  TEST_ASSERT_EQUAL( 7  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_int_enable_bit, nxp_sof_received));
  TEST_ASSERT_EQUAL( 18 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_int_enable_bit, nxp_int_async));
  TEST_ASSERT_EQUAL( 19 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, usb_int_enable_bit, nxp_int_period));

}

void test_register_portsc(void)
{
  TEST_ASSERT_EQUAL( 0  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, current_connect_status));
  TEST_ASSERT_EQUAL( 1  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, connect_status_change));
  TEST_ASSERT_EQUAL( 2  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, port_enable));
  TEST_ASSERT_EQUAL( 3  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, port_enable_change));
  TEST_ASSERT_EQUAL( 4  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, over_current_active));
  TEST_ASSERT_EQUAL( 5  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, over_current_change));
  TEST_ASSERT_EQUAL( 6  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, force_port_resume));
  TEST_ASSERT_EQUAL( 7  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, suspend));
  TEST_ASSERT_EQUAL( 8  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, port_reset));
  TEST_ASSERT_EQUAL( 9  , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, nxp_highspeed_status));
  TEST_ASSERT_EQUAL( 10 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, line_status));
  TEST_ASSERT_EQUAL( 12 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, port_power));
  TEST_ASSERT_EQUAL( 13 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, port_owner));
  TEST_ASSERT_EQUAL( 14 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, port_indicator_control));
  TEST_ASSERT_EQUAL( 16 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, port_test_control));
  TEST_ASSERT_EQUAL( 20 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, wake_on_connect_enable));
  TEST_ASSERT_EQUAL( 21 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, wake_on_disconnect_enable));
  TEST_ASSERT_EQUAL( 22 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, wake_on_over_current_enable));
  TEST_ASSERT_EQUAL( 23 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, nxp_phy_clock_disable));
  TEST_ASSERT_EQUAL( 24 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, nxp_port_force_fullspeed));
  TEST_ASSERT_EQUAL( 26 , BITFIELD_OFFSET_OF_MEMBER(ehci_registers_t, portsc_bit, nxp_port_speed));
}

//--------------------------------------------------------------------+
// EHCI Data Organization
//--------------------------------------------------------------------+
void test_ehci_data(void)
{
  for(uint32_t i=0; i<TUSB_CFG_HOST_CONTROLLER_NUM; i++)
  {
    uint8_t hostid = i+TUSB_CFG_HOST_CONTROLLER_START_INDEX;
    TEST_ASSERT_BITS_LOW(4096-1, (uint32_t)get_period_frame_list(hostid) );
  }

  TEST_IGNORE();
}

//--------------------------------------------------------------------+
// Initialization
//--------------------------------------------------------------------+
void test_hcd_init_data(void)
{
  uint32_t random_data = 0x1234;
  memcpy(&ehci_data, &random_data, sizeof(random_data));

  hcd_init();

  //------------- check memory data -------------//
  for(uint32_t i=0; i<sizeof(ehci_data.device); i++)
    TEST_ASSERT_EQUAL_HEX8(0, ((uint8_t*) ehci_data.device)[i] );
}

void test_hcd_init_usbint(void)
{
  hcd_init();

  for(uint32_t i=0; i<TUSB_CFG_HOST_CONTROLLER_NUM; i++)
  {
    ehci_registers_t* regs = get_operational_register(i+TUSB_CFG_HOST_CONTROLLER_START_INDEX);

    //------------- USB INT Enable-------------//
    TEST_ASSERT(regs->usb_int_enable_bit.usb_error);
    TEST_ASSERT(regs->usb_int_enable_bit.port_change_detect);
    TEST_ASSERT(regs->usb_int_enable_bit.async_advance);

    TEST_ASSERT_FALSE(regs->usb_int_enable_bit.framelist_rollover);
    TEST_ASSERT_FALSE(regs->usb_int_enable_bit.pci_host_system_error);

    TEST_ASSERT_FALSE(regs->usb_int_enable_bit.usb);
    TEST_ASSERT_TRUE(regs->usb_int_enable_bit.nxp_int_async);
    TEST_ASSERT_TRUE(regs->usb_int_enable_bit.nxp_int_period);

    TEST_IGNORE_MESSAGE("not use nxp int async/period, use usbint instead");
  }
}

void test_hcd_init_async_list(void)
{
  hcd_init();

  for(uint32_t i=0; i<TUSB_CFG_HOST_CONTROLLER_NUM; i++)
  {
    uint8_t hostid                = i+TUSB_CFG_HOST_CONTROLLER_START_INDEX;

    ehci_registers_t * const regs = get_operational_register(hostid);
    ehci_qhd_t * const async_head = get_async_head(hostid);

    TEST_ASSERT_EQUAL_HEX(async_head, regs->async_list_base);

    TEST_ASSERT_EQUAL_HEX(async_head, align32(async_head) );
    TEST_ASSERT_EQUAL(EHCI_QUEUE_ELEMENT_QHD, async_head->next.type);
    TEST_ASSERT_FALSE(async_head->next.terminate);

    TEST_ASSERT(async_head->head_list_flag);
    TEST_ASSERT(async_head->qtd_overlay.next.terminate);
    TEST_ASSERT(async_head->qtd_overlay.alternate.terminate);
    TEST_ASSERT(async_head->qtd_overlay.halted);
  }
}

void test_hcd_init_period_list(void)
{
#if EHCI_PERIODIC_LIST
  hcd_init();

  for(uint32_t i=0; i<TUSB_CFG_HOST_CONTROLLER_NUM; i++)
  {
    uint8_t           const hostid      = i+TUSB_CFG_HOST_CONTROLLER_START_INDEX;
    ehci_registers_t* const regs        = get_operational_register(hostid);
    ehci_qhd_t *      const period_head = get_period_head(hostid);
    ehci_link_t *     const framelist   = get_period_frame_list(hostid);

    TEST_ASSERT_EQUAL_HEX( (uint32_t) framelist, regs->periodic_list_base);
    for(uint32_t list_idx=0; list_idx < EHCI_FRAMELIST_SIZE; list_idx++)
    {
      TEST_ASSERT_EQUAL_HEX( (uint32_t) period_head, align32((uint32_t)framelist[list_idx].address) );
      TEST_ASSERT_FALSE(framelist[list_idx].terminate);
      TEST_ASSERT_EQUAL(EHCI_QUEUE_ELEMENT_QHD, framelist[list_idx].type);
    }

    TEST_ASSERT(period_head->smask)
    TEST_ASSERT_TRUE(period_head->next.terminate);
    TEST_ASSERT(period_head->qtd_overlay.next.terminate);
    TEST_ASSERT(period_head->qtd_overlay.alternate.terminate);
    TEST_ASSERT(period_head->qtd_overlay.halted);
  }
#endif
}

void test_hcd_init_tt_control(void)
{
  hcd_init();

  for(uint32_t i=0; i<TUSB_CFG_HOST_CONTROLLER_NUM; i++)
  {
    uint8_t           const hostid      = i+TUSB_CFG_HOST_CONTROLLER_START_INDEX;
    ehci_registers_t* const regs        = get_operational_register(hostid);

    TEST_ASSERT_EQUAL(0, regs->tt_control);
  }
}

void test_hcd_init_usbcmd(void)
{
  hcd_init();

  for(uint32_t i=0; i<TUSB_CFG_HOST_CONTROLLER_NUM; i++)
  {
    uint8_t           const hostid      = i+TUSB_CFG_HOST_CONTROLLER_START_INDEX;
    ehci_registers_t* const regs        = get_operational_register(hostid);

    TEST_ASSERT(regs->usb_cmd_bit.async_enable);

#if EHCI_PERIODIC_LIST
    TEST_ASSERT(regs->usb_cmd_bit.periodic_enable);
#else
    TEST_ASSERT_FALSE(regs->usb_cmd_bit.periodic_enable);
#endif

    //------------- Framelist size (NXP specific) -------------//
    TEST_ASSERT_BITS(BIN8(11), EHCI_CFG_FRAMELIST_SIZE_BITS, regs->usb_cmd_bit.framelist_size);
    TEST_ASSERT_EQUAL(EHCI_CFG_FRAMELIST_SIZE_BITS >> 2, regs->usb_cmd_bit.nxp_framelist_size_msb);
  }

}
//--------------------------------------------------------------------+
// Helper
//--------------------------------------------------------------------+
int8_t first_pos_of_high_bit(uint32_t value)
{
  for (int8_t i=0; i<32; i++)
  {
    if (value & BIT_(i))
      return i;
  }
  return (-1);
}

uint8_t number_of_high_bits(uint32_t value)
{
  uint8_t result=0;
  for(uint8_t i=0; i<32; i++)
  {
    if (value & BIT_(i))
      result++;
  }
  return result;
}