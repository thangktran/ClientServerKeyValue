#pragma once

#include <cstddef>

namespace cskv::protocol {

const char* LISTEN_SERVER = "ClientServerKVListenServer";
const char* SPECIAL_EXIT_SEQUENCE = "DO_EXIT";
constexpr size_t VALUE_SIZE = 100;


typedef struct ConnectionRequest {
    char    name[VALUE_SIZE];
    size_t  len;
} ConnectionRequest;


typedef struct ConnectionResponse {
    bool success;
} ConnectionResponse;


enum class KvCommand : unsigned char {
    INSERT,
    DEL,
    GET,
    FINISH
};

typedef struct KvRequest {
    KvCommand  cmd;
    unsigned key;
    char     value[VALUE_SIZE];
    size_t   len;
}KvRequest;

typedef struct KvResponse {
    bool   success;
    char   value[VALUE_SIZE];
    size_t len;
}KvResponse;

} // namespace cskv