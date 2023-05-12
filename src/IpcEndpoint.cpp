#include "include/IpcEndpoint.hpp"

#include <iostream>
#include <cassert>
#include <cstring>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>


namespace cskv {



void exitOnError(std::string_view msg) {
    perror(msg.data());
    exit(EXIT_FAILURE);
}

IpcEndpoint::IpcEndpoint(std::string_view endpointName, bool create)
    : _endpointName(endpointName),
    _owner(create),
    _shmFd(-1),
    _shm(nullptr) {
    
    if (_owner) {

        _shmFd = shm_open(_endpointName.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); 

        if(_shmFd == -1) {
            exitOnError("shm_open");
        }

        if (ftruncate(_shmFd, sizeof(shmbuf)) == -1) {
            exitOnError("ftruncate");
        }

        void* ptr = mmap(NULL, sizeof(*_shm), PROT_READ | PROT_WRITE, MAP_SHARED, _shmFd, 0);
        _shm = static_cast<shmbuf*>(ptr);

        if (_shm == MAP_FAILED) {
            exitOnError("mmap");
        }

        if (sem_init(&_shm->semReq, 1, 0) == -1) {
            exitOnError("_shm->semReq");
        }

        if (sem_init(&_shm->semRes, 1, 0) == -1) {
            exitOnError("_shm->semRes");
        }

        if (sem_init(&_shm->semSlot, 1, 1) == -1) {
            exitOnError("_shm->semSlot");
        }

    }
    else {

        _shmFd = shm_open(_endpointName.c_str(), O_RDWR, 0);

        if(_shmFd == -1) {
            exitOnError("shm_open");
        }

        void* ptr = mmap(NULL, sizeof(*_shm), PROT_READ | PROT_WRITE, MAP_SHARED, _shmFd, 0);
        _shm = static_cast<shmbuf*>(ptr);

        if (_shm == MAP_FAILED) {
            exitOnError("mmap");
        }

    }

    
}


IpcEndpoint::~IpcEndpoint() {

    if (_owner) {
        shm_unlink(_endpointName.c_str());
    }

}


std::optional<size_t> IpcEndpoint::writeReq(const void* buf, size_t len) {

    if (len > BUFFER_SIZE) {
        std::cerr << "writeReq: size too large" << std::endl;
        return std::nullopt;
    }

    // wait until there's no other running request.
    if (sem_wait(&_shm->semSlot) == -1) {
        exitOnError("sem_post(&_shm->semSlot)");
    }

    _shm->cnt = len;
    std::memcpy(&_shm->buf, buf, len);

    if (sem_post(&_shm->semReq) == -1) {
        exitOnError("sem_post(&_shm->semReq)");
    }

    return len;
}


std::optional<size_t> IpcEndpoint::readRes(void* buf, size_t len) {

    if (len > BUFFER_SIZE) {
        std::cerr << "readRes: size too large" << std::endl;
        return std::nullopt;
    }

    if (sem_wait(&_shm->semRes) == -1) {
        exitOnError("sem_wait(&_shm->semRes)");
    }

    size_t readLen = _shm->cnt;
    std::memcpy(buf, &_shm->buf, len);

    // got our response. Finish request cycle.
    if (sem_post(&_shm->semSlot) == -1) {
        exitOnError("sem_post(&_shm->semSlot)");
    }

    return readLen;
}


std::optional<size_t> IpcEndpoint::readReq(void* buf, size_t len) {

    if (len > BUFFER_SIZE) {
        std::cerr << "readReq: size too large" << std::endl;
        return std::nullopt;
    }

    if (sem_wait(&_shm->semReq) == -1) {
        exitOnError("sem_wait(&_shm->semReq)");
    }

    std::memcpy(buf, &_shm->buf, len);

    return _shm->cnt;
}


std::optional<size_t> IpcEndpoint::writeRes(const void* buf, size_t len) {

    if (len > BUFFER_SIZE) {
        std::cerr << "writeRes: size too large" << std::endl;
        return std::nullopt;
    }

    _shm->cnt = len;
    std::memcpy(&_shm->buf, buf, len);

    if (sem_post(&_shm->semRes) == -1) {
        exitOnError("sem_post(&_shm->semRes)");
    }    

    return len;
}


} // namespace cskv