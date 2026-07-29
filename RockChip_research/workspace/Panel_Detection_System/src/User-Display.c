/*
 * @Author: YNJin YNJin0902@icloud.com
 * @Date: 2026-03-23
 * @LastEditors: YNJin YNJin0902@icloud.com
 * @LastEditTime: 
 * @FilePath: 
 * @Description: 
 */

#include "User-Display.h"

volatile int g_running = 1;


/**
 * @brief Release resources associated with a DRM dumb buffer.
 *
 * This function unmaps the framebuffer memory, removes the framebuffer object,
 * and destroys the associated dumb buffer handle.
 *
 * @param fd  File descriptor of the DRM device.
 * @param buf Pointer to the framebuffer buffer structure to be cleaned up.
 *
 * @return None.
 */
void cleanup_buffer(int fd, drm_buffer_t *buf)
{
    if (!buf) return;

    if (buf->map && buf->size) {
        munmap(buf->map, buf->size);
        buf->map = NULL;
    }

    if (buf->fb_id) {
        drmModeRmFB(fd, buf->fb_id);
        buf->fb_id = 0;
    }

    if (buf->handle) {
        struct drm_mode_destroy_dumb destroy = {0};
        destroy.handle = buf->handle;
        ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
        buf->handle = 0;
    }
}

/**
 * @brief Release DRM display-related resources.
 *
 * Frees connector, encoder, and original CRTC objects
 * to prevent memory leaks.
 *
 * @param disp Pointer to the display structure to be cleaned up.
 *
 * @return None.
 */
void cleanup_display(drm_display_t *disp) 
{
    if (!disp) return;

    if (disp->orig_crtc) {
        drmModeFreeCrtc(disp->orig_crtc);
        disp->orig_crtc = NULL;
    }
    if (disp->enc) {
        drmModeFreeEncoder(disp->enc);
        disp->enc = NULL;
    }
    if (disp->conn) {
        drmModeFreeConnector(disp->conn);
        disp->conn = NULL;
    }
}

/**
 * @brief Convert DRM connector type to a human-readable string.
 *
 * Maps connector type constants to display interface names,
 * such as HDMI, DP, LVDS, or DSI.
 *
 * @param type DRM connector type.
 *
 * @return Corresponding connector type string; "Unknown" if not recognized.
 */
const char *connector_type_str(uint32_t type) 
{
    switch (type) 
    {
        case DRM_MODE_CONNECTOR_Unknown:     return "Unknown";
        case DRM_MODE_CONNECTOR_VGA:         return "VGA";
        case DRM_MODE_CONNECTOR_DVII:        return "DVI-I";
        case DRM_MODE_CONNECTOR_DVID:        return "DVI-D";
        case DRM_MODE_CONNECTOR_DVIA:        return "DVI-A";
        case DRM_MODE_CONNECTOR_Composite:   return "Composite";
        case DRM_MODE_CONNECTOR_SVIDEO:      return "SVIDEO";
        case DRM_MODE_CONNECTOR_LVDS:        return "LVDS";
        case DRM_MODE_CONNECTOR_Component:   return "Component";
        case DRM_MODE_CONNECTOR_9PinDIN:     return "DIN";
        case DRM_MODE_CONNECTOR_DisplayPort: return "DP";
        case DRM_MODE_CONNECTOR_HDMIA:       return "HDMI-A";
        case DRM_MODE_CONNECTOR_HDMIB:       return "HDMI-B";
        case DRM_MODE_CONNECTOR_TV:          return "TV";
        case DRM_MODE_CONNECTOR_eDP:         return "eDP";
        case DRM_MODE_CONNECTOR_VIRTUAL:     return "Virtual";
        case DRM_MODE_CONNECTOR_DSI:         return "DSI";
        case DRM_MODE_CONNECTOR_DPI:         return "DPI";
        case DRM_MODE_CONNECTOR_WRITEBACK:   return "Writeback";
        case DRM_MODE_CONNECTOR_SPI:         return "SPI";
        default:                             return "Unknown";
    }
}

