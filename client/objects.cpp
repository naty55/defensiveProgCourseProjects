#include "objects.hpp"
#include "config.hpp"
#include <cstring>
#include <iostream>

void convert_value_to_bytes(long val, char* buffer, short buffer_size){
    for (size_t i = 0; i < buffer_size; ++i) {
        buffer[i] = (val >> (i * 8)) & 0xFF; 
    }
}

unsigned long convert_bytes_to_value(char *buffer, short offset, short buffer_size) {
    unsigned long num = 0;
    std::cout << "Starting to extract ";
    for (size_t i = 0; i < buffer_size; ++i) {
        unsigned char b = (buffer[offset + i]);
        num |= (b << (i * 8));
    }
    std::cout << "Num:" << std::dec << (num) << std::endl;
    return num;
}

Request::Request(
    unsigned char clientId[HEADER_CLIENT_ID_SIZE],
    unsigned char client_version,
    unsigned short int request_code,
    unsigned long int payload_size,
    char payload[]) {
        std::memcpy(this->clientId, clientId, HEADER_CLIENT_ID_SIZE);
        this->client_version = client_version;
        this->request_code = request_code;
        this->payload_size = payload_size;
        std::memcpy(this->payload, payload, payload_size);
        std::cout << "Payload: " << payload << std::endl;
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
    char request_code[HEADER_REQUEST_CODE_SIZE] = {0};
    convert_value_to_bytes(this->request_code, request_code, HEADER_REQUEST_CODE_SIZE);
    std::memcpy(buffer + offset,  request_code, HEADER_REQUEST_CODE_SIZE);
    offset += HEADER_REQUEST_CODE_SIZE;
    char payload_size[HEADER_PAYLOAD_SIZE_SIZE] = {0};
    convert_value_to_bytes(this->payload_size, payload_size, HEADER_PAYLOAD_SIZE_SIZE);
    std::memcpy(buffer + offset, payload_size, HEADER_PAYLOAD_SIZE_SIZE);
    offset += HEADER_PAYLOAD_SIZE_SIZE;
    std::memcpy(buffer + offset,  this->payload, this->payload_size);
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
    std::memcpy(payload, response_bytes + offset, size);
}

std::ostream &operator<<(std::ostream &os, const Response &response) {
    os << "Response - server_version=" << std::dec << response.server_version << ", response_code=" << std::dec << response.response_code << ", payload_size=" << std::dec << response.payload_size;
    return os;
}

