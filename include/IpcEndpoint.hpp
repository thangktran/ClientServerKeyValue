#pragma once

#include <string>
#include <string_view>
#include <optional>

#include <semaphore.h>

namespace cskv {


constexpr unsigned BUFFER_SIZE = 1024;
typedef struct shmbuf {
    sem_t  semReq;            /* POSIX unnamed semaphore */
    sem_t  semRes;            /* POSIX unnamed semaphore */
    sem_t  semSlot;           /* 1 req+res cycle*/
    size_t cnt;               /* Number of bytes used in 'buf' */
    char   buf[BUFFER_SIZE];  /* Data being transferred */
}shmbuf;


class IpcEndpoint {

public:
    IpcEndpoint(std::string_view endpointName, bool create = false);
    ~IpcEndpoint();

    std::optional<size_t> writeReq(const void* buf, size_t len);
    std::optional<size_t> readRes(void* buf, size_t len);

    std::optional<size_t> readReq(void* buf, size_t len);
    std::optional<size_t> writeRes(const void* buf, size_t len);
    
private:
    std::string _endpointName;
    bool _owner;
    int _shmFd;
    shmbuf* _shm;
};


} // namespace cskv