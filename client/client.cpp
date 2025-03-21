#include "client.hpp"
#include "utils.hpp"
#include <cstring>
#include <string>
#include <iostream>
#include <memory>
#include "client_net.hpp"
#include "exceptions.hpp"
#include "AESWrapper.hpp"

Client::Client() {
    auto pk = rsapriv.getPublicKey();
    std::memcpy(_clientPublicKey, pk.c_str(), HEADER_CLIENT_PUBLIC_KEY_SIZE);
}

Client::Client(std::string &filename) {
}

void Client::setClientId(const uint8_t (&clientId)[HEADER_CLIENT_ID_SIZE]) {
    std::memcpy(_clientId, clientId, HEADER_CLIENT_ID_SIZE);
}

void Client::setClientName(const std::string &clientName) {
    name = clientName;
}

void Client::setRegistered(bool isRegistered) {
    _isRegistered = isRegistered;
}

bool Client::isRegistered() const {
    return _isRegistered;
}

const uint8_t* Client::getClientId() {
    return _clientId;
}

const char* Client::getPublicKeyOfSelf() {
    return _clientPublicKey;
}

void Client::clearKnownPeers () {
    peers.clear();
}

void Client::addPeer(const std::string &name, const uint8_t clientId[HEADER_CLIENT_ID_SIZE]) {
    Peer peer;
    std::memcpy(peer.clientId, clientId, HEADER_CLIENT_ID_SIZE);
    peers[name] = peer;

}

void Client::printPeers() {
    std::cout << "Users count: " << peers.size() << "\n"; 
    std::cout << "Users:\n";
    for (auto const& [name, peer] : peers) {
        std::cout << "Name: " << name << " ClientId: ";
        printBytes(peer.clientId, HEADER_CLIENT_ID_SIZE);
    }
}

bool Client::is_peer_known(const std::string &peer_name) {
    return peers.find(peer_name) != peers.end();
}

const uint8_t* Client::getClientIdOf(const std::string &peer_name) {
    return peers[peer_name].clientId;
}

void Client::setPublicKey(const std::string &peer_name, const uint8_t public_key[HEADER_CLIENT_PUBLIC_KEY_SIZE]) {
    std::memcpy(peers[peer_name].publicKey, public_key, HEADER_CLIENT_PUBLIC_KEY_SIZE);
    peers[peer_name].isPublicKeySet = true;
}

void Client::setSymmetricKey(const std::string& peer_name, const uint8_t symmetric_key[SYMMETRIC_KEY_SIZE]) {
    std::memcpy(peers[peer_name].symmetricKey, symmetric_key, SYMMETRIC_KEY_SIZE);
    peers[peer_name].isSymmetricKeySet = true;
}

bool Client::registerClient(const std::string &client_name) {
    unsigned char payload[HEADER_CLIENT_NAME_SIZE + HEADER_CLIENT_PUBLIC_KEY_SIZE] = { 0 };
    std::memcpy(payload, client_name.c_str(), client_name.size());
    std::memcpy(payload + HEADER_CLIENT_NAME_SIZE, getPublicKeyOfSelf(), HEADER_CLIENT_PUBLIC_KEY_SIZE);
    Request req(getClientId(), CLIENT_VERSION, RequestCode::REQ_REGISTRATION, (unsigned long int) (HEADER_CLIENT_NAME_SIZE + HEADER_CLIENT_PUBLIC_KEY_SIZE), payload);
    std::unique_ptr<Response> res;
    try {
        res = send_request_and_get_response(req, ResponseCode::RES_REGISTRATION);
    } catch (stringable_client_exception& e) {
        std::cout << e;
        return false;
    }

    const uint8_t* res_payload = res.get()->getPayload();
    uint8_t client_id[HEADER_CLIENT_ID_SIZE] = { 0 };
    std::memcpy(client_id, res_payload, HEADER_CLIENT_ID_SIZE);
    setClientId(client_id);
    setClientName(client_name);
    setRegistered(true);
    std::cout << "Registered! client_id=";
    printBytes(client_id, HEADER_CLIENT_ID_SIZE);
    std::cout << "client_name=" << client_name << std::endl;
    return true;
}

bool Client::getPeers() {
    Request req(getClientId(), CLIENT_VERSION, RequestCode::REQ_CLIENTS_LIST, (unsigned long int) 0, nullptr);
    std::unique_ptr<Response> res;
    try {
        res = send_request_and_get_response(req, ResponseCode::RES_USERS);
    }
    catch (stringable_client_exception& e) {
        std::cout << e;
        return false;
    }
    const uint8_t* res_payload = res.get()->getPayload();
    size_t peer_size = static_cast<size_t>(HEADER_CLIENT_ID_SIZE) + HEADER_CLIENT_NAME_SIZE;
    size_t user_count = res.get()->getPayloadSize() / peer_size;

    if (res.get()->getPayloadSize() % peer_size) {
        std::cout << "Wrong size of payload" << std::endl;
        return false;
    }

    for (int i = 0; i < user_count; i++) {
        char user_name[HEADER_CLIENT_NAME_SIZE] = { 0 };
        uint8_t user_id[HEADER_CLIENT_ID_SIZE] = { 0 };
        std::memcpy(user_id, res_payload + i * (16 + 255), 16);
        std::memcpy(user_name, res_payload + i * (16 + 255) + 16, 255);
        std::string peer_name = std::string(user_name);
		if (!is_peer_known(peer_name)) {
            addPeer(peer_name, user_id);
		}
    }
    return true;
}

