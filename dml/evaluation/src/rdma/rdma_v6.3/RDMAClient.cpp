#include "RDMABase.h"
#include "RDMAClient.h"

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
#include <pthread.h>

#include <iostream>

#include <thread>
#include <fcntl.h>

RDMAClient::RDMAClient(RDMAAdapter &rdma_adapter)
{
    this->rdma_adapter_ = rdma_adapter;
    std::cout << "Creating RDMAClient" << std::endl;
}
RDMAClient::~RDMAClient()
{
    std::cout << "Destroying RDMAClient" << std::endl;
}

void RDMAClient::setup()
{
    std::cout << "Configure RDMAClient" << std::endl;
    this->_init();
}
void RDMAClient::start_service()
{
    std::cout << "RDMAClient starts service" << std::endl;
}

void RDMAClient::_send_loops()
{
    std::cout << "In RDMAClient send loops" << std::endl;
}

void RDMAClient::_init()
{
    std::cout << "Initing RDMAClient" << std::endl;

    // if (this->ctx == 0)
    // {
    //     this->ctx = new RDMAContext();
    //     LOG_INFO("RDMAContext: %p\n", this->ctx);
    // }

    struct rdma_cm_id *conn = NULL;
    struct rdma_event_channel *ec = NULL;

    std::string local_eth = this->rdma_adapter_.client_ip; /*get each lev ip*/

    memset(&ser_in, 0, sizeof(ser_in));
    memset(&local_in, 0, sizeof(local_in));

    /*bind remote socket*/
    ser_in.sin_family = AF_INET;
    ser_in.sin_port = htons(this->rdma_adapter_.server_port); /*connect to public port remote*/
    inet_pton(AF_INET, this->rdma_adapter_.server_ip.c_str(),
              &ser_in.sin_addr);

    /*bind local part*/
    local_in.sin_family = AF_INET;

    std::cout << local_eth.c_str() << "----->" << this->rdma_adapter_.server_ip.c_str() << std::endl;
    inet_pton(AF_INET, local_eth.c_str(), &local_in.sin_addr);

    TEST_Z(ec = rdma_create_event_channel());
    TEST_NZ(rdma_create_id(ec, &conn, NULL, RDMA_PS_TCP));
    TEST_NZ(rdma_resolve_addr(conn, (struct sockaddr *)(&local_in),
                              (struct sockaddr *)(&ser_in), TIMEOUT_IN_MS));

    conn->context = (void *)this->ctx;

    this->event_loop(ec);

    std::cout << "RDMAClient is launched" << std::endl;
    while (1)
    {
        std::cout << "main thread sleep for 10 seconds" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    rdma_destroy_event_channel(ec);
}

void RDMAClient::event_loop(struct rdma_event_channel *ec)
{
    struct rdma_cm_event *event = NULL;
    struct rdma_conn_param cm_params;

    build_params(&cm_params);

    int connect_count = 0;
    struct rdma_cm_id *conn = NULL;

    while (rdma_get_cm_event(ec, &event) == 0)
    {
        struct rdma_cm_event event_copy;
        memcpy(&event_copy, event, sizeof(*event));
        rdma_ack_cm_event(event);

        switch (event_copy.event)
        {
        case RDMA_CM_EVENT_ADDR_RESOLVED:
        {
            //LOG_INFO("In %s\n", "RDMA_CM_EVENT_ADDR_RESOLVED");
            build_connection(event_copy.id);
            TEST_NZ(rdma_resolve_route(event_copy.id,
                                       TIMEOUT_IN_MS));
            break;
        }
        case RDMA_CM_EVENT_ROUTE_RESOLVED:
        {
            //LOG_INFO("In %s\n", "RDMA_CM_EVENT_ROUTE_RESOLVED");
            TEST_NZ(rdma_connect(event_copy.id, &cm_params));
            break;
        }
        case RDMA_CM_EVENT_CONNECT_REQUEST:
        {
            printf("Error: client never request a connection\n");
            exit(-1);
            break;
        }
        case RDMA_CM_EVENT_ESTABLISHED:
        {
            //LOG_INFO("In %s\n", "RDMA_CM_EVENT_ESTABLISHED");
            on_pre_conn(event_copy.id);
            this->send_thread = new std::thread([this, event_copy]() {
                this->poll_cq((void *)(event_copy.id));
            });

            std::cout << this->rdma_adapter_.client_ip << " has connected to server [" << this->rdma_adapter_.server_ip << ", " << this->rdma_adapter_.server_port << "]" << std::endl;

            //  exit if connected to remote;
            //break;
            break;
        }
        case RDMA_CM_EVENT_REJECTED:
        {
            //LOG_INFO("In %s\n", "RDMA_CM_EVENT_REJECTED");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            connect_count++;
            struct RDMAContext *ctx_ = (struct RDMAContext *)(event_copy.id->context);
            rdma_destroy_qp(event_copy.id);
            rdma_destroy_id(event_copy.id);
            rdma_destroy_event_channel(ec);

            if (connect_count > 10 * 300) //after 600 seconds, it will exit.
            {
                std::cerr << 300 << "seconds is passed, error in connect to server" << this->rdma_adapter_.server_ip << ", check your network condition" << std::endl;
                exit(-1);
            }
            else
            {
                if (connect_count % 10 == 0)
                    std::cout << "Failed when connect to server, Check your Server [" << this->rdma_adapter_.server_ip << ":" << this->rdma_adapter_.server_port << "] is launched" << std::endl;

                TEST_Z(ec = rdma_create_event_channel());
                TEST_NZ(rdma_create_id(ec, &conn, NULL, RDMA_PS_TCP));
                TEST_NZ(rdma_resolve_addr(conn, (struct sockaddr *)(&local_in), (struct sockaddr *)(&ser_in), TIMEOUT_IN_MS));

                conn->context = (void *)ctx_;
            }
            break;
        }
        case RDMA_CM_EVENT_DISCONNECTED:
        {
            LOG_INFO("In %s\n", "RDMA_CM_EVENT_DISCONNECTED");
            rdma_destroy_qp(event_copy.id);
            on_disconnect(event_copy.id);
            rdma_destroy_id(event_copy.id);
            exit(-1);
        }
        default:
            break;
        }
        /*
        if (event_copy.event == RDMA_CM_EVENT_ADDR_RESOLVED)
        {
            printf("RDMA_CM_EVENT_ADDR_RESOLVED\n");
            build_connection(event_copy.id);
            //on_pre_con
            TEST_NZ(rdma_resolve_route(event_copy.id, TIMEOUT_IN_MS));
        }
        else if (event_copy.event == RDMA_CM_EVENT_ROUTE_RESOLVED)
        {
            printf("RDMA_CM_EVENT_ROUTE_RESOLVED\n");
            TEST_NZ(rdma_connect(event_copy.id, &cm_params));
        }
        else if (event_copy.event == RDMA_CM_EVENT_CONNECT_REQUEST)
        {
            printf("Client should never go to the request connection\n");
            exit(-1);
        }
        else if (event_copy.event == RDMA_CM_EVENT_ESTABLISHED)
        {
            printf("RDMA_CM_EVENT_ESTABLISHED\n");
            on_pre_conn(event_copy.id);
            this->send_thread = new std::thread([this, event_copy]() {
                this->poll_cq((void *)(event_copy.id));
            });

            std::cout << this->rdma_adapter_.client_ip << " has connected to server [" << this->rdma_adapter_.server_ip << ", " << this->rdma_adapter_.server_port << "]" << std::endl;

            //  exit if connected to remote;
            //break;
        }
        else if (event_copy.event == RDMA_CM_EVENT_REJECTED)
        {
            printf("RDMA CM EVENT REJECTED\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            connect_count++;

            //rdma_on_disconnect(event_copy.id);
            rdma_destroy_qp(event_copy.id);
            rdma_destroy_id(event_copy.id);
            rdma_destroy_event_channel(ec);

            if (connect_count > 10 * 300) //after 600 seconds, it will exit.
            {
                //release all resources and close all;

                std::cerr << 300 << "seconds is passed, error in connect to server" << this->rdma_adapter_.server_ip << ", check your network condition" << std::endl;
                exit(-1);
            }
            else
            {
                if (connect_count % 10 == 0)
                    std::cout << "Failed when connect to server, Check your Server [" << this->rdma_adapter_.server_ip << ":" << this->rdma_adapter_.server_port << "] is launched" << std::endl;
                struct RDMAContext *ctx_ = (struct RDMAContext *)(event_copy.id->context);
                TEST_Z(ec = rdma_create_event_channel());
                TEST_NZ(rdma_create_id(ec, &conn, NULL, RDMA_PS_TCP));
                TEST_NZ(rdma_resolve_addr(conn, (struct sockaddr *)(&local_in), (struct sockaddr *)(&ser_in), TIMEOUT_IN_MS));

                conn->context = (void *)ctx_;
            }
        }
        else if (event_copy.event == RDMA_CM_EVENT_DISCONNECTED)
        {
            printf("RDMA_CM_EVENT_DISCONNECTED\n");
            rdma_destroy_qp(event_copy.id);

            on_disconnect(event_copy.id);
            rdma_destroy_id(event_copy.id);

            exit(-1);
        }
        else
        {
            printf("event = %d\n", event_copy.event);
            rc_die("unknown event client\n");
        }
        */
    }
}

void *RDMAClient::poll_cq(void *_id)
{
    struct ibv_cq *cq = NULL;
    struct ibv_wc wc;

    struct rdma_cm_id *id = (rdma_cm_id *)_id;

    struct RDMAContext *ctx = (struct RDMAContext *)id->context;
    //struct RDMAContext *ctx = s_ctx;

    void *ev_ctx = NULL;

    while (1)
    {
        TEST_NZ(ibv_get_cq_event(ctx->comp_channel, &cq, &ev_ctx));
        ibv_ack_cq_events(cq, 1);
        TEST_NZ(ibv_req_notify_cq(cq, 0));
        int nc = 0;

        if (0)
        {
            do
            {
                nc = ibv_poll_cq(cq, 1, &wc);
                for (int index = 0; index < nc; index++)
                {
                }
            } while (nc > 0);
        }

        while (ibv_poll_cq(cq, 1, &wc))
        {
            if (wc.status == IBV_WC_SUCCESS)
                on_completion(&wc);
            else
                rc_die("poll_cq: status is not IBV_WC_SUCCESS");
        }
    }
    return NULL;
}

void RDMAClient::on_pre_conn(struct rdma_cm_id *id)
{
    int ret;

    ctx = (struct RDMAContext *)id->context;
    ctx->id = id;

    ret = posix_memalign((void **)&ctx->buffer,
                         sysconf(_SC_PAGESIZE),
                         BUFFER_SIZE);

    if (ret)
    {
        fprintf(stderr, "posix_memalign: %s\n",
                strerror(ret));
        exit(-1);
    }

    TEST_Z(ctx->buffer_mr = ibv_reg_mr(rc_get_pd(id),
                                       ctx->buffer,
                                       BUFFER_SIZE,
                                       0));

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
                                                   sizeof(struct message), IBV_ACCESS_LOCAL_WRITE));

        post_receive(msg_index);
    }
    LOG_INFO("Client register buffer: %p\n", ctx->buffer);
}

