/*
 * @Author: YNJin YNJin0902@icloud.com
 * @Date: 2025-12-18
 * @LastEditors: YNJin YNJin0902@icloud.com
 * @LastEditTime: 
 * @FilePath: 
 * @Description: 
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "User-USB.h"

#include <gtk/gtk.h>

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>



// =============================================================
// Configuration
// =============================================================

/** Loop measurement count used by the "RunLoop" button. */
#define LOOP_TIME 500

/** Maximum number of measurements buffered in memory for CSV export. */
#define MAX_MEAS 5000

/** CIE plot axis limits used by the drawing area. */
#define X_MIN 0.0
#define X_MAX 0.8
#define Y_MIN 0.0
#define Y_MAX 0.9

// =============================================================
// Types / State
// =============================================================


// =============================================================
// Function Declarations
// =============================================================

int gui_run(int *argc, char ***argv);
gboolean on_draw_cie(GtkWidget *widget, cairo_t *cr, gpointer data);
void on_btn_check_clicked(GtkButton *button, gpointer user_data);
void on_window_destroy(GtkWidget *widget, gpointer data);
void on_btn_zero_cal_clicked(GtkButton *button, gpointer user_data);
void on_btn_measure_clicked(GtkButton *button, gpointer user_data);
void on_btn_runloop_clicked(GtkButton *button, gpointer user_data);
void on_btn_save_clicked(GtkButton *button, gpointer user_data);
void on_btn_check_clicked(GtkButton *button, gpointer user_data);
void on_btn_npu_model_clicked(GtkButton *button, gpointer user_data);
void on_btn_npu_measure_clicked(GtkButton *button, gpointer user_data);
void on_btn_panel_detect_clicked(GtkButton *button, gpointer user_data);
void set_npu_log_dir(const char* dir);

#ifdef __cplusplus
}
#endif