bool Client::requestPublicKey(const std::string& peer_name) {
	if (!is_peer_known(peer_name)) {
		std::cout << "Peer is not in list\n";
		return false;
	}
    const uint8_t* target_client_id = getClientIdOf(peer_name);
    Request req(getClientId(), CLIENT_VERSION, RequestCode::REQ_PUBLIC_KEY, (unsigned long int) HEADER_CLIENT_ID_SIZE, target_client_id);
    std::unique_ptr<Response> res;
    try {
        res = send_request_and_get_response(req, ResponseCode::RES_PUBLIC_KEY);
    }
    catch (stringable_client_exception& e) {
        std::cout << e;
        return false;
    }
    const uint8_t* payload = res.get()->getPayload();
    uint8_t client_id_from_res[HEADER_CLIENT_ID_SIZE];
    uint8_t client_public_key[HEADER_CLIENT_PUBLIC_KEY_SIZE];
    std::memcpy(client_id_from_res, payload, HEADER_CLIENT_ID_SIZE);
    std::memcpy(client_public_key, payload + HEADER_CLIENT_ID_SIZE, HEADER_CLIENT_PUBLIC_KEY_SIZE);
    setPublicKey(peer_name, client_public_key);
    return true;
}

bool Client::requestPendingMessages(std::vector<ReceivedMessage>& messages) {
    Request req(getClientId(), CLIENT_VERSION, RequestCode::REQ_PENDING_MSGS, (unsigned long int) 0, nullptr);
    std::unique_ptr<Response> res;
    try {
        res = send_request_and_get_response(req, ResponseCode::RES_PENDING_MSGS);
    }
    catch (stringable_client_exception& e) {
        std::cout << e;
        return false;
    }
    const uint8_t* res_payload = res.get()->getPayload();
	size_t payload_size = res.get()->getPayloadSize();

	if (payload_size == 0) {
		return true;
	}
    const uint8_t* res_payload_end = res_payload + payload_size;
    while (res_payload < res_payload_end) {
        const RecievedMessageHeader* ptr = reinterpret_cast<const RecievedMessageHeader*>(res_payload);
        size_t size_of_next_message = ptr->content_size + sizeof(RecievedMessageHeader);
		if (res_payload + size_of_next_message > res_payload_end) {
			std::cout << "Wrong size of payload" << std::endl;
            return false;
		}
		handleMessage(ptr, res_payload + sizeof(RecievedMessageHeader), messages);
        res_payload += size_of_next_message;
    }
    return true;
}

bool Client::handleMessage(const RecievedMessageHeader *header, const uint8_t *payload, std::vector<ReceivedMessage>& messages) {
	size_t content_size = header->content_size;
    MessageType message_type = static_cast<MessageType>(header->message_type);
    std::string display_string = "";
    std::string peer_name = get_peer_by_client_id(header->from_client_id);
    if (MessageType::MSG_SYMMETRIC_KEY_REQUEST != message_type && !is_peer_known(peer_name) ) {
        std::cout << "Got message from unkonwn peer - dropping message\n";
        return false;
    }
    switch (message_type) {
    case MessageType::MSG_SYMMETRIC_KEY_REQUEST: {
            display_string = "Request for symmetric key";
            break;
        }
    case MessageType::MSG_SYMMETRIC_KEY_SEND: {
        display_string = "symmetric key received";
        if (peers[peer_name].askedForSymmetricKey) {
            std::string sk = rsapriv.decrypt(reinterpret_cast<const char*>(payload), static_cast<unsigned int>(content_size));
            setSymmetricKey(peer_name, reinterpret_cast<const uint8_t*>(sk.c_str()));
            std::cout << "New symmetric key: ";
			printBytes(reinterpret_cast<const uint8_t*>(sk.c_str()), SYMMETRIC_KEY_SIZE);
        }
        else {
			std::cout << "Received symmetric key from " << peer_name << " without asking for it - message is droped\n";
        }
        break;
        }
    case MessageType::MSG_FILE:
    case MessageType::MSG_TEXT: {
        if (!peers[peer_name].isSymmetricKeySet) {
			display_string = "can't decrypt message - symmetric key not found";
			return false;
        }
        else {
            AESWrapper aes(peers[peer_name].symmetricKey, SYMMETRIC_KEY_SIZE);
            display_string = aes.decrypt(reinterpret_cast<const char*>(payload), static_cast<unsigned int>(content_size));
            if (message_type == MessageType::MSG_FILE) {
                try {
					std::string relative_path = get_timestamp() + "_" + bytes_to_hex_string(header->message_id, HEADER_MESSAGE_ID_SIZE);
                    std::string file_path = save_file_in_temp_directory(display_string, relative_path);
                    display_string = "File saved to: " + file_path;
                }
                catch (const stringable_client_exception& e) {
                    std::cout << e;
                    return false;
                }
				
            }
        }
        
    }

    }
    RecievedMessageHeader _header;
	std::memcpy(&_header, header, sizeof(RecievedMessageHeader));
	messages.emplace_back(_header, peer_name, display_string);
	return true;
}

