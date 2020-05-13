/*
* epoll 基于非阻塞I/O事件驱动
*/

#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define MAX_EVENTS 32
#define BUFLEN 1000 * 1000 * 4 //4M
#define SERV_PORT 8080
#define TIME_OUT_SECOND 10

void recv_data(int fd, int events, void *arg);
void send_data(int fd, int events, void *arg);

/*描述就绪文件描述符相关信息
*/

struct myevent_s
{
    int fd;                                           //要监听的文件描述符
    int events;                                       //对应的监听事件
    void *arg;                                        //泛型参数
    void (*call_back)(int fd, int events, void *arg); //回调函数
    int status;
    char buf[BUFLEN];
    int len;
    long last_active; //记录每次加入红黑树g_efd的时间
};

int g_efd;                                 //epoll_create返回的全局句柄
struct myevent_s g_events[MAX_EVENTS + 1]; //自定义结构体泛型数组，+1-->listen fd;

//初始化结构体
void event_set(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg)
{
    ev->fd = fd;
    ev->call_back = call_back;
    ev->events = 0;
    ev->arg = arg;
    ev->status = 0;
    // memset(ev->buf, 0, sizeof(ev->buf));
    // ev->len = 0;
    ev->last_active = time(NULL); //调用event的时间
    return;
}

//将fd添加到epoll注册的事件合集
void event_add(int efd, int events, struct myevent_s *ev)
{
    struct epoll_event epv = {0, {0}};
    int op;
    epv.data.ptr = ev;
    epv.events = ev->events = events;

    if (ev->status == 1)
    {
        op = EPOLL_CTL_MOD;
    }
    else
    {
        op = EPOLL_CTL_ADD;
        ev->status = 1;
    }

    if (epoll_ctl(efd, op, ev->fd, &epv) < 0)
    {
        printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
    }
    else
    {
        return;
        printf("event add OK [fd=%d], op=%d, events[%0X]\n", ev->fd, op, events);
    }

    return;
}

//从事件合集中删除
void event_del(int efd, struct myevent_s *ev)
{
    struct epoll_event epv = {0, {0}};

    if (ev->status != 1)
    {
        return;
    }

    epv.data.ptr = ev;
    ev->status = 0;
    epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);

    return;
}

//当有新的连接时，调用监听listenfd的此回调函数，指定新sockfd的回调函数
void accept_conn(int lfd, int events, void *arg)
{
    printf("****************************\n");
    struct sockaddr_in cin;
    socklen_t len = sizeof(cin);
    int cfd, i;

    if ((cfd = accept(lfd, (struct sockaddr *)&cin, &len)) == -1)
    {
        if (errno != EAGAIN && errno != EINTR)
        {
            /* 暂时不做出错处理 */
        }
        printf("%s: accept, %s\n", __func__, strerror(errno));
        return;
    }

    do
    {
        for (i = 0; i < MAX_EVENTS; i++)
        {
            if (g_events[i].status == 0)
            {
                break;
            }
        }

        if (i == MAX_EVENTS)
        {
            printf("%s: max connect limit[%d]\n", __func__, MAX_EVENTS);
            break;
        }

        int flag = 0;
        if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0)
        {
            printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
            break;
        }

        event_set(&g_events[i], cfd, recv_data, &g_events[i]);
        event_add(g_efd, EPOLLIN, &g_events[i]);
    } while (0);

    printf("new connect [%s:%d][time:%ld], pos[%d]\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), g_events[i].last_active, i);

    return;
}
size_t total_bytes = 0;
struct timeval start_, stop_;
void show_performance()
{
    gettimeofday(&stop_, 0);
    long res = (stop_.tv_sec - start_.tv_sec) * 1000000 + stop_.tv_usec - start_.tv_usec;
    float perfm = 8 * total_bytes / 1000.0 / 1000 / (res / 1000000.0);
    printf("Recv rate: %10f Mbs, %5f Gbps\n", perfm, perfm / 1000);
    start_ = stop_;
    total_bytes = 0;
}
void recv_data(int fd, int events, void *arg)
{
    struct myevent_s *ev = (struct myevent_s *)arg;
    int len;

    len = recv(fd, ev->buf, sizeof(ev->buf), 0);
    event_del(g_efd, ev); //接受完数据 删除该监听事件

    if (len > 0)
    {
        ev->len = len;
        //ev->buf[len] = '\0';
        //printf("%s[%d]@%s,\t C[%d]:%s\n", __func__, __LINE__, __FILE__, fd, ev->buf);
        /* 转换为发送事件 */
        //event_set(ev, fd, send_data, ev);
        event_set(ev, fd, recv_data, ev);
        event_add(g_efd, EPOLLIN, ev); //再把其添加到监听事件中,此时回调函数改为了send_data
        //将socket事件修改为EPOLLOUT用于服务器发送消息给客户端

        total_bytes += len;
        if (total_bytes > 5000000000)
        {
            show_performance();
        }
    }
    else if (len == 0)
    {
        close(ev->fd);
        /* ev-g_events 地址相减得到偏移元素位置 */
        printf("%s[%d]@%s,\t[fd=%d] pos[%d], closed\n", __func__, __LINE__, __FILE__, fd, (int)(ev - g_events));
    }
    else
    {
        close(ev->fd);
        printf("%s[%d]@%s,\trecv[fd=%d] error[%d]:%s\n", __func__, __LINE__, __FILE__, fd, errno, strerror(errno));
    }

    return;
}

