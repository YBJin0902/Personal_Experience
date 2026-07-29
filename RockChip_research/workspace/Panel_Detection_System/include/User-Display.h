/*
 * @Author: YNJin YNJin0902@icloud.com
 * @Date: 2026-03-23
 * @LastEditors: YNJin YNJin0902@icloud.com
 * @LastEditTime: 
 * @FilePath: 
 * @Description: For RK3588 LVDS display-port
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_mode.h>

/**
 * @brief : DRM display FrameBuffer.
 */
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t handle;
    uint64_t size;
    uint8_t *map;
    uint32_t fb_id;
} drm_buffer_t;

/**
 * @brief : For DRM display subsystem device.
 */
typedef struct {
    drmModeConnector *conn;
    drmModeEncoder *enc;
    drmModeCrtc *orig_crtc;
    drmModeModeInfo mode;
    uint32_t crtc_id;
    uint32_t conn_id;
} drm_display_t;


void cleanup_buffer(int fd, drm_buffer_t *buf);
void cleanup_display(drm_display_t *disp);
const char *connector_type_str(uint32_t type);
int setup_display_from_connector(int fd, 
								 drmModeConnector *conn, 
								 drm_display_t *disp);
int find_display(int fd, drm_display_t *disp);
int create_dumb_buffer(int fd, 
					   uint32_t width, 
					   uint32_t height, 
					   drm_buffer_t *buf);
uint32_t parse_color(const char *name);
void fill_solid_color(drm_buffer_t *buf, uint32_t color);