void RDMAClient::on_disconnect(struct rdma_cm_id *id)
{
    log_info("[Client] In function [%d]:%s\n", __LINE__, __func__);
    struct RDMAContext *new_ctx = (struct RDMAContext *)id->context;

    ibv_dereg_mr(new_ctx->buffer_mr);
    free(new_ctx->buffer);

    {
        //unregister msf
    }

    free(new_ctx);
}

void RDMAClient::post_receive(uint32_t msg_id)
{
    struct rdma_cm_id *id = ctx->id;
    struct ibv_recv_wr wr,
        *bad_wr = NULL;
    struct ibv_sge sge;

    memset(&wr, 0, sizeof(wr));

    wr.wr_id = msg_id;
    wr.sg_list = &sge;
    wr.num_sge = 1;

    sge.addr = (uintptr_t)ctx->msg[msg_id];
    sge.length = sizeof(struct message);
    sge.lkey = ctx->msg_mr[msg_id]->lkey;

    TEST_NZ(ibv_post_recv(id->qp, &wr, &bad_wr));
}

void RDMAClient::write_remote(uint32_t buffer_id, uint32_t len)
{
    struct rdma_cm_id *id = ctx->id;

    struct ibv_send_wr wr, *bad_wr = NULL;
    struct ibv_sge sge;

    memset(&wr, 0, sizeof(wr));

    uint32_t imm_data_prepared = buffer_id << 24;
    imm_data_prepared &= 0XFF000000;
    imm_data_prepared += len;

    wr.wr_id = buffer_id; //(uintptr_t)id;
    wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.imm_data = htonl(imm_data_prepared);

    wr.wr.rdma.remote_addr = ctx->peer_addr + buffer_id * BUFFER_SIZE;
    wr.wr.rdma.rkey = ctx->peer_rkey;

    if (len)
    {
        wr.sg_list = &sge;
        wr.num_sge = 1;

        sge.addr = (uintptr_t)ctx->buffer;
        sge.length = len;
        sge.lkey = ctx->buffer_mr->lkey;
    }

    TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));
}

