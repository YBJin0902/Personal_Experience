/*
 * @Author: YNJin YNJin0902@icloud.com
 * @Date: 2025-12-18
 * @LastEditors: YNJin YNJin0902@icloud.com
 * @LastEditTime: 
 * @FilePath: 
 * @Description: 
 */



#include "User-GUI.h"
#include "User-NPU.h"
#include "User-Display.h"

/**
 * @brief For NPU model selection.
 */
typedef struct {
    GtkWindow *win;        // parent window
    GtkWidget *entry_path; // 用來顯示路徑的 GtkEntry (可選)
    char model_path[1024]; // 真正保存的路徑
} AppCtx;

/**
 * @brief One measurement record used for CSV export.
 */
typedef struct {
    int    n;
    double X, Y, Lv;
    double x, y;
} Measurement;

Measurement g_meas_list[MAX_MEAS];
int g_meas_count     = 0; /**< Number of buffered measurements. */
int g_measure_index  = 0; /**< Monotonic measurement index (n++ on success). */

// Last measurement values used by UI.
double   g_last_X  = 0.0;
double   g_last_Y  = 0.0;
double   g_last_Lv = 0.0;
double   g_last_x  = 0.0;
double   g_last_y  = 0.0;
gboolean g_has_measure = FALSE;

char g_model_path[512] = {0};

// =============================================================
// GTK Widgets (owned by this module)
// =============================================================
GtkWidget *g_btn_measure  = NULL;
GtkWidget *g_btn_runloop  = NULL;
GtkWidget *g_btn_npu_measure  = NULL;

GtkWidget *g_drawing_area = NULL;

GtkWidget *g_label_val_x  = NULL;
GtkWidget *g_label_val_y  = NULL;
GtkWidget *g_label_val_lv = NULL;
GtkWidget *g_label_val_NPU = NULL;

static rknn_handle_t g_rknn;
static char g_npu_log_dir[512] = "./";

// =============================================================
// Internal helpers
// =============================================================

/**
 * @brief Clamp a value into [min, max].
 *
 * @param v Input value.
 * @param vmin Lower bound.
 * @param vmax Upper bound.
 * @return Clamped value.
 */
double clamp_double(double v, double vmin, double vmax)
{
    if (v < vmin) return vmin;
    if (v > vmax) return vmax;
    return v;
}

/**
 * @brief Map (x,y) in CIE space to drawing-area pixel coordinates.
 *
 * The mapping assumes:
 * - origin is located at (x_axis, y_axis) (bottom-left of the plot)
 * - +x is to the right
 * - +y is upward
 *
 * @param x CIE chromaticity x.
 * @param y CIE chromaticity y.
 * @param x_axis Pixel X coordinate of the plot origin.
 * @param y_axis Pixel Y coordinate of the plot origin.
 * @param plot_w Plot width in pixels.
 * @param plot_h Plot height in pixels.
 * @param px Output pixel X.
 * @param py Output pixel Y.
 * @return None.
 */
void map_xy_to_pixel_plot(double x, double y,
                          double x_axis, double y_axis,
                          double plot_w, double plot_h,
                          double *px, double *py)
{
    x = clamp_double(x, X_MIN, X_MAX);
    y = clamp_double(y, Y_MIN, Y_MAX);

    const double xr = (x - X_MIN) / (X_MAX - X_MIN);
    const double yr = (y - Y_MIN) / (Y_MAX - Y_MIN);

    *px = x_axis + xr * plot_w;
    *py = y_axis - yr * plot_h; // bigger y draws upward
}

/**
 * @brief Write buffered measurements to CSV file.
 *
 * The CSV format is:
 *   n,X,Y,Lv,x,y
 *
 * @param filename Target path.
 * @return 0 on success, non-zero on error.
 */
int write_measurements_csv(const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    fprintf(fp, "n,X,Y,Lv,x,y\n");

    for (int i = 0; i < g_meas_count; ++i) {
        Measurement *m = &g_meas_list[i];
        fprintf(fp, "%d,%.6f,%.6f,%.6f,%.5f,%.5f\n",
                m->n, m->X, m->Y, m->Lv, m->x, m->y);
    }

    fclose(fp);

    // Make file accessible to everyone (as requested previously).
    if (chmod(filename, 0777) != 0) {
        perror("chmod");
        // Not fatal for the CSV content; still report success for writing.
    }

    return 0;
}