/**
 * @brief Initialize display configuration from a connector.
 *
 * Extracts mode, encoder, and CRTC information from the given connector
 * and populates the drm_display_t structure for display setup.
 *
 * @param fd   File descriptor of the DRM device.
 * @param conn DRM connector object.
 * @param disp Pointer to the display structure to be initialized.
 *
 * @return 0 on success, -1 on failure.
 */
int setup_display_from_connector(int fd, drmModeConnector *conn, drm_display_t *disp) 
{
    memset(disp, 0, sizeof(*disp));

    disp->conn = conn;
    disp->conn_id = conn->connector_id;
    disp->mode = conn->modes[0];

    if (conn->encoder_id) {
        disp->enc = drmModeGetEncoder(fd, conn->encoder_id);
    }

    if (disp->enc) {
        disp->crtc_id = disp->enc->crtc_id;
    } else {
        for (int i = 0; i < conn->count_encoders; i++) {
            drmModeEncoder *enc = drmModeGetEncoder(fd, conn->encoders[i]);
            if (!enc)
                continue;

            disp->enc = enc;
            disp->crtc_id = enc->crtc_id;
            break;
        }
    }

    if (!disp->enc || !disp->crtc_id) {
        fprintf(stderr, "No usable encoder/crtc found\n");
        return -1;
    }

    disp->orig_crtc = drmModeGetCrtc(fd, disp->crtc_id);
    if (!disp->orig_crtc) {
        fprintf(stderr, "drmModeGetCrtc failed\n");
        return -1;
    }

    fprintf(stdout, "Connector ID : %u\n", disp->conn_id);
    fprintf(stdout, "CRTC ID      : %u\n", disp->crtc_id);
    fprintf(stdout, "Mode         : %s %ux%u\n",
            disp->mode.name, disp->mode.hdisplay, disp->mode.vdisplay);

    return 0;
}

/**
 * @brief Find and select an available display connector.
 *
 * Scans all DRM connectors and prioritizes "DSI-1".
 * If not found, falls back to the first connected connector with valid modes.
 *
 * @param fd   File descriptor of the DRM device.
 * @param disp Pointer to the display structure to store the result.
 *
 * @return 0 on success, -1 if no suitable connector is found.
 */
int find_display(int fd, drm_display_t *disp) 
{
    drmModeRes *res = NULL;
    drmModeConnector *fallback_conn = NULL;
    int ret = -1;

    res = drmModeGetResources(fd);
    if (!res) {
        fprintf(stderr, "drmModeGetResources failed\n");
        return -1;
    }

    for (int i = 0; i < res->count_connectors; i++) 
    {
        drmModeConnector *conn = drmModeGetConnector(fd, res->connectors[i]);
        char name[64];

        if (!conn)
            continue;

        snprintf(name, sizeof(name), "%s-%u",
                 connector_type_str(conn->connector_type),
                 conn->connector_type_id);

        fprintf(stdout, "Found connector: id=%u name=%s status=%s modes=%d\n",
                conn->connector_id,
                name,
                (conn->connection == DRM_MODE_CONNECTED) ? "connected" : "disconnected",
                conn->count_modes);

        if (conn->connection != DRM_MODE_CONNECTED || conn->count_modes == 0) 
        {
            drmModeFreeConnector(conn);
            continue;
        }

        if (strcmp(name, "DSI-1") == 0) 
        {
            fprintf(stdout, "Selected target connector: %s\n", name);
            ret = setup_display_from_connector(fd, conn, disp);
            if (ret < 0) {
                drmModeFreeConnector(conn);
            }
            goto out;
        }

        if (!fallback_conn) 
        {
            fallback_conn = conn;
        } 
        else 
        {
            drmModeFreeConnector(conn);
        }
    }

    if (fallback_conn) 
    {
        fprintf(stdout, "DSI-1 not found, fallback to first connected connector\n");
        ret = setup_display_from_connector(fd, fallback_conn, disp);

        if (ret < 0) 
        {
            drmModeFreeConnector(fallback_conn);
        }

        fallback_conn = NULL;
        goto out;
    }

    fprintf(stderr, "No connected connector found\n");

out:
    if (fallback_conn) 
    {
        drmModeFreeConnector(fallback_conn);
    }

    drmModeFreeResources(res);
    return ret;
}

