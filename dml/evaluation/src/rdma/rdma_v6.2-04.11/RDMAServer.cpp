#include "RDMAServer.h"
#include <iostream>
#include <rdma/rdma_cma.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <iostream>

#include <thread>
#include <fcntl.h>
//#include "../utils/ATimer.h"

RDMAServer::RDMAServer(RDMAAdapter &rdma_adapter)
{
    this->rdma_adapter_ = rdma_adapter;
    std::cout << "Creating RDMAServer" << std::endl;
}
RDMAServer::~RDMAServer()
{
    std::cout << "Destroy RDMAServer" << std::endl;
}

void RDMAServer::setup()
{
    std::cout << "Setup RDMAServer" << std::endl;
    this->_init();
}
void RDMAServer::start_service()
{
    std::cout << "RDMA is starting service" << std::endl;
}

void RDMAServer::server_event_loops()
{
    LOG_INFO("In function: %s\n", __FUNCTION__);

    struct rdma_cm_event *event = NULL;
    struct rdma_conn_param cm_params;

    printf("RDMAServer is inited, waiting for client connection\n");

    build_params(&cm_params);

    while (rdma_get_cm_event(this->rdma_adapter_.event_channel, &event) == 0)
    {
        struct rdma_cm_event event_copy;

        memcpy(&event_copy, event, sizeof(*event));
        rdma_ack_cm_event(event);

        switch (event_copy.event)
        {
        case RDMA_CM_EVENT_CONNECT_REQUEST:
        {
            LOG_INFO("Recv msg: %s\n", "RDMA_CM_EVENT_CONNECT_REQUEST");

            build_connection(event_copy.id);
            on_pre_conn(event_copy.id);
            TEST_NZ(rdma_accept(event_copy.id, &cm_params));
            break;
        }
        case RDMA_CM_EVENT_ESTABLISHED:
        {
            LOG_INFO("Recv msg: %s\n", "RDMA_CM_EVENT_ESTABLISHED");

            on_connection(event_copy.id);
            this->rdma_adapter_.recv_rdma_cm_id.push_back(event_copy.id);

            struct sockaddr *peer_addr = rdma_get_peer_addr(event_copy.id);
            struct sockaddr_in *client_addr = (struct sockaddr_in *)peer_addr;
            printf("A connection from [%s:%d] is established\n",
                   inet_ntoa(client_addr->sin_addr), client_addr->sin_port);
            break;
        }
        case RDMA_CM_EVENT_DISCONNECTED:
        {
            LOG_INFO("Recv msg: %s\n", "RDMA_CM_EVENT_DISCONNECTED");

            rdma_destroy_qp(event_copy.id);
            on_disconnect(event_copy.id);
            rdma_destroy_id(event_copy.id);
            break;
        }
        default:
            rc_die("unknown event server\n");
            break;
        }
    }

    while (1)
    {
        std::cout << "Should never get here" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    printf("RDMA recv loops exit now...\n");
    return;
}
void RDMAServer::_init()
{
    //LOG_INFO("Initing RDMAServer\n");

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;                              /*ipv4*/
    sin.sin_port = htons(this->rdma_adapter_.server_port); /*server listen public ports*/
    sin.sin_addr.s_addr = INADDR_ANY;                      /*listen any connects*/

    LOG_INFO("Listen port: %d\n", this->rdma_adapter_.server_port);

    TEST_Z(this->rdma_adapter_.event_channel = rdma_create_event_channel());
    TEST_NZ(rdma_create_id(this->rdma_adapter_.event_channel,
                           &this->rdma_adapter_.listener,
                           NULL, RDMA_PS_TCP));

    TEST_NZ(rdma_bind_addr(this->rdma_adapter_.listener,
                           (struct sockaddr *)&sin));

    TEST_NZ(rdma_listen(this->rdma_adapter_.listener,
                        100));

    this->aggregator_thread = new std::thread([this]() { this->server_event_loops(); });

    while (1)
    {
        int time_duration = 5;
        std::this_thread::sleep_for(std::chrono::seconds(time_duration));
        this->show_performance(time_duration);
    }
}

void *RDMAServer::poll_cq(void *_id)
{
    struct ibv_cq *cq = NULL;
    //struct ibv_wc wc;
    struct ibv_wc wcs[MAX_DATA_IN_FLIGHT * 2];
    struct rdma_cm_id *id = (rdma_cm_id *)_id;

    struct RDMAContext *ctx = (struct RDMAContext *)id->context;
    if (ctx == NULL || ctx == 0 || ctx == nullptr)
    {
        LOG_INFO("Error in get ctx: %x, %s\n", ctx, __FUNCTION__);
        exit(-1);
    }

    void *ev_ctx = NULL;

    while (true)
    {
        TEST_NZ(ibv_get_cq_event(ctx->comp_channel, &cq, &ev_ctx));
        ibv_ack_cq_events(cq, 1);
        TEST_NZ(ibv_req_notify_cq(cq, 0));

        int nc = 0;

        do
        {
            nc = ibv_poll_cq(cq, MAX_DATA_IN_FLIGHT * 2, wcs);
            for (int index = 0; index < nc; index++)
            {
                if (wcs[index].status == IBV_WC_SUCCESS)
                {
                    on_completion(&wcs[index]);
                }
                else
                    rc_die("poll_cq: status is not IBV_WC_SUCCESS");
            }
        } while (nc > 0);
    }
    return NULL;
}
void RDMAServer::on_connection(struct rdma_cm_id *id)
{
    struct RDMAContext *ctx = (struct RDMAContext *)id->context;

    ctx->msg[0]->id = MSG_MR;

    ctx->msg[0]->data.mr.addr = (uintptr_t)ctx->buffer_mr->addr;
    ctx->msg[0]->data.mr.rkey = ctx->buffer_mr->rkey;

    send_message(id, 0);
}

static int client_index = 0;
void RDMAServer::build_context(struct rdma_cm_id *id)
{
    RDMABase::build_context(id);

    struct RDMAContext *ctx = (struct RDMAContext *)id->context;
    ctx->client_index = client_index++;
    std::thread *recv_thread = new std::thread([this, id]() {
        this->poll_cq((void *)id);
    });
    recv_threads.push_back(recv_thread);
}

void RDMAServer::on_pre_conn(struct rdma_cm_id *id)
{

    struct RDMAContext *ctx = (struct RDMAContext *)id->context;

    int ret = 0;

    ret = posix_memalign((void **)&ctx->buffer,
                         sysconf(_SC_PAGESIZE),
                         BUFFER_SIZE * MAX_DATA_IN_FLIGHT);
    if (ret)
    {
        fprintf(stderr, "posix_memalign: %s\n",
                strerror(ret));
        exit(-1);
    }
    TEST_Z(ctx->buffer_mr = ibv_reg_mr(rc_get_pd(id),
                                       ctx->buffer,
                                       BUFFER_SIZE * MAX_DATA_IN_FLIGHT,
                                       IBV_ACCESS_LOCAL_WRITE |
                                           IBV_ACCESS_REMOTE_WRITE));

    for (int msg_index = 0; msg_index < MAX_DATA_IN_FLIGHT; msg_index++)
    {
        ret = posix_memalign((void **)&ctx->msg[msg_index],
                             sysconf(_SC_PAGESIZE),
                             sizeof(struct message));
        if (ret)
        {
            fprintf(stderr, "posix_memalign: %s\n",
                    strerror(ret));
            exit(-1);
        }

        TEST_Z(ctx->msg_mr[msg_index] = ibv_reg_mr(rc_get_pd(id),
                                                   ctx->msg[msg_index],
                                                   sizeof(struct message),
                                                   0));
    }
    for (int msg_index = 0; msg_index < MAX_DATA_IN_FLIGHT; msg_index++)
    {
        post_receive(id);
    }
}

void RDMAServer::on_disconnect(struct rdma_cm_id *id)
{
    struct RDMAContext *new_ctx = (struct RDMAContext *)id->context;

    ibv_dereg_mr(new_ctx->buffer_mr);
    free(new_ctx->buffer);

    for (int msg_index = 0; msg_index < MAX_DATA_IN_FLIGHT; msg_index++)
    {
        ibv_dereg_mr(new_ctx->msg_mr[msg_index]);
        free(new_ctx->msg[msg_index]);
    }

    printf("Disconnection: Say goodbye with %s\n",
           new_ctx->connection_id);
    free(new_ctx);
}

void RDMAServer::post_receive(struct rdma_cm_id *id)
{
    struct ibv_recv_wr wr, *bad_wr = NULL;

    memset(&wr, 0, sizeof(wr));

    wr.wr_id = (uintptr_t)id;
    wr.sg_list = NULL;
    wr.num_sge = 0;

    TEST_NZ(ibv_post_recv(id->qp, &wr, &bad_wr));
}

void RDMAServer::send_message(struct rdma_cm_id *id, uint32_t token_id)
{
    struct RDMAContext *ctx = (struct RDMAContext *)id->context;

    struct ibv_send_wr wr, *bad_wr = NULL;
    struct ibv_sge sge;

    memset(&wr, 0, sizeof(wr));

    wr.wr_id = (uintptr_t)id;
    wr.opcode = IBV_WR_SEND;
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.send_flags = IBV_SEND_SIGNALED;

    //printf("Send msg with: %d\n", token_id);

    sge.addr = (uintptr_t)ctx->msg[token_id];
    sge.length = sizeof(struct message);
    sge.lkey = ctx->msg_mr[token_id]->lkey;

    TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));
}