/**
 * @brief Parse "XYZ" response and compute CIE 1931 x,y.
 *
 * Expected response example:
 *   OK01,P1 85.531127;156.35797;20.493055
 *
 * @param resp Response buffer (NUL-terminated string).
 * @param out_X Output X.
 * @param out_Y Output Y.
 * @param out_Lv Output Lv (treated as Z in xy calculation).
 * @param out_x Output chromaticity x.
 * @param out_y Output chromaticity y.
 * @return 0 on success, non-zero on parse error.
 */
int parse_xyz_and_xy(const char *resp,
                     double *out_X, double *out_Y, double *out_Lv,
                     double *out_x, double *out_y)
{
    const char *p = strchr(resp, ' ');
    if (!p) {
        fprintf(stderr, "[WARN] Unexpected response format: %s\n", resp);
        return -1;
    }

    double X, Y, Lv;
    const int n = sscanf(p + 1, "%lf;%lf;%lf", &X, &Y, &Lv);
    if (n != 3) {
        fprintf(stderr, "[WARN] Cannot parse XYZ from: %s\n", p + 1);
        return -2;
    }

    const double sum = X + Y + Lv;
    if (sum == 0.0) {
        fprintf(stderr, "[WARN] X+Y+Lv == 0\n");
        return -3;
    }

    *out_X = X;
    *out_Y = Y;
    *out_Lv = Lv;
    *out_x = X / sum;
    *out_y = Y / sum;
    return 0;
}

/**
 * @brief Update big labels under the plot using the latest measurement.
 *
 * @param x CIE chromaticity x.
 * @param y CIE chromaticity y.
 * @param Lv Luminance value.
 * @return None.
 */
void update_value_labels(double x, double y, double Lv)
{
    char buf[64];

    if (g_label_val_x) {
        snprintf(buf, sizeof(buf), "%.5f", x);
        gtk_label_set_text(GTK_LABEL(g_label_val_x), buf);
    }
    if (g_label_val_y) {
        snprintf(buf, sizeof(buf), "%.5f", y);
        gtk_label_set_text(GTK_LABEL(g_label_val_y), buf);
    }
    if (g_label_val_lv) {
        snprintf(buf, sizeof(buf), "%.5f", Lv);
        gtk_label_set_text(GTK_LABEL(g_label_val_lv), buf);
    }
}

/**
 * @brief Update big labels under the plot using the NPU model botton.
 *
 * @param NPU model path.
 * @return None.
 */
void update_NPU_path_value_labels(char *path)
{
    gtk_label_set_text(GTK_LABEL(g_label_val_NPU), path);
}

/**
 * @brief Record a measurement to the in-memory buffer.
 *
 * @param X Tristimulus X.
 * @param Y Tristimulus Y.
 * @param Lv Luminance / Z.
 * @param x Chromaticity x.
 * @param y Chromaticity y.
 * @return 0 if recorded, non-zero if buffer is full.
 */
int record_measurement(double X, double Y, double Lv, double x, double y)
{
    if (g_meas_count >= MAX_MEAS) {
        fprintf(stderr, "[WARN] Measurement buffer full, not recording more.\n");
        return -1;
    }

    g_meas_list[g_meas_count].n  = g_measure_index;
    g_meas_list[g_meas_count].X  = X;
    g_meas_list[g_meas_count].Y  = Y;
    g_meas_list[g_meas_count].Lv = Lv;
    g_meas_list[g_meas_count].x  = x;
    g_meas_list[g_meas_count].y  = y;
    g_meas_count++;

    return 0;
}

// =============================================================
// GTK callbacks
// =============================================================

/**
 * @brief Draw callback for the CIE plot drawing area.
 *
 * The drawing sequence is:
 * 1) White background
 * 2) Fake CIE-like colored background (grid fill)
 * 3) Axes and tick labels
 * 4) Red dot for the latest measurement (if available)
 *
 * @param widget Drawing area widget.
 * @param cr Cairo context.
 * @param data User data (unused).
 * @return FALSE to indicate no further processing is required.
 */
