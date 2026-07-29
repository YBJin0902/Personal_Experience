/*
 * @Author: YNJin YNJin0902@icloud.com
 * @Date: 2025-12-18
 * @LastEditors: YNJin YNJin0902@icloud.com
 * @LastEditTime: 
 * @FilePath: 
 * @Description: 
 */

#include "User-NPU.h"

/* ===================== labels ===================== */

char perf_NPU[128];

static const char* LABELS_10[RKNN_CLASS_NUM] = {
    "black","blue","cyan","green","orange","pink","purple","red","white","yellow"
};

/* ===================== utils ===================== */

static int cmp_desc_prob(const void* a, const void* b)
{
    const rknn_cls_prob_t* pa = (const rknn_cls_prob_t*)a;
    const rknn_cls_prob_t* pb = (const rknn_cls_prob_t*)b;
    if (pa->prob < pb->prob) return 1;
    if (pa->prob > pb->prob) return -1;
    return 0;
}

static unsigned char* load_file(const char* filename, int* out_size)
{
    FILE* fp = fopen(filename, "rb");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz <= 0) { fclose(fp); return NULL; }

    unsigned char* buf = (unsigned char*)malloc(sz);
    if (!buf) { fclose(fp); return NULL; }

    if (fread(buf, 1, sz, fp) != (size_t)sz) {
        fclose(fp);
        free(buf);
        return NULL;
    }

    fclose(fp);
    *out_size = (int)sz;
    return buf;
}

/**
 * @brief Initialize RKNN model (load + rknn_init)
 * @param h        handle
 * @param path     rknn model path
 * @return 0 on success
 */
int rknn_open(rknn_handle_t* h, const char* path)
{
    if (!h || !path || !path[0]) return -1;

    /* already opened with same model */
    if (h->initialized && strcmp(h->model_path, path) == 0)
        return 0;

    rknn_close(h);

    int model_size = 0;
    unsigned char* model_data = load_file(path, &model_size);
    if (!model_data) return -2;

    rknn_context ctx = 0;
    int ret = rknn_init(&ctx, model_data, model_size, RKNN_FLAG_COLLECT_PERF_MASK, NULL);
    free(model_data);
    if (ret != RKNN_SUCC) return -3;

    rknn_core_mask core_mask = RKNN_NPU_CORE_AUTO;
    ret = rknn_set_core_mask(ctx, core_mask);
    if (ret != RKNN_SUCC) return -3;

    memset(h, 0, sizeof(*h));
    h->ctx = (void*)ctx;
    h->initialized = 1;
    strncpy(h->model_path, path, sizeof(h->model_path) - 1);

    return 0;
}

/**
 * @brief Run inference with (x,y)
 * @param h        handle (must be opened)
 * @param X        input X
 * @param Y        input Y
 * @param Lv       input Lv
 * @param x        input x
 * @param y        input y
 * @param probs    output probabilities [RKNN_CLASS_NUM]
 * @return 0 on success
 */
int rknn_infer_xy(rknn_handle_t* h, float X, float Y, float Lv, float x, float y,
                  float probs[RKNN_CLASS_NUM])
{
    if (!h || !h->initialized || !probs) return -1;

    rknn_context ctx = (rknn_context)h->ctx;

    float in_xy[5] = { X, Y, Lv, x, y };

    rknn_input input;
    memset(&input, 0, sizeof(input));
    input.index = 0;
    input.type  = RKNN_TENSOR_FLOAT32;
    input.size  = sizeof(in_xy);
    input.fmt   = RKNN_TENSOR_NHWC;
    input.buf   = in_xy;

    if (rknn_inputs_set(ctx, 1, &input) != RKNN_SUCC) return -2;

    if (rknn_run(ctx, NULL) != RKNN_SUCC) return -3;

    rknn_perf_detail perf_detail;
    memset(&perf_detail, 0, sizeof(perf_detail));
    int qret = rknn_query(ctx, RKNN_QUERY_PERF_DETAIL, &perf_detail, sizeof(perf_detail));

    printf("Performance details :\n %s\n", perf_detail.perf_data);

    rknn_output output;
    memset(&output, 0, sizeof(output));
    output.want_float = 1;

    if (rknn_outputs_get(ctx, 1, &output, NULL) != RKNN_SUCC)
        return -4;

    const float* p = (const float*)output.buf;
    int n = (int)(output.size / sizeof(float));

    for (int i = 0; i < RKNN_CLASS_NUM; i++)
        probs[i] = (i < n) ? p[i] : 0.0f;

    rknn_outputs_release(ctx, 1, &output);
    return 0;
}