void RDMAServer::on_completion(struct ibv_wc *wc)
{
    struct rdma_cm_id *id = (struct rdma_cm_id *)(uintptr_t)wc->wr_id;
    struct RDMAContext *ctx = (struct RDMAContext *)id->context;

    if (wc->opcode == IBV_WC_RECV_RDMA_WITH_IMM)
    {
        uint32_t imm_data_recv = ntohl(wc->imm_data);
        uint32_t size = imm_data_recv & 0XFFFFFF;
        uint32_t buffer_id = (imm_data_recv >> 24) & 0xFF;

        if (buffer_id != ctx->buffer[buffer_id * BUFFER_SIZE] ||
            buffer_id != ctx->buffer[buffer_id * BUFFER_SIZE + size - 1])
        {
            printf("Unknown error: Recv buffer id: %u, size: %u, data:[%d - %d]\n",
                   buffer_id, size, ctx->buffer[buffer_id * BUFFER_SIZE],
                   ctx->buffer[buffer_id * BUFFER_SIZE + size - 1]);
            exit(-1);
        }

        if (size == 0)
        {
            ctx->msg[buffer_id]->id = MSG_DONE;
            send_message(id, buffer_id);
        }
        else
        {
            if (ctx->connection_id[0] == 0)
            {
                memcpy(ctx->connection_id, ctx->buffer + 1, size);
                ctx->connection_id[size - 2] = 0;
            }

            ctx->recv_bytes += size;
            if (ctx->recv_bytes > 10000000000)
            {
                struct timeval now;
                gettimeofday(&now, NULL);
                float time_cost = (now.tv_usec - ctx->start.tv_usec) / 1000000.0 + now.tv_sec - ctx->start.tv_sec;
                ctx->start = now;

                printf("[%s] Recv rate: %.2f Gbps\n",
                       ctx->connection_id,
                       8.0 * ctx->recv_bytes / 1000.0 / 1000.0 / 1000.0 / time_cost);

                ctx->recv_bytes = 0;
            }

            {
                this->add_performance(size);
            }
            int base_addr = buffer_id * BUFFER_SIZE;
            ctx->buffer[base_addr] = ctx->buffer[base_addr + size - 1] = 0;
            post_receive(id);

            ctx->msg[buffer_id]->id = MSG_READY;
            ctx->msg[buffer_id]->data.batch_index[0] = buffer_id;
            send_message(id, buffer_id);
        }
    }
}