void send_data(int fd, int events, void *arg)
{
    struct myevent_s *ev = (struct myevent_s *)arg;
    int len;

    len = send(fd, ev->buf, ev->len, 0);
    // printf("fd=%d\tev->buf=%s\ttev->len=%d\n", fd, ev->buf, ev->len);
    // printf("send len = %d\n", len);

    event_del(g_efd, ev); //删除该事件
    if (len > 0)
    {
        //printf("send[fd=%d], [%d]%s\n", fd, len, ev->buf);
        event_set(ev, fd, recv_data, ev);
        event_add(g_efd, EPOLLIN, ev); //添加该事件，已更改回调函数为recv_data
        //修改socket事件为EPOLLIN用于接收客户端发来的消息
    }
    else
    {
        close(ev->fd);
        printf("send[fd=%d] error %s\n", fd, strerror(errno));
    }

    return;
}

//初始化一个sockfd
void init_listen_socket(int efd, short port)
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(lfd, F_SETFL, O_NONBLOCK);
    event_set(&g_events[MAX_EVENTS], lfd, accept_conn, &g_events[MAX_EVENTS]);
    event_add(efd, EPOLLIN, &g_events[MAX_EVENTS]);

    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    bind(lfd, (struct sockaddr *)&sin, sizeof(sin));

    listen(lfd, 20);

    return;
}

int main(int argc, char const *argv[])
{
    gettimeofday(&start_, 0);
    unsigned short port = SERV_PORT;
    if (argc == 2)
    {
        port = atoi(argv[1]);
    }

    g_efd = epoll_create(MAX_EVENTS + 1); //创建红黑树，返回全局fd
    if (g_efd < 0)
    {
        printf("create efd in %s err %s\n", __func__, strerror(errno));
        exit(-1);
    }

    init_listen_socket(g_efd, port);           //初始化监听套接字
    struct epoll_event events[MAX_EVENTS + 1]; //保存就绪事件的文件描述符数组

    printf("Server running: port [%d]\n", port);

    int checkpos = 0, i = 0;
    while (1)
    { //超时验证，每次测试100个连接，不测试listenfd 当客户端TIME_OUT_SECOND秒内没有和服务器通信，则关闭此客户端连接
        long now = time(NULL);
        for (i = 0; i < 100; i++, checkpos++)
        {
            if (checkpos == MAX_EVENTS)
                checkpos = 0;
            if (g_events[checkpos].status != 1) //不在红黑树上
                continue;
            long duration = now - g_events[checkpos].last_active;
            if (duration >= TIME_OUT_SECOND) //超时，关闭连接
            {
                close(g_events[checkpos].fd);
                printf("[fd=%d] timeout\n", g_events[checkpos].fd);
                event_del(g_efd, &g_events[checkpos]); //将客户端从红黑树摘除
            }
        }
        //监听红黑树，将就绪事件文件描述符加入到events数组中，1秒没有事件满足，返回0

        int nfd = epoll_wait(g_efd, events, MAX_EVENTS + 1, 1000);
        if (nfd < 0)
        {
            printf("epoll wait error, exit\n");
            break;
        }

        for (i = 0; i < nfd; i++)
        {
            //使用自定义结构体myevents_s指针，接受，联合体data 的void* ptr成员
            struct myevent_s *ev = (struct myevent_s *)events[i].data.ptr;
            if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)) //读就绪
                ev->call_back(ev->fd, events[i].events, ev->arg);
            if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)) //写就绪
                ev->call_back(ev->fd, events[i].events, ev->arg);
        }
    }
    return 0;
}
