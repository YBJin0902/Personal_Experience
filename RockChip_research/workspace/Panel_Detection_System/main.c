/*
 * @Author: YNJin YNJin0902@icloud.com
 * @Date: 2025-12-18
 * @LastEditors: YNJin YNJin0902@icloud.com
 * @LastEditTime: 
 * @FilePath: 
 * @Description: 
 */

#include <stdio.h>
#include <string.h>

#include "User-USB.h"
#include "User-GUI.h"
//#include "User-NPU.h"

/* ====================== Struct ====================== */


/* ====================== Define ====================== */
//#define model_path "model/rknn_fp16_v1.rhnn"

/* ====================== Global ====================== */
extern double   g_last_X;
extern double   g_last_Y;
extern double   g_last_Lv;
extern double   g_last_x;
extern double   g_last_y;
extern gboolean g_has_measure;

extern char g_model_path[];

extern GtkWidget *g_btn_measure;
extern GtkWidget *g_btn_runloop;
extern GtkWidget *g_btn_npu_measure;

extern GtkWidget *g_drawing_area;

extern GtkWidget *g_label_val_x;
extern GtkWidget *g_label_val_y;
extern GtkWidget *g_label_val_lv;
extern GtkWidget *g_label_val_NPU;

/* ====================== Function ==================== */

void all_button_create(GtkBox* botton_box, GtkWidget *window)
{
    // Create buttons
    GtkWidget *btnCheck   = gtk_button_new_with_label("check device");
    GtkWidget *btnZeroCal = gtk_button_new_with_label("0-cal");
    GtkWidget *btnMeasure = gtk_button_new_with_label("Measure");
    GtkWidget *btnRunLoop = gtk_button_new_with_label("RunLoop");
    GtkWidget *btnSave    = gtk_button_new_with_label("Save as");

    GtkWidget *btnNPUModel    = gtk_button_new_with_label("Choose NPU Model");
    GtkWidget *btnNPUMeasure  = gtk_button_new_with_label("NPU & Measure");

    GtkWidget *btnPanelDetect = gtk_button_new_with_label("Panel detect");

    g_btn_measure = btnMeasure;
    gtk_widget_set_sensitive(btnMeasure, FALSE); // enbale after 0-cal.

    //g_btn_runloop = btnRunLoop;
    //gtk_widget_set_sensitive(btnRunLoop, FALSE); // enbale after 0-cal.

    g_btn_npu_measure = btnNPUMeasure;
    gtk_widget_set_sensitive(btnNPUMeasure, FALSE); // enbale after 0-cal.

    g_signal_connect(btnCheck,   "clicked",
                     G_CALLBACK(on_btn_check_clicked), window);
    g_signal_connect(btnZeroCal, "clicked",
                     G_CALLBACK(on_btn_zero_cal_clicked), window);
    g_signal_connect(btnMeasure, "clicked",
                     G_CALLBACK(on_btn_measure_clicked), window);
    g_signal_connect(btnRunLoop, "clicked",
                     G_CALLBACK(on_btn_runloop_clicked), window);
    g_signal_connect(btnSave,    "clicked",
                     G_CALLBACK(on_btn_save_clicked), window);
    g_signal_connect(btnNPUModel, "clicked",
                     G_CALLBACK(on_btn_npu_model_clicked), window);
    g_signal_connect(btnNPUMeasure, "clicked",
                     G_CALLBACK(on_btn_npu_measure_clicked), window);
    g_signal_connect(btnPanelDetect, "clicked",
                     G_CALLBACK(on_btn_panel_detect_clicked), window);

    gtk_box_pack_start(GTK_BOX(botton_box), btnCheck,   FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(botton_box), btnZeroCal, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(botton_box), btnMeasure, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(botton_box), btnRunLoop, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(botton_box), btnNPUModel, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(botton_box), btnNPUMeasure, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(botton_box), btnPanelDetect, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(botton_box), btnSave,    FALSE, FALSE, 5);
}

/**
 * @brief Main Program entry point.
 *
 *
 * @param None.
 * @param None.
 * @return .
 */
int main(int argc, char *argv[])
{
    int usb_ready = 0;

    // Need to open USB device first.
	if (usb_open_device() != 0) {
        fprintf(stderr, "[WARN] USB not available, run in GUI-only mode.\n");
        usb_ready = 0;
    } else {
        usb_ready = 1;
    }

    set_npu_log_dir("/home/linaro/Desktop/NPU_Log_files");

    // Initialize GTK UI.
    gtk_init(&argc, &argv);

    // Created window for the APP.
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "LCM Panel-Detected");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);
    //gtk_window_fullscreen(GTK_WINDOW(window)); // Fullscreen mode

    g_signal_connect(window, "destroy",
                     G_CALLBACK(on_window_destroy), NULL);

    // Left-right box layout
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
    gtk_container_add(GTK_CONTAINER(window), hbox);


    // Left side botton area.
    GtkWidget *vbox_left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_box_pack_start(GTK_BOX(hbox), vbox_left, FALSE, FALSE, 10);
    
    all_button_create(GTK_BOX(vbox_left), window);

    // Right side: top CIE plot, bottom large labels for x,y,Lv.
    GtkWidget *vbox_right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(hbox), vbox_right, TRUE, TRUE, 10);
    
    // Drawing Area for CIE graph (self-drawn, so directly set size).
    g_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(g_drawing_area, 700, 500);

    g_signal_connect(g_drawing_area, "draw", G_CALLBACK(on_draw_cie), NULL);
    gtk_box_pack_start(GTK_BOX(vbox_right), g_drawing_area, FALSE, FALSE, 5);

    // Below the drawing area, create a grid for value labels.
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_box_pack_start(GTK_BOX(vbox_right), grid, FALSE, FALSE, 10);

    GtkWidget *label_x = gtk_label_new("x =");
    GtkWidget *label_y = gtk_label_new("y =");
    GtkWidget *label_lv = gtk_label_new("Lv =");
    GtkWidget *label_NPU = gtk_label_new("NPU path =");

    g_label_val_x  = gtk_label_new("--");
    g_label_val_y  = gtk_label_new("--");
    g_label_val_lv = gtk_label_new("--");
    g_label_val_NPU = gtk_label_new("--");

    PangoFontDescription *desc = pango_font_description_from_string("Sans Bold 18");
    gtk_widget_override_font(label_x, desc);
    gtk_widget_override_font(label_y, desc);
    gtk_widget_override_font(label_lv, desc);
    gtk_widget_override_font(label_NPU, desc);
    gtk_widget_override_font(g_label_val_x, desc);
    gtk_widget_override_font(g_label_val_y, desc);
    gtk_widget_override_font(g_label_val_lv, desc);
    gtk_widget_override_font(g_label_val_NPU, desc);
    pango_font_description_free(desc);

    gtk_grid_attach(GTK_GRID(grid), label_x,       0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), g_label_val_x, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_y,       0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), g_label_val_y, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_lv,      0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), g_label_val_lv,1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_NPU,      0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), g_label_val_NPU,1, 3, 1, 1);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}