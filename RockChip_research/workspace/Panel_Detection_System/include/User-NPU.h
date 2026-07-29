/*
 * @Author: YNJin YNJin0902@icloud.com
 * @Date: 2025-12-18
 * @LastEditors: YNJin YNJin0902@icloud.com
 * @LastEditTime: 
 * @FilePath: 
 * @Description: 
 */

#ifndef USER_RKNN_H
#define USER_RKNN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>

#include "rknn_api.h"

// =============================================================
// Configuration
// =============================================================
#define RKNN_CLASS_NUM 10

// =============================================================
// Types / State
// =============================================================
typedef struct {
    const char* name;
    float prob;
    int idx;
} rknn_cls_prob_t;

/* RKNN handle */
typedef struct {
    int initialized;
    void* ctx;                 // rknn_context
    char model_path[1024];
} rknn_handle_t;


// =============================================================
// Function
// =============================================================

int rknn_open(rknn_handle_t* h, const char* path);
int rknn_infer_xy(rknn_handle_t* h, float X, float Y, float Lv, float x, float y,
                  float probs[RKNN_CLASS_NUM]);
void rknn_close(rknn_handle_t* h);
void rknn_sort_probs(const float probs[RKNN_CLASS_NUM],
                     rknn_cls_prob_t out[RKNN_CLASS_NUM]);
void rknn_format_result(double X, double Y, double Lv,
                        double x, double y,
                        const float probs[RKNN_CLASS_NUM],
                        char* buf, size_t buf_size);
int rknn_dump_usage_log(rknn_handle_t* h,
                        const char* log_path,
                        float x, float y,
                        const float probs[RKNN_CLASS_NUM]);

#ifdef __cplusplus
}
#endif

#endif /* USER_RKNN_H */
