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