/**
 * @brief Release RKNN resources
 */
void rknn_close(rknn_handle_t* h)
{
    if (!h || !h->initialized) return;

    rknn_context ctx = (rknn_context)h->ctx;
    rknn_destroy(ctx);

    memset(h, 0, sizeof(*h));
}


/**
 * @brief Sort probabilities (desc)
 */
void rknn_sort_probs(const float probs[RKNN_CLASS_NUM],
                     rknn_cls_prob_t out[RKNN_CLASS_NUM])
{
    for (int i = 0; i < RKNN_CLASS_NUM; i++) {
        out[i].name = LABELS_10[i];
        out[i].prob = probs[i];
        out[i].idx  = i;
    }

    qsort(out, RKNN_CLASS_NUM, sizeof(out[0]), cmp_desc_prob);
}


/**
 * @brief Format result to text (for GTK dialog)
 */
void rknn_format_result(double X, double Y, double Lv,
                        double x, double y,
                        const float probs[RKNN_CLASS_NUM],
                        char* buf, size_t buf_size)
{
    rknn_cls_prob_t items[RKNN_CLASS_NUM];
    rknn_sort_probs(probs, items);

    int off = 0;
    off += snprintf(buf + off, buf_size - off,
                    "Input: \nX=%.6f  Y=%.6f  Z=%.6f  x=%.6f  y=%.6f\n"
                    "---------------------------------\n\n",
                    X, Y, Lv, x, y);

    for (int i = 0; i < RKNN_CLASS_NUM; i++) 
    {
        off += snprintf(buf + off, buf_size - off,
                        "%2d)  %-7s : %7.3f%%\n",
                        i + 1,
                        items[i].name,
                        items[i].prob * 100.0f);
    }

    snprintf(buf + off, buf_size - off,
             "---------------------------------\n\n"
             "Most likely : %s (%.3f%%)\n",
             items[0].name,
             items[0].prob * 100.0f);
}

char* get_model_basename_no_ext(const char* path, char* out, size_t out_sz)
{
    const char* base = strrchr(path, '/');
    base = (base) ? (base + 1) : path;

    snprintf(out, out_sz, "%s", base);

    char* dot = strrchr(out, '.');
    if (dot) {
        *dot = '\0';
    }
    return out;
}

int rknn_dump_usage_log(rknn_handle_t* h,
                        const char* log_path,
                        float x, float y,
                        const float probs[RKNN_CLASS_NUM])
{
    if (!h || !h->initialized || !log_path || !probs) {
        return -1;
    }

    FILE* fp = fopen(log_path, "a");
    if (!fp) {
        perror("fopen log_path");
        return -2;
    }

    rknn_context ctx = (rknn_context)h->ctx;

    rknn_perf_run perf_run;
    memset(&perf_run, 0, sizeof(perf_run));
    int ret_run = rknn_query(ctx, RKNN_QUERY_PERF_RUN, &perf_run, sizeof(perf_run));

    rknn_perf_detail perf_detail;
    memset(&perf_detail, 0, sizeof(perf_detail));
    int ret_detail = rknn_query(ctx, RKNN_QUERY_PERF_DETAIL, &perf_detail, sizeof(perf_detail));

    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);

    char time_buf[64] = {0};
    if (tm_now) {
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_now);
    } else {
        snprintf(time_buf, sizeof(time_buf), "unknown-time");
    }

    fprintf(fp, "==================================================\n");
    fprintf(fp, "time        : %s\n", time_buf);
    fprintf(fp, "model       : %s\n", h->model_path);
    fprintf(fp, "input(x,y)  : %.6f, %.6f\n", x, y);

    if (ret_run == RKNN_SUCC) {
        fprintf(fp, "run_duration_us : %lld\n", (long long)perf_run.run_duration);
        fprintf(fp, "run_duration_ms : %.3f\n", perf_run.run_duration / 1000.0);
    } else {
        fprintf(fp, "run_duration    : query failed (%d)\n", ret_run);
    }

    fprintf(fp, "probabilities:\n");
    for (int i = 0; i < RKNN_CLASS_NUM; i++) {
        fprintf(fp, "  class[%d] = %.6f\n", i, probs[i]);
    }

    if (ret_detail == RKNN_SUCC && perf_detail.perf_data) {
        fprintf(fp, "perf_detail:\n%s\n", perf_detail.perf_data);
    } else {
        fprintf(fp, "perf_detail: query failed (%d)\n", ret_detail);
    }

    fprintf(fp, "\n");
    fclose(fp);

    chmod(log_path, 0777);
    return 0;
}
