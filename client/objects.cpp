#include "objects.hpp"
#include "config.hpp"
#include <cstring>
#include <iostream>

void convert_value_to_bytes(long val, char* buffer, short buffer_size){
    for (size_t i = 0; i < buffer_size; i++) {
        buffer[i] = (val >> (i * 8)) & 0xFF; 
    }
}

unsigned long convert_bytes_to_value(char *buffer, short offset, short buffer_size) {
    unsigned long num = 0;
    for (size_t i = 0; i < buffer_size; i++) {
        unsigned char b = (buffer[offset + i]);
        num |= (b << (i * 8));
    }
    return num;
}

Request::Request(
    char clientId[HEADER_CLIENT_ID_SIZE],
    unsigned char client_version,
    unsigned short int request_code,
    unsigned long int payload_size,
    unsigned char *payload) {
        std::memcpy(this->clientId, clientId, HEADER_CLIENT_ID_SIZE);
        this->client_version = client_version;
        this->request_code = request_code;
        this->payload_size = payload_size;

        if (payload != nullptr) {
            std::cout << "Payload: " << payload << std::endl;
            this->payload.assign(payload, payload + payload_size);
        }
}

void Request::to_bytes(char * buffer, size_t size)
{   
    if (size < size_in_bytes()) {
        raise;
    }
    
    size_t offset = 0;
    std::memcpy(buffer, this->clientId, HEADER_CLIENT_ID_SIZE);

    offset += HEADER_CLIENT_ID_SIZE;
    
    char client_version[HEADER_CLIENT_VERSION_SIZE] = {this->client_version};
    std::memcpy(buffer + offset, client_version, HEADER_CLIENT_VERSION_SIZE);
    offset += HEADER_CLIENT_VERSION_SIZE;

    char request_code_[HEADER_REQUEST_CODE_SIZE] = {0};
    convert_value_to_bytes(this->request_code, request_code_, HEADER_REQUEST_CODE_SIZE);
    std::memcpy(buffer + offset,  request_code_, HEADER_REQUEST_CODE_SIZE);
    offset += HEADER_REQUEST_CODE_SIZE;

    char payload_size_[HEADER_PAYLOAD_SIZE_SIZE] = {0};
    convert_value_to_bytes(this->payload_size, payload_size_, HEADER_PAYLOAD_SIZE_SIZE);
    std::memcpy(buffer + offset, payload_size_, HEADER_PAYLOAD_SIZE_SIZE);
    offset += HEADER_PAYLOAD_SIZE_SIZE;

    std::memcpy(buffer + offset,  this->payload.data(), this->payload_size);
}

size_t Request::size_in_bytes(){
    return this->payload_size + HEADER_SIZE;
}

Response::Response(char *response_bytes, size_t size)
{
    size_t offset = 0;
    this->server_version = convert_bytes_to_value(response_bytes, offset, HEADER_SERVER_VERSION_SIZE);
    offset += HEADER_SERVER_VERSION_SIZE;
    this->response_code = convert_bytes_to_value(response_bytes, offset, HEADER_RESPONSE_CODE_SIZE);
    offset += HEADER_RESPONSE_CODE_SIZE;
    this->payload_size = convert_bytes_to_value(response_bytes, offset, HEADER_PAYLOAD_SIZE_SIZE);
    offset += HEADER_PAYLOAD_SIZE_SIZE;
    this->payload.assign(response_bytes + offset, response_bytes + offset + payload_size);
}

Response::~Response() {
    std::cout << "DESTROYED" << std::endl;
}

char* Response::getPayload()
{
    return this->payload.data();
}

size_t Response::getPayloadSize() {
    return this->payload_size;
}

std::ostream &operator<<(std::ostream &os, const Request &request) {
    os << "Request - client_version=" << std::dec << (short)request.client_version << ", client_id=" << request.clientId << ", request_code=" << request.request_code << ", payload_size=" << std::dec << request.payload_size;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Response &response)
{
    os << "Response - server_version=" << std::dec << response.server_version << ", response_code=" << std::dec << response.response_code << ", payload_size=" << std::dec << response.payload_size;
    return os;
}

Message::Message(
    char target_client_id[HEADER_CLIENT_ID_SIZE],
    unsigned char message_type,
    unsigned long int content_size,
    char message_content[]) {
        std::memcpy(this->target_client_id, target_client_id, HEADER_CLIENT_ID_SIZE);
        this->message_type = message_type;
        this->content_size = content_size;
        if (message_content != nullptr) {
            this->message_content.assign(message_content, message_content + content_size);
        }
}

void Message::to_bytes(unsigned char *buffer, size_t size) {
    size_t offset = 0;
    std::memcpy(buffer, target_client_id, HEADER_CLIENT_ID_SIZE);
    offset += HEADER_CLIENT_ID_SIZE;

    char message_type[1] = {this->message_type};
    std::memcpy(buffer + offset, message_type, 1);
    offset += 1;

    char content_size_[4] = {0};
    convert_value_to_bytes(this->content_size, content_size_, 4);
    std::memcpy(buffer + offset,  content_size_, 4);
    offset += 4;

    std::memcpy(buffer + offset, message_content.data(), this->content_size);
}

size_t Message::size_in_bytes() {
    return HEADER_CLIENT_ID_SIZE + 1 + 4 + this->content_size;
}

std::ostream& operator<<(std::ostream& os, const Message& message) {
    os << "Messsage - ";
    return os;
}
