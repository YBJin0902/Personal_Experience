/*
 * @Author: YNJin YNJin0902@icloud.com
 * @Date: 2025-12-18
 * @LastEditors: YNJin YNJin0902@icloud.com
 * @LastEditTime: 
 * @FilePath: 
 * @Description: For CA310 USB communited source code, needed linux usb driver
 *               USB Version : 2.0
 */

#include "User-USB.h"


/* ====================== Define ====================== */

// ====== Minolta COLOR ANALYZER (CA-310) ======
#define VENDOR_ID   0x0686      // Minolta
#define PRODUCT_ID  0x1002      // COLOR ANALYZER
#define IFACE_NUM   0           // Interface 0

// ====== USB End Point Device ======
#define EP_OUT      0x02        // EP 2 OUT
#define EP_IN       0x82        // EP 2 IN

// ====== USB Data Delay Timeout ======
#define TIMEOUT_MS  1000


/* ====================== Global ====================== */
static libusb_device_handle *g_dev = NULL;


/* ====================== Function ====================== */

/**
 * @brief Check CA310 Device connected.
 *        for User to check the device has been connected.
 *
 * @param None.
 * @return None.
 */
void check_device_status(void)
{
    libusb_device_handle *handle = NULL;

    printf("=== Checking USB Device Availability ===\n");

    handle = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
    if (!handle) {
        printf("[FAIL] Cannot find or open device (VID=0x%04X PID=0x%04X)\n",
               VENDOR_ID, PRODUCT_ID);
        return;
    }
    else{
        printf("[OK] Device found and opened successfully.\n");
    }
    
    int r = libusb_claim_interface(handle, IFACE_NUM);
    if (r != 0) {
        printf("[WARN] Device opened but cannot claim interface %d: %s\n",
               IFACE_NUM, libusb_error_name(r));

        libusb_close(handle);
        return;
    }

    printf("[OK] Interface %d claimed successfully.\n", IFACE_NUM);

    libusb_release_interface(handle, IFACE_NUM);
    libusb_close(handle);

    printf("[OK] Device test completed.\n");
}

/**
 * @brief Open USB port and connect the device
 *        
 *
 * @param None.
 * @return r : error reason.
 *         0 : success.
 */
int usb_open_device(void)
{
    int r;

    r = libusb_init(NULL);
    if (r < 0) {
        fprintf(stderr, "libusb_init error: %s\n", libusb_error_name(r));
        return r;
    }

    g_dev = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
    if (!g_dev) {
        fprintf(stderr, "Cannot open device (VID=0x%04X PID=0x%04X)\n",
                VENDOR_ID, PRODUCT_ID);
        libusb_exit(NULL);
        return -1;
    }

    if (libusb_kernel_driver_active(g_dev, IFACE_NUM) == 1) {
        printf("Kernel driver active, detaching...\n");
        r = libusb_detach_kernel_driver(g_dev, IFACE_NUM);
        if (r != 0) {
            fprintf(stderr, "Cannot detach kernel driver: %s\n",
                    libusb_error_name(r));
            libusb_close(g_dev);
            libusb_exit(NULL);
            g_dev = NULL;
            return r;
        }
    }

    r = libusb_claim_interface(g_dev, IFACE_NUM);
    if (r != 0) {
        fprintf(stderr, "Cannot claim interface: %s\n",
                libusb_error_name(r));
        libusb_close(g_dev);
        libusb_exit(NULL);
        g_dev = NULL;
        return r;
    }

    printf("USB device opened and interface %d claimed.\n", IFACE_NUM);
    return 0;
}

/**
 * @brief Closs USB port and disconnect the device safily.
 *        
 *
 * @param None.
 * @return None.
 */
void usb_close_device(void)
{
    if (g_dev) {
        libusb_release_interface(g_dev, IFACE_NUM);
        libusb_close(g_dev);
        g_dev = NULL;
    }
    libusb_exit(NULL);
    printf("USB closed.\n");
}

