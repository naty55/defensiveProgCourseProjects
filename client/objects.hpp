#pragma once

#include "protocol.hpp"
#include <crtdefs.h>
#include <ostream>
#include <vector>

class Request {
    private: 
    RequestHeader header;
    std::vector<uint8_t> payload;
    public:
    Request(
        const uint8_t * clientId,
        uint8_t client_version,
        RequestCode request_code,
        unsigned long int payload_size,
        const uint8_t *payload);
    void to_bytes(char* buffer, size_t size);
    size_t size_in_bytes();
    friend std::ostream& operator<<(std::ostream& os, const Request& request);
};

// class RegisterRequest : public Request {
//     public:
//     RegisterRequest(unsigned char clientId[HEADER_CLIENT_ID_SIZE], unsigned char client_version, unsigned char client_name[HEADER_CLIENT_NAME_SIZE]);
// };


class Response {
    private: 
    ResponseHeader header;
    std::vector<uint8_t> payload;
    public:
    Response(const uint8_t *response_bytes, size_t size);
    ~Response();
    uint8_t *getPayload();
    size_t getPayloadSize();
    unsigned short int getResponseCode();
    friend std::ostream& operator<<(std::ostream& os, const Response& response);
};


class Message {
    private: 
    MessageHeader header;
    std::string message_content;
    public:
    Message(
        const uint8_t target_client_id[HEADER_CLIENT_ID_SIZE],
        const uint8_t message_type,
        const uint32_t content_size,
        const std::string &content);
    void to_bytes(unsigned char* buffer, size_t size);
    size_t size_in_bytes();        
    friend std::ostream& operator<<(std::ostream& os, const Message& message);
};
