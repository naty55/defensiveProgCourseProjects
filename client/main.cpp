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
#include "exceptions.hpp"

bool handle_command(std::string& line, Client& client);
void client_loop(Client& client);

int main() {
    try {
        Client client;
        if (client.isRegistered()) {
            std::cout << "Hello " + client.get_client_name() << " Welcome back";
        }
        client_loop(client);
    } 
    catch (critical_client_exception& e) {
        std::cout << e;
        exit(-1);
    }
    catch (...) {
        std::cout << "An exception occurred\n";
        exit(-1);
    }
    std::cout << "See you later!" << std::endl;
    return 0;
}

static void client_loop(Client &client) {
    bool running = true;
    while (running) {
        std::cout << CLIENT_MESSAGE << std::endl;
        std::string line;
        try {
            while (line.size() == 0) {
                std::getline(std::cin, line);
            }
            running = handle_command(line, client);
            line.clear();
        }
        catch (const stringable_client_exception& e) {
            std::cout << e;
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}

bool handle_command(std::string& line, Client& client) {
    int input_num = 0;
    try {
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

    switch (input_num) {
    case Commands::REGISTER: {
		if (client.isRegistered()) {
			std::cout << "You are already registered\n";
			return true;
		}
        std::string client_name;
        while (client_name.size() + 1 > HEADER_CLIENT_NAME_SIZE || client_name.size() == 0) {
            std::cout << "Please enter your name (should be of 1-254 chars): ";
            std::getline(std::cin, client_name);
        }
		client.registerClient(client_name);
        client.save_me_info();
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
        std::cout << "Got " << messages.size() << " new messages\n";
        std::cout << "Messages: \n";
		for (auto& msg : messages) {
            std::cout << msg;
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
        std::string peer_name;
        std::cout << "Who do you want to share a file with? ";
        std::getline(std::cin, peer_name);
        std::cout << "Please enter a filename: ";
        std::string file_name;
        std::getline(std::cin, file_name);
        client.sendFileMessage(read_file(file_name), peer_name);
        break;
    }
    default: {
        throw stringable_client_exception("Not a valid command");
        break;
    }
    }
    return true;
}