/**
 * @brief Send USB data via linux usb kernel driver.
 *        
 *
 * @param cmd : the ASCII word to send.
 * @return r : error reason.
 *         0 : success.
 */
int send_command_ascii(const char *cmd)
{
    if (!g_dev) {
        printf("[ERROR] Device not opened\n");
        return -1;
    }

    // send buffer：ASCII + CR(0x0D)
    uint8_t tx_buf[128];
    int tx_len = snprintf((char*)tx_buf, sizeof(tx_buf), "%s", cmd);
    tx_buf[tx_len++] = 0x0D;   // auto add CR in end.

    printf("[SEND] %s\\r  (HEX: ", cmd);
    for (int i = 0; i < tx_len; i++)
        printf("%02X ", tx_buf[i]);

    printf(")\n");

    int transferred;
    int r = libusb_bulk_transfer(
        g_dev,
        EP_OUT,
        tx_buf,
        tx_len,
        &transferred,
        TIMEOUT_MS
    );

    if (r != 0) {
        printf("[ERROR] USB OUT: %s\n", libusb_error_name(r));
        return r;
    }

    // ===============================
    //   wait until recived data
    // ===============================
    uint8_t rx_buf[256];
    int rx_len = 0;

    const int MAX_WAIT_MS = 5000;   // wait maximum timeout for 5 secs.
    int waited = 0;

    while (1)
    {
        r = libusb_bulk_transfer(
            g_dev,
            EP_IN,
            rx_buf,
            sizeof(rx_buf) - 1,
            &rx_len,
            TIMEOUT_MS   //timeout = 1000ms
        );

        if (r == 0 && rx_len > 0) break;

        if (r == LIBUSB_ERROR_TIMEOUT || rx_len == 0) {
            waited += TIMEOUT_MS;

            printf("[WAITING] no response yet... (%d ms)\n", waited);

            if (waited >= MAX_WAIT_MS) {
                printf("[ERROR] Timeout waiting for device response.\n");
                return -1;
            }

            continue;
        }

        // Error
        printf("[ERROR] USB IN: %s\n", libusb_error_name(r));
        return r;
    }

    rx_buf[rx_len] = 0;

    printf("[RECV ASCII] %s\n", rx_buf);

    printf("[RECV HEX] ");
    for (int i = 0; i < rx_len; i++)
        printf("%02X ", rx_buf[i]);
    printf("\n");

    return 0;
}

/**
 * @brief Send USB data via linux usb kernel driver and return the recived data.
 *        
 *
 * @param cmd : the ASCII word to send.
 *        out_resp : store recived data buffer.
 *        out_size : $out_resp buffer size.
 * @return r : error reason.
 *         0 : success.
 */
int send_command_ascii_with_resp(const char *cmd, char *out_resp, int out_size)
{
    if (!g_dev) {
        printf("[ERROR] Device not opened\n");
        return -1;
    }

    uint8_t tx_buf[128];
    int tx_len = snprintf((char*)tx_buf, sizeof(tx_buf), "%s", cmd);
    tx_buf[tx_len++] = 0x0D;   // CR

    printf("[SEND] %s\\r  (HEX: ", cmd);
    for (int i = 0; i < tx_len; i++)
        printf("%02X ", tx_buf[i]);
    printf(")\n");

    int transferred;
    int r = libusb_bulk_transfer(
        g_dev,
        EP_OUT,
        tx_buf,
        tx_len,
        &transferred,
        TIMEOUT_MS
    );

    if (r != 0) {
        printf("[ERROR] USB OUT: %s\n", libusb_error_name(r));
        return r;
    }

    uint8_t rx_buf[256];
    int rx_len = 0;
    const int MAX_WAIT_MS = 5000;
    int waited = 0;

    while (1) {
        r = libusb_bulk_transfer(
            g_dev,
            EP_IN,
            rx_buf,
            sizeof(rx_buf) - 1,
            &rx_len,
            TIMEOUT_MS
        );

        if (r == 0 && rx_len > 0) {
            break;
        }

        if (r == LIBUSB_ERROR_TIMEOUT || rx_len == 0) {
            waited += TIMEOUT_MS;
            printf("[WAITING] no response yet... (%d ms)\n", waited);
            if (waited >= MAX_WAIT_MS) {
                printf("[ERROR] Timeout waiting for device response.\n");
                return -1;
            }
            continue;
        }

        printf("[ERROR] USB IN: %s\n", libusb_error_name(r));
        return r;
    }

    rx_buf[rx_len] = 0;

    printf("[RECV ASCII] %s\n", rx_buf);
    printf("[RECV HEX] ");

    for (int i = 0; i < rx_len; i++)
        printf("%02X ", rx_buf[i]);
    printf("\n");

    // copy to buffer.
    if (out_resp && out_size > 0) {
        int copy_len = (rx_len < out_size - 1) ? rx_len : (out_size - 1);
        memcpy(out_resp, rx_buf, copy_len);
        out_resp[copy_len] = '\0';
    }

    return 0;
}

