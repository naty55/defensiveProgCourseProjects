#ifndef CLIENT_CONFIG_HPP
#define CLIENT_CONFIG_HPP
#include <string>


// General
const int BUFFER_SIZE = 1024;
const std::string CLIENT_MESSAGE = "MeesageU Client at your service.\n\n110) Register \n120) Request for clients list \n130) Request for public key \n140) Request for waiting messages\n150) Send a text message \n151) Send a request for symmetric key \n152) Send your symmetric key \n0) Exit client";

// Server 
const char SERVER_PORT[] = "1357";
const char SERVER_HOST[] = "localhost";


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

