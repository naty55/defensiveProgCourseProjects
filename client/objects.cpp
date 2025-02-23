#include "objects.hpp"
#include "config.hpp"
#include <cstring>
#include <iostream>

Request::Request(
    const uint8_t *clientId,
    uint8_t client_version,
    RequestCode request_code,
    unsigned long int payload_size,
    const uint8_t *payload) {
        std::memcpy(header.client_id, clientId, HEADER_CLIENT_ID_SIZE);
        header.client_version = client_version;
        header.request_code = request_code;
        header.payload_size = payload_size;
        if (payload != nullptr) {
            std::cout << "Payload: " << payload << std::endl;
            this->payload.assign(payload, payload + payload_size);
        }
}

void Request::to_bytes(char * buffer, size_t size)
{   
    if (size < size_in_bytes()) {
        std::cout << "Buffer size is less than the size of the request" << std::endl;
        std::cout << "Buffer size: " << size << std::endl;
        raise;
    }
    std::memcpy(buffer, &header, sizeof(header));
    std::memcpy(buffer + sizeof(header),  this->payload.data(), header.payload_size);
}

size_t Request::size_in_bytes(){
    return header.payload_size + sizeof(header);
}

Response::Response(const uint8_t *response_bytes, size_t size) {
    std::memcpy(&header, response_bytes, sizeof(header));
    this->payload.assign(response_bytes + sizeof(header), response_bytes + sizeof(header) + header.payload_size);
}

Response::~Response() {
    std::cout << "DESTROYED" << std::endl;
}

uint8_t *Response::getPayload()
{
    return this->payload.data();
}

size_t Response::getPayloadSize() {
    return this->header.payload_size;
}

uint16_t Response::getResponseCode() {
    return header.response_code;
}

std::ostream &operator<<(std::ostream &os, const Request &request) {
    os << "Request - client_version=" << std::dec << (short)request.header.client_version << ", client_id=" << request.header.client_id << ", request_code=" << request.header.request_code << ", payload_size=" << std::dec << request.header.payload_size;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Response &response)
{
    os << "Response - server_version=" << std::dec << response.header.server_version << ", response_code=" << std::dec << response.header.response_code << ", payload_size=" << std::dec << response.header.payload_size;
    return os;
}

Message::Message(
    const uint8_t target_client_id[HEADER_CLIENT_ID_SIZE],
    const uint8_t message_type,
    const uint32_t content_size,
    const std::string &content) {
        std::memcpy(header.to_client_id, target_client_id, HEADER_CLIENT_ID_SIZE);
        header.message_type = message_type;
        header.content_size = content_size;
        this->message_content = content;
        
}

void Message::to_bytes(unsigned char *buffer, size_t size) {
    if (size < size_in_bytes()) {
        std::cout << "Buffer size is less than the size of the message" << std::endl;
        std::cout << "Buffer size: " << size << std::endl;
        raise;
    }
    std::memcpy(buffer, &header, sizeof(header));
    std::memcpy(buffer + sizeof(header), message_content.data(), header.content_size);
}

size_t Message::size_in_bytes() {
    return sizeof(header) + header.content_size;
}

std::ostream& operator<<(std::ostream& os, const Message& message) {
    os << "Messsage - type: " << message.header.message_type << " content_size: " << message.header.content_size;
    return os;
}