gboolean on_draw_cie(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    (void)data;

    const int width  = gtk_widget_get_allocated_width(widget);
    const int height = gtk_widget_get_allocated_height(widget);

    // Background: white
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    // Plot margins
    const double margin_left   = 60.0;
    const double margin_right  = 20.0;
    const double margin_top    = 20.0;
    const double margin_bottom = 20.0;

    const double plot_w = width  - margin_left - margin_right;
    const double plot_h = height - margin_top  - margin_bottom;
    if (plot_w <= 0 || plot_h <= 0) {
        return FALSE;
    }

    // Plot origin (x=0,y=0)
    const double x_axis = margin_left;
    const double y_axis = height - margin_bottom;

    // --------------- Fake CIE background ---------------
    // Grid size (smaller -> smoother but heavier CPU)
    const int step = 4;

    for (int iy = 0; iy < (int)plot_h; iy += step) {
        for (int ix = 0; ix < (int)plot_w; ix += step) {
            const double xr = (double)ix / plot_w;
            const double yr = (double)iy / plot_h;

            const double x = X_MIN + xr * (X_MAX - X_MIN);
            const double y = Y_MIN + (1.0 - yr) * (Y_MAX - Y_MIN);

            // Simple pseudo coloring: x controls red, y controls green, blue inversely.
            double R = x / X_MAX;
            double G = y / Y_MAX;
            double B = 1.0 - R;

            // Reduce saturation (mix with white)
            const double sat = 0.6;
            R = sat * R + (1.0 - sat);
            G = sat * G + (1.0 - sat);
            B = sat * B + (1.0 - sat);

            R = clamp_double(R, 0.0, 1.0);
            G = clamp_double(G, 0.0, 1.0);
            B = clamp_double(B, 0.0, 1.0);

            cairo_set_source_rgb(cr, R, G, B);
            cairo_rectangle(cr, x_axis + ix, y_axis - iy - step, step, step);
            cairo_fill(cr);
        }
    }

    // --------------- Axes and ticks ---------------
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 1.0);

    // X axis
    cairo_move_to(cr, x_axis, y_axis);
    cairo_line_to(cr, x_axis + plot_w, y_axis);
    // Y axis
    cairo_move_to(cr, x_axis, y_axis);
    cairo_line_to(cr, x_axis, y_axis - plot_h);
    cairo_stroke(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 10.0);
    cairo_set_line_width(cr, 0.5);

    const double xticks[] = {0.0, 0.2, 0.4, 0.6, 0.8};
    const double yticks[] = {0.0, 0.3, 0.6, 0.9};

    // X ticks
    for (int i = 0; i < (int)(sizeof(xticks) / sizeof(xticks[0])); ++i) {
        const double xr = (xticks[i] - X_MIN) / (X_MAX - X_MIN);
        const double vx = x_axis + xr * plot_w;

        cairo_move_to(cr, vx, y_axis);
        cairo_line_to(cr, vx, y_axis + 5);
        cairo_stroke(cr);

        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f", xticks[i]);
        cairo_move_to(cr, vx - 8, y_axis + 18);
        cairo_show_text(cr, buf);
    }

    // Y ticks
    for (int i = 0; i < (int)(sizeof(yticks) / sizeof(yticks[0])); ++i) {
        const double yr = (yticks[i] - Y_MIN) / (Y_MAX - Y_MIN);
        const double vy = y_axis - yr * plot_h;

        cairo_move_to(cr, x_axis, vy);
        cairo_line_to(cr, x_axis - 5, vy);
        cairo_stroke(cr);

        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f", yticks[i]);
        cairo_move_to(cr, x_axis - 30, vy + 4);
        cairo_show_text(cr, buf);
    }

    // --------------- Measurement dot ---------------
    if (g_has_measure) {
        double px, py;
        map_xy_to_pixel_plot(g_last_x, g_last_y, x_axis, y_axis, plot_w, plot_h, &px, &py);

        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        cairo_arc(cr, px, py, 4.0, 0, 2 * 3.1415926);
        cairo_fill(cr);
    }

    return FALSE;
}

/**
 * @brief "Check device" button click callback.
 *
 * @param button Button widget.
 * @param user_data User data (unused).
 * @return None.
 */
void on_btn_check_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;
    check_device_status();
}

/**
 * @brief Window destroy callback.
 *
 * Closes USB device and exits the GTK main loop.
 *
 * @param widget Window widget.
 * @param data User data (unused).
 * @return None.
 */
void on_window_destroy(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;
    usb_close_device();
    gtk_main_quit();
}

/**
 * @brief "0-cal" button click callback.
 *
 * Performs zero calibration on all channels and then enables the "Measure" button.
 *
 * @param button Button widget.
 * @param user_data User data (unused).
 * @return None.
 */
void on_btn_zero_cal_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;

    printf("\n=== 0-cal Button Clicked ===\n");
    zero_cal_all_channels();

    if (g_btn_measure) {
        gtk_widget_set_sensitive(g_btn_measure, TRUE);
    }

    if (g_btn_runloop) {
        gtk_widget_set_sensitive(g_btn_runloop, TRUE);
    }
}

