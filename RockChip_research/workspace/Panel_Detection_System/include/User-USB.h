/*
 * @Author: YNJin YNJin0902@icloud.com
 * @Date: 2025-12-18
 * @LastEditors: YNJin YNJin0902@icloud.com
 * @LastEditTime: 
 * @FilePath: 
 * @Description: For CA310 USB communited, needed linux usb driver
 *               USB Version : 2.0
 */

#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


/* ====================== Function ====================== */
void check_device_status(void);
int usb_open_device(void);
void usb_close_device(void);

void zero_cal_all_channels(void);
int init_one_channel(int ch);

int send_command_ascii(const char *cmd);
int send_command_ascii_with_resp(const char *cmd, char *out_resp, int out_size);