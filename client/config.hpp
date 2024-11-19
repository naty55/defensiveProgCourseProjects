#ifndef CLIENT_CONFIG_HPP
#define CLIENT_CONFIG_HPP

// General
const int BUFFER_SIZE = 1024;

// Server 
const int SERVER_PORT = 1357;
// char SERVER_HOST[] = "localhost";


// Protocol
const unsigned short HEADER_CLIENT_ID_SIZE = 16;
const unsigned short HEADER_CLIENT_VERSION_SIZE = 1;
const unsigned short HEADER_SERVER_VERSION_SIZE = 1;
const unsigned short HEADER_REQUEST_CODE_SIZE = 2;
const unsigned short HEADER_RESPONSE_CODE_SIZE = 2;
const unsigned short HEADER_PAYLOAD_SIZE_SIZE = 4;
const unsigned short HEADER_SIZE = HEADER_CLIENT_ID_SIZE +
                                   HEADER_CLIENT_VERSION_SIZE + 
                                   HEADER_REQUEST_CODE_SIZE + 
                                   HEADER_PAYLOAD_SIZE_SIZE;




#endif 