/**
 * @brief "Measure" button click callback.
 *
 * Sends "XYZ" to device, parses XYZ, computes x,y, updates UI labels and plot,
 * and stores the measurement to the internal buffer.
 *
 * @param button Button widget.
 * @param user_data User data (unused).
 * @return None.
 */
void on_btn_measure_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;

    printf("\n=== Measure ===\n");

    char resp[256];
    if (send_command_ascii_with_resp("XYZ", resp, sizeof(resp)) != 0) {
        fprintf(stderr, "[ERROR] Measure failed.\n");
        return;
    }

    double X, Y, Lv, x, y;
    if (parse_xyz_and_xy(resp, &X, &Y, &Lv, &x, &y) != 0) {
        return;
    }

    g_last_X = X;
    g_last_Y = Y;
    g_last_Lv = Lv;
    g_last_x = x;
    g_last_y = y;
    g_has_measure = TRUE;

    //g_measure_index++; // measurement index starts from 1
    //record_measurement(X, Y, Lv, x, y);

    printf("Measure #%d\n", g_measure_index);
    printf("X = %.6f, Y = %.6f, Lv = %.6f\n", X, Y, Lv);
    printf("x = %.5f, y = %.5f\n", x, y);

    update_value_labels(x, y, Lv);

    if (g_drawing_area) {
        gtk_widget_queue_draw(g_drawing_area);
    }
}

/**
 * @brief "RunLoop" button click callback.
 *
 * Performs LOOP_TIME consecutive "XYZ" measurements and buffers them, then opens
 * a save dialog to export the collected samples as CSV.
 *
 * @param button Button widget.
 * @param user_data Pointer to parent GtkWindow.
 * @return None.
 */
void on_btn_runloop_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    GtkWindow *parent = GTK_WINDOW(user_data);

    printf("\n=== Run Loop (%d measurements) ===\n", LOOP_TIME);

    char resp[256];

    for (int i = 0; i < LOOP_TIME; i++) {
        if (send_command_ascii_with_resp("XYZ", resp, sizeof(resp)) != 0) {
            fprintf(stderr, "[ERROR] Measure failed at index %d.\n", i + 1);
            return;
        }

        double X, Y, Lv, x, y;
        if (parse_xyz_and_xy(resp, &X, &Y, &Lv, &x, &y) != 0) {
            return;
        }

        g_has_measure = TRUE;
        g_measure_index++;

        if (record_measurement(X, Y, Lv, x, y) != 0) {
            fprintf(stderr, "[WARN] Buffer full. Stop.\n");
            break;
        }

        if ((i % 50) == 0) {
            printf("  progress: %d / %d\n", i, LOOP_TIME);
        }
    }

    // Auto open save dialog after loop
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Save CSV (500 loops done)",
        parent,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save",   GTK_RESPONSE_ACCEPT,
        NULL);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "runloop_500.csv");

    const gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        printf("Saving 500-run CSV to: %s\n", filename);

        (void)write_measurements_csv(filename);
        g_free(filename);
    }
    
    memset(g_meas_list, 0, sizeof(g_meas_list));
    g_meas_count = 0;

    gtk_widget_destroy(dialog);
}

/**
 * @brief "Save as" button click callback.
 *
 * Opens a save dialog and exports all buffered measurements to CSV.
 *
 * @param button Button widget.
 * @param user_data Pointer to parent GtkWindow.
 * @return None.
 */
void on_btn_save_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    GtkWindow *parent = GTK_WINDOW(user_data);

    if (g_meas_count == 0) {
        fprintf(stderr, "[WARN] No measurement yet, nothing to save.\n");
        return;
    }

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Save CSV",
        parent,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save",   GTK_RESPONSE_ACCEPT,
        NULL);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "measure.csv");

    const gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        printf("Saving to: %s\n", filename);

        (void)write_measurements_csv(filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

/**
 * @brief "Check NPU Model" button click callback.
 *
 * @param button Button widget.
 * @param user_data Pointer to parent GtkWindow.
 * @return None.
 */
void on_btn_npu_model_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Select NPU Model",
        NULL,                        
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open",   GTK_RESPONSE_ACCEPT,
        NULL);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Model Files (*.rknn, *.onnx, *.bin)");
    gtk_file_filter_add_pattern(filter, "*.rknn");
    gtk_file_filter_add_pattern(filter, "*.onnx");
    gtk_file_filter_add_pattern(filter, "*.bin");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        if (path)
        {
            snprintf(g_model_path, 1024, "%s", path);
            g_print("Selected model: %s\n", g_model_path);
            update_NPU_path_value_labels(g_model_path);
            g_free(path);
        }
    }

    if (g_btn_npu_measure) {
        gtk_widget_set_sensitive(g_btn_npu_measure, TRUE);
    }

    gtk_widget_destroy(dialog);
}

