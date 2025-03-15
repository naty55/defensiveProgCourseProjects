#include <boost/asio.hpp>
#include <iostream>
#include "config.hpp"
#include "objects.hpp"
#include "protocol.hpp"
#include "client.hpp"
#include <array>
#include <string>
#include "client_net.hpp"
#include "utils.hpp"


void client_loop();
bool handle_command(std::string& line, Client& client);
bool handle_message(ReceivedMessage& msg);

int main() {
    client_loop();
    std::cout << "See you later!" << std::endl;
    return 0;
}

void client_loop() {
    bool running = true;
    Client client;
    while (running) {
        std::cout << CLIENT_MESSAGE << std::endl;
        std::string line;
        std::getline(std::cin, line);
        std::cout << line << "\n";
        try {
            running = handle_command(line, client);
        }
        catch (const std::exception& e) {
            std::cout << "Caught a standard exception: " << e.what() << std::endl;
        }
        catch (...) {
            std::cout << "Caught an unknown exception." << std::endl;
        }
    }
}

bool handle_command(std::string& line, Client& client) {
    int input_num = 0;
    try {
		if (line.size() == 0) {
			return true;
		}
        input_num = std::stoi(line);
    }
    catch (...) {
        std::cout << "It's not you it's me - please select again" << std::endl;
        return true;
    }

    if (Commands::EXIT == input_num) {
        std::cout << "Exiting" << std::endl;
        return false;
    }

    if (!client.isRegistered() && input_num != Commands::REGISTER) {
        std::cout << "You need to register first" << std::endl;
        return true;
    }

    std::unique_ptr<Response> res;

    switch (input_num) {
    case Commands::REGISTER: {
		if (client.isRegistered()) {
			std::cout << "You are already registered\n";
			return true;
		}
        std::string client_name;
        while (client_name.size() + 1 > HEADER_CLIENT_NAME_SIZE || client_name.size() == 0) {
            std::cout << "Please enter your name: ";
            std::cin >> client_name;
        }
		client.registerClient(client_name);
        break;
    }
    case Commands::CLIENTS_LIST: {
        client.getPeers();
        client.printPeers();
        break;
    }
    case Commands::PUBLIC_KEY: {
        std::cout << "Whose public key do you want to request? ";
        std::string peer_name;
        std::getline(std::cin, peer_name);
        if (client.requestPublicKey(peer_name)) {
            std::cout << "Set public key for " << peer_name << "\n";
        }
        break;
    }
    case Commands::PENDING_MSGS: {
		std::vector<ReceivedMessage> messages;
		bool result = client.requestPendingMessages(messages);
		if (!result) {
			std::cout << "Failed to get messages\n";
			return true;
		}
		for (auto& msg : messages) {
			handle_message(msg);
		}
        break;
    }
    case Commands::SEND_MSG: {
        std::cout << "Who do you want to talk today? ";
        std::string peer_name;
        std::getline(std::cin, peer_name);
        std::cout << "Please write your message ";
        std::string str_message;
        std::getline(std::cin, str_message);
		client.sendTextMessage(str_message, peer_name);
        break;
    }
    case Commands::SYMMETRIC_KEY_REQUEST: {
        std::cout << "Who do you want to ask for a symmetric key? ";
        std::string peer_name;
        std::getline(std::cin, peer_name);
		client.sendSymmetricKeyReqMessage(peer_name);
        break;

    }
    case Commands::SYMMETRIC_KEY_SEND: {
        std::cout << "Who do you want to send Symmetric key to? ";
        std::string peer_name;
        std::getline(std::cin, peer_name);
		client.sendSymmetricKeyMessage(peer_name);
        break;
    }
    case Commands::FILE_MSG: {
        std::cout << "Who do you want to send Symmetric key to? ";
        std::string peer_name;
        std::getline(std::cin, peer_name);
        client.sendFileMessage("file content", peer_name);
        break;
        break;
    }
    default: {
        break;
    }
    }
    return true;
}

bool handle_message(ReceivedMessage& msg) {

    std::cout << "Handling message: " << msg << "\n";
    MessageType type = (MessageType)msg.getMessageType();
    switch (type) {
    case MessageType::MSG_TEXT: {
        std::cout << "Message from: " << msg.getFromClientId() << "\n";
        std::cout << "Message: " << msg.getContent() << "\n";
        break;
    }
	case MessageType::MSG_SYMMETRIC_KEY_REQUEST: {
		std::cout << "Symmetric key request from: " << msg.getFromClientId() << "\n";
		break;
    }
	case MessageType::MSG_SYMMETRIC_KEY_SEND: {
		std::cout << "Symmetric key send from: " << msg.getFromClientId() << "\n";
		break;
	} case MessageType::MSG_FILE: {
		std::cout << "File message from: " << msg.getFromClientId() << "\n";
		break;
	}
	default: {
		std::cout << "Unknown message type\n";
		break;
	}
	}
    std::cout << "----EOM----\n";
	return true;
}