/**
 * @brief Create a DRM dumb buffer and map it into user space.
 *
 * Allocates a dumb buffer with specified width, height, and 32bpp,
 * registers it as a framebuffer, and maps it for CPU access via mmap.
 *
 * @param fd     File descriptor of the DRM device.
 * @param width  Framebuffer width.
 * @param height Framebuffer height.
 * @param buf    Pointer to buffer structure to store the result.
 *
 * @return 0 on success, -1 on failure.
 */
int create_dumb_buffer(int fd, uint32_t width, uint32_t height, drm_buffer_t *buf) 
{
    struct drm_mode_create_dumb create = {0};
    struct drm_mode_map_dumb map = {0};
    int ret;

    memset(buf, 0, sizeof(*buf));

    create.width = width;
    create.height = height;
    create.bpp = 32;

    ret = ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);
    if (ret < 0) {
        perror("DRM_IOCTL_MODE_CREATE_DUMB");
        return -1;
    }

    buf->width  = width;
    buf->height = height;
    buf->pitch  = create.pitch;
    buf->size   = create.size;
    buf->handle = create.handle;

    ret = drmModeAddFB(fd, width, height, 24, 32, buf->pitch, buf->handle, &buf->fb_id);
    if (ret) {
        perror("drmModeAddFB");
        cleanup_buffer(fd, buf);
        return -1;
    }

    map.handle = buf->handle;
    ret = ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map);
    if (ret) {
        perror("DRM_IOCTL_MODE_MAP_DUMB");
        cleanup_buffer(fd, buf);
        return -1;
    }

    buf->map = mmap(0, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, map.offset);
    if (buf->map == MAP_FAILED) {
        perror("mmap");
        buf->map = NULL;
        cleanup_buffer(fd, buf);
        return -1;
    }

    memset(buf->map, 0, buf->size);
    return 0;
}

/**
 * @brief Parse a color string into a 24-bit RGB value.
 *
 * Supports common color names (red, green, blue, etc.)
 * and hexadecimal format "0xRRGGBB".
 *
 * @param name Color string.
 *
 * @return Parsed 24-bit RGB value. Defaults to white if unknown.
 */
uint32_t parse_color(const char *name) 
{
    if (!name) return 0x00FFFFFF;

    if (strcasecmp(name, "red") == 0) {
        return 0x00FF0000;
    } else if (strcasecmp(name, "green") == 0) {
        return 0x0000FF00;
    } else if (strcasecmp(name, "blue") == 0) {
        return 0x000000FF;
    } else if (strcasecmp(name, "white") == 0) {
        return 0x00FFFFFF;
    } else if (strcasecmp(name, "black") == 0) {
        return 0x00000000;
    } else if (strcasecmp(name, "yellow") == 0) {
        return 0x00FFFF00;
    } else if (strcasecmp(name, "cyan") == 0) {
        return 0x0000FFFF;
    } else if (strcasecmp(name, "magenta") == 0) {
        return 0x00FF00FF;
    }

    if (strncasecmp(name, "0x", 2) == 0) {
        return (uint32_t)strtoul(name, NULL, 16) & 0x00FFFFFF;
    }

    fprintf(stderr, "Unknown color '%s', use white\n", name);
    return 0x00FFFFFF;
}


/**
 * @brief Fill the framebuffer with a solid color.
 *
 * Writes the specified color value to every pixel
 * in the framebuffer.
 *
 * @param buf   Pointer to the framebuffer buffer.
 * @param color 32-bit color value.
 *
 * @return None.
 */
void fill_solid_color(drm_buffer_t *buf, uint32_t color) 
{
    for (uint32_t y = 0; y < buf->height; y++) {
        uint32_t *row = (uint32_t *)(buf->map + (size_t)y * buf->pitch);
        for (uint32_t x = 0; x < buf->width; x++) {
            row[x] = color;
        }
    }
}

