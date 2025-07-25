#include "objects.hpp"
#include "config.hpp"
#include "exceptions.hpp"
#include <cstring>
#include <iostream>

Request::Request(
    const uint8_t *clientId,
    uint8_t client_version,
    RequestCode request_code,
    uint32_t payload_size,
    const uint8_t *payload) {
        std::memcpy(header.client_id, clientId, HEADER_CLIENT_ID_SIZE);
        header.client_version = client_version;
        header.request_code = static_cast<uint16_t>(request_code);
        header.payload_size = payload_size;
        if (payload != nullptr) {
            std::cout << "Payload: " << payload << std::endl;
            this->payload.assign(payload, payload + payload_size);
        }
}

void Request::to_bytes(std::vector<char> &buffer) const {   
    buffer.insert(buffer.begin(), reinterpret_cast<const char*>(&header), reinterpret_cast<const char*>(&header) + sizeof(header));
    buffer.insert(buffer.begin() + sizeof(header), payload.data(), payload.data() + payload.size());
}

size_t Request::size_in_bytes() const {
    return header.payload_size + sizeof(header);
}

RequestCode Request::getRequestCode() const {
    return RequestCode(header.request_code);
}

Response::Response(const uint8_t *response_bytes, size_t size) {
    std::memcpy(&header, response_bytes, sizeof(header));
    this->payload.assign(response_bytes + sizeof(header), response_bytes + sizeof(header) + header.payload_size);
}

const uint8_t *Response::getPayload() const {
    return this->payload.data();
}

size_t Response::getPayloadSize() const {
    return this->header.payload_size;
}

uint16_t Response::getResponseCode() const {
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
    const std::string &content) {
        std::memcpy(header.to_client_id, target_client_id, HEADER_CLIENT_ID_SIZE);
        header.message_type = message_type;
        header.content_size = static_cast<uint32_t>(content.size());
        this->message_content = content;
        if (this->message_content.size() != header.content_size) {
            throw stringable_client_exception("actual message content length != content length");
        }
}

void Message::to_bytes(std::vector<uint8_t> &buffer) const {
    buffer.insert(buffer.begin(), reinterpret_cast<const char*>(&header), reinterpret_cast<const char*>(&header) + sizeof(header));
    if (header.content_size > 0) {
        buffer.insert(buffer.begin() + sizeof(header), message_content.data(), message_content.data() + header.content_size);
    }
}

size_t Message::size_in_bytes() const {
    return sizeof(header) + header.content_size;
}

std::ostream& operator<<(std::ostream& os, const Message& message) {
    os << "Messsage - type: " << message.header.message_type << " content_size: " << message.header.content_size;
    return os;
}

const uint8_t* ReceivedMessage::getClientId() {
    return header.from_client_id;
}

ReceivedMessage::ReceivedMessage(const RecievedMessageHeader& header, const std::string from_client_name, const std::string message_content) {
    this->header = header;
    this->message_type = static_cast<MessageType>(header.message_type);
    this->from_client_name = from_client_name;
    this->message_display_string = message_content;
}

const std::string ReceivedMessage::getClientName() {
    return from_client_name;
}

const uint8_t* ReceivedMessage::getMessageId() {
    return header.message_id;
}

uint8_t ReceivedMessage::getMessageType() const {
	return header.message_type;
}

uint32_t ReceivedMessage::getContentSize() const {
    return header.content_size;
}

const std::string ReceivedMessage::getContent() {
    return message_display_string;
}

std::ostream& operator<<(std::ostream& os, const ReceivedMessage& message) {
	os << "From: " << message.from_client_name << "\nContent: \n" << message.message_display_string << "\n----<EOM>----\n";
    return os;
}