void show_dialog_common(GtkWindow *parent,
                        GtkMessageType type,
                        const char *title,
                        const char *fmt,
                        va_list ap)
{
    char msg[4096];
    vsnprintf(msg, sizeof(msg), fmt, ap);

    GtkWidget *dialog = gtk_message_dialog_new(
        NULL,                
        GTK_DIALOG_MODAL,
        type,
        GTK_BUTTONS_OK,
        "%s", title);

    gtk_message_dialog_format_secondary_text(
        GTK_MESSAGE_DIALOG(dialog), "%s", msg);

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/**
 * @brief Show error dialog (modal, OK button).
 * @param parent Parent GtkWindow (can be NULL).
 * @param title  Dialog title.
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
void show_error_dialog(GtkWindow *parent,
                       const char *title,
                       const char *fmt, ...)
{
    (void)parent;

    va_list ap;
    va_start(ap, fmt);
    show_dialog_common(parent, GTK_MESSAGE_ERROR, title, fmt, ap);
    va_end(ap);
}


/**
 * @brief Show info dialog (modal, OK button).
 */
void show_info_dialog(GtkWindow *parent,
                      const char *title,
                      const char *fmt, ...)
{
    (void)parent;

    va_list ap;
    va_start(ap, fmt);
    show_dialog_common(parent, GTK_MESSAGE_INFO, title, fmt, ap);
    va_end(ap);
}

void set_npu_log_dir(const char* dir)
{
    if (!dir || !dir[0]) return;

    snprintf(g_npu_log_dir, sizeof(g_npu_log_dir), "%s", dir);

    struct stat st = {0};

    if (stat(g_npu_log_dir, &st) == -1) 
    {
        mkdir(g_npu_log_dir, 0777);
    }
}

void get_model_name_no_ext(const char* path, char* out, size_t out_sz)
{
    const char* base = strrchr(path, '/');
    base = (base) ? (base + 1) : path;

    snprintf(out, out_sz, "%s", base);

    char* dot = strrchr(out, '.');
    if (dot) {
        *dot = '\0';
    }
}

void build_npu_log_filename(const char* model_path, char* out, size_t out_sz)
{
    static unsigned int seq = 0;

    char model_name[256] = {0};
    char time_buf[64] = {0};

    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);

    if (!model_path || !model_path[0]) {
        snprintf(model_name, sizeof(model_name), "unknown");
    } else {
        get_model_name_no_ext(model_path, model_name, sizeof(model_name));
    }

    if (tm_now) {
        strftime(time_buf, sizeof(time_buf), "%Y%m%d_%H%M%S", tm_now);
    } else {
        snprintf(time_buf, sizeof(time_buf), "unknown_time");
    }

    seq++;

    snprintf(out, out_sz,
        "%s/npu_usage_%s_%s_%03u.log",
        g_npu_log_dir,
        model_name,
        time_buf,
        seq);
}

/**
 * @brief "NPU infer and Measure" button click callback.
 *
 * @param button Button widget.
 * @param user_data Pointer to parent GtkWindow.
 * @return None.
 */
void on_btn_npu_measure_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;

    double X, Y, Lv, x, y;
    char resp[256];

    if (send_command_ascii_with_resp("XYZ", resp, sizeof(resp)) != 0) {
        show_error_dialog(NULL, "CA310", "Measure failed");
        return;
    }

    if (parse_xyz_and_xy(resp, &X, &Y, &Lv, &x, &y) != 0) {
        show_error_dialog(NULL, "CA310", "Parse XYZ failed");
        return;
    }

    if (rknn_open(&g_rknn, g_model_path) != 0) {
        show_error_dialog(NULL, "RKNN", "Open model failed");
        return;
    }

    float probs[RKNN_CLASS_NUM];
    if (rknn_infer_xy(&g_rknn, (float)X, (float)Y, (float)Lv, (float)x, (float)y, probs) != 0) {
        show_error_dialog(NULL, "RKNN", "Inference failed");
        return;
    }

    char log_path[512] = {0};
    build_npu_log_filename(g_model_path, log_path, sizeof(log_path));

    if (rknn_dump_usage_log(&g_rknn, log_path, (float)x, (float)y, probs) != 0) {
        fprintf(stderr, "[WARN] failed to write NPU usage log: %s\n", log_path);
    } else {
        printf("[INFO] NPU usage log appended to: %s\n", log_path);
    }

    char text[4096];
    char text2[4608];

    rknn_format_result(X, Y, Lv, x, y, probs, text, sizeof(text));
    snprintf(text2, sizeof(text2), "%s\nLog file:\n%s", text, log_path);

    show_info_dialog(NULL, "NPU Result", "%s", text2);
}

