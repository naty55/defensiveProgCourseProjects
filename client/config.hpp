#pragma once
#include <string>


// General
const int BUFFER_SIZE = 4096;
const std::string CLIENT_MESSAGE = "MeesageU Client at your service.\n\n110) Register \n120) Request for clients list \n130) Request for public key \n140) Request for waiting messages\n150) Send a text message \n151) Send a request for symmetric key \n152) Send your symmetric key \n153) Send a file\n0) Exit client";

// Server 
const char SERVER_PORT[] = "1357";
const char SERVER_HOST[] = "localhost";

enum Commands {
	REGISTER = 110,
	CLIENTS_LIST = 120,
	PUBLIC_KEY = 130,
	PENDING_MSGS = 140,
	SEND_MSG = 150,
	SYMMETRIC_KEY_REQUEST = 151,
	SYMMETRIC_KEY_SEND = 152,
	FILE_MSG = 153,
	EXIT = 0
};

const uint8_t CLIENT_VERSION = 2;