void RDMAClient::send_next_chunk(uint32_t buffer_id)
{
    //struct client_context *ctx = (struct client_context *)id->context;

    ssize_t size = 0;

    //size = read(ctx->fd, ctx->buffer, BUFFER_SIZE);
    size = BUFFER_SIZE;

    if (size == -1)
        rc_die("read() failed\n");

    write_remote(buffer_id, size);
}

void RDMAClient::send_file_name(struct rdma_cm_id *id)
{
    struct RDMAContext *ctx = (struct RDMAContext *)id->context;
    this->ip_addr_ = rdma_adapter_.get_client_ip();
    sprintf(ctx->buffer, "0%s0", this->ip_addr_.c_str());
    printf("Sending file name: %s\n", this->ip_addr_.c_str());
    //strcpy(ctx->buffer, "0120");
    write_remote(0, 2 + this->ip_addr_.length());
}

void RDMAClient::on_completion(struct ibv_wc *wc)
{

    struct rdma_cm_id *id = ctx->id;
    uint32_t msg_id = wc->wr_id;

    if (wc->opcode & IBV_WC_RECV)
    {

        if (ctx->msg[msg_id]->id == MSG_MR)
        {
            ctx->peer_addr = ctx->msg[msg_id]->data.mr.addr;
            ctx->peer_rkey = ctx->msg[msg_id]->data.mr.rkey;

            printf("received MR, sending file name\n");
            send_file_name(id);
        }
        else if (ctx->msg[msg_id]->id == MSG_READY)
        {
            //printf("received READY, sending chunk\n");
            ctx->buffer[0] = '0' + msg_id;
            ctx->buffer[BUFFER_SIZE - 1] = '0' + msg_id;
            send_next_chunk(msg_id);
        }
        else if (ctx->msg[msg_id]->id == MSG_DONE)
        {
            printf("received DONE, disconnecting\n");
            //rc_disconnect(id);
            rdma_disconnect(id);
            return;
        }

        post_receive(msg_id);
    }
}