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
    void to_bytes(std::vector<char> &buffer) const;
    size_t size_in_bytes() const;
	RequestCode getRequestCode() const;
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
    ~Response() = default;
    const uint8_t *getPayload() const;
    size_t getPayloadSize() const ;
    unsigned short int getResponseCode() const;
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
        const std::string &content);
    void to_bytes(std::vector<uint8_t>& buffer) const;
    size_t size_in_bytes() const;        
    friend std::ostream& operator<<(std::ostream& os, const Message& message);
};


class ReceivedMessage {
private:
    RecievedMessageHeader header;
    MessageType message_type;
    std::string from_client_name;
	std::string message_display_string;
public:
    ReceivedMessage(const RecievedMessageHeader& header, const std::string from_client_name, const std::string message_content);
    const std::string getClientName();
    const uint8_t* getClientId();
	const uint8_t* getMessageId();
	uint8_t getMessageType() const ;
	uint32_t getContentSize() const;
	const std::string getContent();
    friend std::ostream& operator<<(std::ostream& os, const ReceivedMessage& message);
};