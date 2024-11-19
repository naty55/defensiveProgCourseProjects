#ifndef OBJECTS_HPP
#define OBJECT_HPP
#include "config.hpp"
#include <crtdefs.h>
#include <ostream>

class Request {
    private: 
    unsigned char clientId[HEADER_CLIENT_ID_SIZE];
    unsigned char client_version;
    unsigned short int request_code;
    unsigned long int payload_size;
    char payload[];
    public:
    Request(
        unsigned char clientId[HEADER_CLIENT_ID_SIZE],
        unsigned char client_version,
        unsigned short int request_code,
        unsigned long int payload_size,
        char payload[]);
    void to_bytes(char* buffer, size_t size);
    size_t size_in_bytes();
};


class Response {
    private: 
    unsigned short int server_version;
    unsigned short int response_code;
    unsigned long int payload_size;
    char payload[];
    public:
    Response(char *response_bytes, size_t size);
    friend std::ostream& operator<<(std::ostream& os, const Response& response);
};

#endif