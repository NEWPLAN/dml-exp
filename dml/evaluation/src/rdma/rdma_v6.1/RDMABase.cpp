#include "RDMABase.h"
#include <rdma/rdma_cma.h>

//50 M for default size;

#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>
#include <unordered_map>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdarg.h>

void RDMABase::rc_die(const char *reason)
{
    extern int errno;
    fprintf(stderr, "%s\nstrerror= %s\n", reason, strerror(errno));
    exit(-1);
}

void RDMABase::log_info(const char *format, ...)
{
    char now_time[32];
    char s[1024];
    char content[1024];
    //char *ptr = content;
    struct tm *tmnow;
    struct timeval tv;
    bzero(content, 1024);
    va_list arg;
    va_start(arg, format);
    vsprintf(s, format, arg);
    va_end(arg);

    gettimeofday(&tv, NULL);
    tmnow = localtime(&tv.tv_sec);

    sprintf(now_time, "%04d/%02d/%02d %02d:%02d:%02d:%06ld ",
            tmnow->tm_year + 1900, tmnow->tm_mon + 1, tmnow->tm_mday, tmnow->tm_hour,
            tmnow->tm_min, tmnow->tm_sec, tv.tv_usec);

    sprintf(content, "%s %s", now_time, s);
    printf("%s", content);
}

long long RDMABase::current_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

struct ibv_pd *RDMABase::rc_get_pd(struct rdma_cm_id *id)
{
    struct RDMAContext *ctx = (struct RDMAContext *)id->context;
    if (ctx == 0 || ctx == nullptr || ctx == NULL)
    {
        LOG_INFO("Error of get pd: %x\n", ctx);
        return NULL;
    }
    return ctx->pd;
}

void RDMABase::build_params(struct rdma_conn_param *params)
{
    LOG_INFO("Building RDMA params in %s\n", __FUNCTION__);

    memset(params, 0, sizeof(*params));

    params->initiator_depth = params->responder_resources = 1;
    params->rnr_retry_count = 7; /* infinite retry */
    params->retry_count = 7;
}

void RDMABase::build_context(struct rdma_cm_id *id)
{
    struct ibv_context *verbs = id->verbs;
    struct RDMAContext *ctx = (struct RDMAContext *)id->context;

    if (ctx)
    {
        if (ctx->ibv_ctx != verbs)
            rc_die("cannot handle events in more than one context.");
        return;
    }

    struct RDMAContext *new_ctx = (struct RDMAContext *)malloc(sizeof(struct RDMAContext));
    id->context = (void *)new_ctx;
    new_ctx->ibv_ctx = verbs;

    TEST_Z(new_ctx->pd = ibv_alloc_pd(new_ctx->ibv_ctx));
    TEST_Z(new_ctx->comp_channel = ibv_create_comp_channel(new_ctx->ibv_ctx));
    TEST_Z(new_ctx->cq = ibv_create_cq(new_ctx->ibv_ctx,
                                       MAX_DATA_IN_FLIGHT * 2 + 10,
                                       NULL,
                                       new_ctx->comp_channel,
                                       0));
    TEST_NZ(ibv_req_notify_cq(new_ctx->cq, 0));
    //LOG_INFO("Show ctx: %p, in function: %s\n", id->context, __FUNCTION__);
}

void RDMABase::build_qp_attr(struct rdma_cm_id *id,
                             struct ibv_qp_init_attr *qp_attr)
{
    struct RDMAContext *ctx = (struct RDMAContext *)id->context;
    if (ctx == 0 || ctx == nullptr || ctx == NULL)
    {
        LOG_INFO("Error of building QP_attr: %x\n", ctx);
        exit(-1);
    }
    memset(qp_attr, 0, sizeof(*qp_attr));

    qp_attr->send_cq = ctx->cq;
    qp_attr->recv_cq = ctx->cq;

    qp_attr->sq_sig_all = 1; //new added.

    qp_attr->qp_type = IBV_QPT_RC;

    qp_attr->cap.max_send_wr = MAX_DATA_IN_FLIGHT + 10 + 1;
    qp_attr->cap.max_recv_wr = MAX_DATA_IN_FLIGHT + 10 + 1;
    qp_attr->cap.max_send_sge = 1;
    qp_attr->cap.max_recv_sge = 1;
}

void RDMABase::build_connection(struct rdma_cm_id *id)
{
    LOG_INFO("Build connection: %p\n", id->context);
    struct ibv_qp_init_attr qp_attr;
    build_context(id);
    build_qp_attr(id, &qp_attr);
    struct RDMAContext *ctx = (struct RDMAContext *)id->context;
    TEST_NZ(rdma_create_qp(id, ctx->pd, &qp_attr));

    LOG_INFO("Show ctx: %p, in function: %s\n", id->context, __FUNCTION__);
}

#include <mutex>
#include <iostream>

std::mutex mtx;
size_t perf_data = 0;
void RDMABase::show_performance(int time_duration)
{
    size_t tmp_perf = 0;
    {
        std::lock_guard<std::mutex> myltx(mtx);
        tmp_perf = perf_data;
        perf_data = 0;
    }
    printf("total performance: %.2f Gbps\n",
           8.0 * tmp_perf / 1000 / 1000 / 1000 / time_duration);
}
void RDMABase::add_performance(size_t data_num)
{
    std::lock_guard<std::mutex> myltx(mtx);
    perf_data += data_num;
}