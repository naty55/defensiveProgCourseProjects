#pragma once
#include <cstdint>

const unsigned short HEADER_CLIENT_ID_SIZE = 16;
const unsigned short HEADER_CLIENT_NAME_SIZE = 255;
const unsigned short HEADER_CLIENT_PUBLIC_KEY_SIZE = 160;


enum RequestCode {
	REQ_REGISTRATION   = 600,
	REQ_CLIENTS_LIST   = 601,
	REQ_PUBLIC_KEY     = 602,
	REQ_SEND_MSG       = 603,
	REQ_PENDING_MSGS    = 604    
};

enum ResponseCode {
	RES_REGISTRATION  = 2100,
	RES_USERS         = 2101,
	RES_PUBLIC_KEY    = 2102,
	RES_MSG_SENT      = 2103,
	RES_PENDING_MSGS  = 2104,
	RES_SERVER_ERROR  = 9000    
};

enum MessageType {
	MSG_SYMMETRIC_KEY_REQUEST = 1,   
	MSG_SYMMETRIC_KEY_SEND    = 2,   
	MSG_TEXT                  = 3,   
	MSG_FILE                  = 4 
};

#pragma pack(push, 1)

typedef struct RequestHeader {
    uint8_t client_id[HEADER_CLIENT_ID_SIZE];
    uint8_t client_version;
    uint16_t request_code;
    uint32_t payload_size;
} RequestHeader;

typedef struct ResponseHeader {
    uint8_t server_version;
    uint16_t response_code;
    uint32_t payload_size;
} ResponseHeader;


typedef struct MessageHeader {
    uint8_t to_client_id[HEADER_CLIENT_ID_SIZE];
    uint8_t message_type;
    uint32_t content_size;
} MessageHeader;



#pragma pack(pop)