/**
 * @brief Start detect the panel and use NPU to infer.
 *
 * @param button Button widget.
 * @param user_data Pointer to parent GtkWindow.
 * @return None.
 */
void on_btn_panel_detect_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;

    double X, Y, Lv, x, y;
    char resp[256];

    char log_path[512] = {0};
    char text[4096];
    char text2[4608];

    int fd = -1;
    int ret = -1;

    char *display_device = "/dev/dri/card0";

    drm_display_t disp;
    drm_buffer_t buf;

    size_t color_count = 8;
    const uint32_t colors[] = {
        0x00FF0000, // red
        0x0000FF00, // green
        0x000000FF, // blue
        0x00FFFFFF, // white
        0x00000000, // black
        0x00FFFF00, // yellow
        0x0000FFFF, // cyan
        0x00FF00FF  // magenta
    };

    memset(&disp, 0, sizeof(disp));
    memset(&buf, 0, sizeof(buf));

    fd = open(display_device, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        perror("open drm device");
        return EXIT_FAILURE;
    }

    if (drmSetMaster(fd) && errno != EACCES) {
        perror("drmSetMaster");
    }

    if (find_display(fd, &disp) < 0) {
        perror("find display");
    }

    if (create_dumb_buffer(fd, disp.mode.hdisplay, disp.mode.vdisplay, &buf) < 0) {
        perror("create dumb buffer");
    }

    ret = drmModeSetCrtc(fd,
                         disp.crtc_id,
                         buf.fb_id,
                         0, 0,
                         &disp.conn_id, 1,
                         &disp.mode);
    if (ret) {
        perror("drmModeSetCrtc");
    }

    printf("Start looping colors on %s\n", display_device);

    for(int i = 0; i < color_count; i++)
    {
        printf("Num %d.\r\n Showing Color: 0x%06X\r\n", i + 1, colors[i] & 0xFFFFFF);
        fill_solid_color(&buf, colors[i]);

        sleep(1);

        if (send_command_ascii_with_resp("XYZ", resp, sizeof(resp)) != 0) {
            show_error_dialog(NULL, "CA310", "Measure failed");
            return;
        }

        if (parse_xyz_and_xy(resp, &X, &Y, &Lv, &x, &y) != 0) {
            show_error_dialog(NULL, "CA310", "Parse XYZ failed");
            return;
        }

        float probs[RKNN_CLASS_NUM];
        if (rknn_infer_xy(&g_rknn, (float)X, (float)Y, (float)Lv, (float)x, (float)y, probs) != 0) {
            show_error_dialog(NULL, "RKNN", "Open model failed");
            return;
        }

        build_npu_log_filename(g_model_path, log_path, sizeof(log_path));

        if (rknn_dump_usage_log(&g_rknn, log_path, (float)x, (float)y, probs) != 0) {
            fprintf(stderr, "[WARN] failed to write NPU usage log: %s\n", log_path);
        } else {
            printf("[INFO] NPU usage log appended to: %s\n", log_path);
        }


        rknn_format_result(X, Y, Lv, x, y, probs, text, sizeof(text));
        snprintf(text2, sizeof(text2), "%s\nLog file:\n%s", text, log_path);

        show_info_dialog(NULL, "NPU Result", "%s", text2);

        sleep(3); // delay s.

        memset(log_path, 0, sizeof(log_path));
        memset(text, 0, sizeof(text));
        memset(text2, 0, sizeof(text2));

        //Output panel classification level.

    }

    printf("Detecting done.");

    ret = 0;

    if (disp.orig_crtc) {
        drmModeSetCrtc(fd,
                       disp.orig_crtc->crtc_id,
                       disp.orig_crtc->buffer_id,
                       disp.orig_crtc->x,
                       disp.orig_crtc->y,
                       &disp.conn_id, 1,
                       &disp.orig_crtc->mode);
    }

    cleanup_buffer(fd, &buf);
    cleanup_display(&disp);

    if (fd >= 0) {
        close(fd);
    }

    return;
}