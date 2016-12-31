/**HEADER********************************************************************
 *
 * Copyright (c) 2008, 2013 - 2014 Freescale Semiconductor;
 * All Rights Reserved
 *
 * Copyright (c) 1989-2008 ARC International;
 * All Rights Reserved
 *
 *************************************************************************** 
 *
 * THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
 * IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 **************************************************************************
 *
 * $FileName: ehci_dev_main.h$
 * $Version : 3.8.7.0$
 * $Date    : Jun-7-2012$
 *
 * Comments:
 *
 *  This file contains the defines, externs and
 *  data structure definitions required by the VUSB_HS Device
 *  driver.
 *
 *END*********************************************************************/
#ifndef __ehci_dev_main_h__
#define __ehci_dev_main_h__ 1

void _usb_dci_usbhs_free_dTD(usb_device_handle, void*);
uint8_t _usb_dci_usbhs_add_dTD(usb_device_handle, xd_struct_t*);
void _usb_dci_usbhs_process_tr_complete(usb_device_handle);
void _usb_dci_usbhs_process_reset(usb_device_handle);
void _usb_dci_usbhs_process_tr_complete(usb_device_handle);
void _usb_dci_usbhs_process_suspend(usb_device_handle);
void _usb_dci_usbhs_process_SOF(usb_device_handle);
void _usb_dci_usbhs_process_port_change(usb_device_handle);
void _usb_dci_usbhs_process_error(usb_device_handle);
usb_status usb_dci_usbhs_set_test_mode(usb_device_handle, uint16_t);
void _usb_dci_usbhs_chip_initialize(usb_device_handle);

usb_status usb_dci_usbhs_preinit(usb_device_handle, usb_device_handle *);
usb_status usb_dci_usbhs_init(uint8_t, usb_device_handle);
usb_status usb_dci_usbhs_send_data(usb_device_handle, xd_struct_t*);
usb_status usb_dci_usbhs_recv_data(usb_device_handle, xd_struct_t*);
usb_status usb_dci_usbhs_cancel_transfer(usb_device_handle, uint8_t, uint8_t);
usb_status usb_dci_usbhs_set_address(usb_device_handle, uint8_t);
usb_status usb_dci_usbhs_shutdown(usb_device_handle);
usb_status usb_dci_usbhs_get_setup_data(usb_device_handle, uint8_t, uint8_t *);
usb_status usb_dci_usbhs_assert_resume(usb_device_handle);
usb_status usb_dci_usbhs_init_endpoint(usb_device_handle, xd_struct_t*);
usb_status usb_dci_usbhs_stall_endpoint(usb_device_handle, uint8_t, uint8_t);
usb_status usb_dci_usbhs_unstall_endpoint(usb_device_handle, uint8_t, uint8_t);
usb_status usb_dci_usbhs_deinit_endpoint(usb_device_handle, uint8_t, uint8_t);
usb_status usb_dci_usbhs_get_endpoint_status(usb_device_handle, uint8_t, uint16_t *);
usb_status usb_dci_usbhs_set_endpoint_status(usb_device_handle, uint8_t, uint16_t);
usb_status usb_dci_usbhs_get_transfer_status(usb_device_handle, uint8_t, uint8_t);
usb_status usb_dci_usbhs_get_status(usb_device_handle, uint8_t, uint16_t *);
usb_status usb_dci_usbhs_set_status(usb_device_handle, uint8_t, uint16_t);
usb_status usb_dci_usbhs_alloc_xd(usb_device_handle, xd_struct_t**);

#endif
/* EOF */
