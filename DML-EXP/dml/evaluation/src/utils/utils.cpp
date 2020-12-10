#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>
std::uint32_t lwp_id()
{
#if defined(APPLE)
    return static_cast<std::uint32_t>(std::this_thread::get_id());
#else
    return static_cast<std::uint32_t>(syscall(SYS_gettid));
#endif
}

#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

int ip_parser(const char *ip_prefix, int ip_prefix_length)
{
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpAddrPtr = NULL;

    int node_id = -1;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) // check it is IP4
        {
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (strncmp(addressBuffer, ip_prefix, ip_prefix_length) == 0)
            {
                node_id = atoi(addressBuffer + ip_prefix_length);
                printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
            }
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6) // check it is IP6
        {
            continue;
            // is a valid IP6 Address
            tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
        }
    }
    if (ifAddrStruct != NULL)
    {
        freeifaddrs(ifAddrStruct);
    }

    return node_id;
}