/**
 * @brief For initialization the CA310 channel.
 *        total 100 channel.
 *
 * @param ch : initialization channel number.
 * @return r : error reason.
 *         0 : success.
 */
int init_one_channel(int ch)
{
    int r;
    char cmd_MCH[64], cmd_TDR[64];

    printf("\n=== Init channel %d ===\n", ch);

    snprintf(cmd_MCH, sizeof(cmd_MCH), "MCH,%d", ch);
    r = send_command_ascii(cmd_MCH);
    if (r != 0) return r;

    r = send_command_ascii("STR,0");
    if (r != 0) return r;

    r = send_command_ascii("DPR,1");
    if (r != 0) return r;

    snprintf(cmd_TDR, sizeof(cmd_TDR), "TDR,%d", ch);
    r = send_command_ascii(cmd_TDR);
    if (r != 0) return r;

    r = send_command_ascii("STR,9");
    if (r != 0) return r;

    r = send_command_ascii("DPR,1");
    if (r != 0) return r;

    r = send_command_ascii("OPR,1");
    if (r != 0) return r;

    printf("=== Channel %d init done ===\n", ch);
    return 0;
}

/**
 * @brief For initialization the CA310 Device.
 * 
 *
 * @param None.
 * @return None.
 */
void zero_cal_all_channels(void)
{
    int r;

    printf("\n=== Zero Calibration ALL CHANNELS (0-cal) ===\n");

    r = send_command_ascii("ZRC");
    if (r != 0) {
        printf("[ERROR] ZRC failed, abort.\n");
        return;
    }

    r = send_command_ascii("IDO");
    if (r != 0) {
        printf("[ERROR] IDO failed, abort.\n");
        return;
    }

    r = send_command_ascii("RPR,1");
    if (r != 0) {
        printf("[ERROR] RPR,1 failed, abort.\n");
        return;
    }

    r = send_command_ascii("IDP,1");
    if (r != 0) {
        printf("[ERROR] IDP,1 failed, abort.\n");
        return;
    }

    r = send_command_ascii("STR,0");
    if (r != 0) return;

    r = send_command_ascii("STR,5");
    if (r != 0) return;

    r = send_command_ascii("STR,0");
    if (r != 0) return;

    r = send_command_ascii("MDS,6");
    if (r != 0) return;

    r = send_command_ascii("STR,5");
    if (r != 0) return;

    r = send_command_ascii("MDS,0");
    if (r != 0) return;

    r = send_command_ascii("MCH,0");
    if (r != 0) return;

    printf("\n=== start init 99 channels. ===\n");

    for (int ch = 1; ch <= 99; ++ch) {
        r = init_one_channel(ch);
        if (r != 0) {
            printf("[ERROR] Init channel %d failed, stop here.\n", ch);
            break;
        }
    }

    printf("\n=== Zero Calibration ALL CHANNELS DONE ===\n");
}