const std::string Client::get_peer_by_client_id(const uint8_t client_id[HEADER_CLIENT_ID_SIZE]) const {
    for (const auto& [key, value] : peers) {
        bool is_equal = true;
        int i = 0;
        for (; i < HEADER_CLIENT_ID_SIZE;i++) {
			if (value.clientId[i] != client_id[i]) {
				is_equal = false;
				break;
			}
		}
		if (is_equal) {
			return key;
		}
    }
    return "";
}

bool Client::sendMessage(const Message& message) {
    uint8_t buffer[5000] = { 0 };
    message.to_bytes(buffer, message.size_in_bytes());
    Request req(getClientId(), CLIENT_VERSION, RequestCode::REQ_SEND_MSG, (unsigned long int) message.size_in_bytes(), buffer);
    std::unique_ptr<Response> res;
    try {
        res = send_request_and_get_response(req, ResponseCode::RES_MSG_SENT);
    }
    catch (stringable_client_exception& e) {
        std::cout << e;
        return false;
    }
    const uint8_t* res_payload = res.get()->getPayload();
    printBytes(res_payload, res.get()->getPayloadSize());
    return true;
}

bool Client::sendSymmetricKeyReqMessage(const std::string& peer_name) {
    if (!is_peer_known(peer_name)) {
        std::cout << "Client is not in list - please request all peers from server";
        return false;
    }
	peers[peer_name].askedForSymmetricKey = true;
    const uint8_t* to_client_id = getClientIdOf(peer_name);
    Message message(to_client_id, MessageType::MSG_SYMMETRIC_KEY_REQUEST, "");
	return sendMessage(message);
}

bool Client::sendSymmetricKeyMessage(const std::string& peer_name) {
    if (!is_peer_known(peer_name)) {
        std::cout << "Client is not in list - please request all peers from server";
        return false;
    }
	const Peer peer = peers[peer_name];
	if (!peer.isPublicKeySet) {
		std::cout << "Public key for " <<  peer_name << " is not set - please request public key from server";
		return false;
	}
    const uint8_t* to_client_id = getClientIdOf(peer_name);
	AESWrapper aes;
	setSymmetricKey(peer_name, aes.getKey());
	std::cout << "Set Symmetric key for " << peer_name << std::endl << "Symmetric key:";
	printBytes(peers[peer_name].symmetricKey, SYMMETRIC_KEY_SIZE);
    std::cout << "\n";
    RSAPublicWrapper rsaEncryptor(peer.publicKey, HEADER_CLIENT_PUBLIC_KEY_SIZE);
    std::string encrypted_sk = rsaEncryptor.encrypt(reinterpret_cast<const char*>(peers[peer_name].symmetricKey), SYMMETRIC_KEY_SIZE);
    Message message(to_client_id, MessageType::MSG_SYMMETRIC_KEY_SEND, encrypted_sk);
    return sendMessage(message);
}

bool Client::sendTextMessage(const std::string& message, const std::string& peer_name) {
    return sendTextMessage(message, peer_name, false);
}

bool Client::sendTextMessage(const std::string &message, const std::string &peer_name, bool isFile){
    if (!is_peer_known(peer_name)) {
        std::cout << "Client is not in list - please request all peers from server\n";
        return false;
    }
	const Peer peer = peers[peer_name];
    if (!peer.isSymmetricKeySet) {
		std::cout << "Symmetric key is not set for " << peer_name << " - please request symmetric key\n";
        return false;
    }
    const uint8_t* to_client_id = getClientIdOf(peer_name);
	AESWrapper aes(peer.symmetricKey, SYMMETRIC_KEY_SIZE);
	std::string encrypted_message = aes.encrypt(message);
    MessageType msg_type = isFile ? MessageType::MSG_FILE : MessageType::MSG_TEXT;
    Message message_obj(to_client_id, msg_type, encrypted_message);
	return sendMessage(message_obj);
}

bool Client::sendFileMessage(const std::string& file_content, const std::string& peer_name) {
	return sendTextMessage(file_content, peer_name, true);
}
