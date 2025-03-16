#include "client.hpp"
#include "utils.hpp"
#include <cstring>
#include <string>
#include <iostream>
#include <memory>
#include "client_net.hpp"

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

bool Client::isRegistered() {
    return _isRegistered;
}

const uint8_t* Client::getClientId() {
    return _clientId;
}

const uint8_t* Client::getPublicKeyOfSelf() {
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
    Request req(getClientId(), 1, RequestCode::REQ_REGISTRATION, (unsigned long int) (HEADER_CLIENT_NAME_SIZE + HEADER_CLIENT_PUBLIC_KEY_SIZE), payload);

    std::cout << "Request: " << req << std::endl;
    std::unique_ptr<Response> res = send_request(req);
    std::cout << "Response: " << *res << std::endl;

    if (res.get()->getResponseCode() != ResponseCode::RES_REGISTRATION) {
        std::cout << "Failed to register, please try again" << std::endl;
        return false;
    }
    uint8_t* res_payload = res.get()->getPayload();
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
    Request req(getClientId(), 1, RequestCode::REQ_CLIENTS_LIST, (unsigned long int) 0, nullptr);
    std::cout << "Request: " << req << std::endl;
    std::unique_ptr<Response> res = send_request(req);
    uint8_t* res_payload = res.get()->getPayload();
    size_t user_count = res.get()->getPayloadSize() / (16 + 255);
    if (res.get()->getPayloadSize() % (16 + 255)) {
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
    Request req(getClientId(), 1, RequestCode::REQ_PUBLIC_KEY, (unsigned long int) HEADER_CLIENT_ID_SIZE, target_client_id);
    std::unique_ptr<Response> res = send_request(req);
    uint8_t* payload = res.get()->getPayload();
    uint8_t client_id_from_res[HEADER_CLIENT_ID_SIZE];
    uint8_t client_public_key[HEADER_CLIENT_PUBLIC_KEY_SIZE];
    std::memcpy(client_id_from_res, payload, HEADER_CLIENT_ID_SIZE);
    std::memcpy(client_public_key, payload + HEADER_CLIENT_ID_SIZE, HEADER_CLIENT_PUBLIC_KEY_SIZE);
    setPublicKey(peer_name, client_public_key);
    return true;
}

bool Client::requestPendingMessages(std::vector<ReceivedMessage>& messages) {
    Request req(getClientId(), 1, RequestCode::REQ_PENDING_MSGS, (unsigned long int) 0, nullptr);
    std::unique_ptr<Response> res = send_request(req);
    uint8_t* res_payload = res.get()->getPayload();
	size_t payload_size = res.get()->getPayloadSize();

	if (payload_size == 0) {
		return true;
	}
    uint8_t* res_payload_end = res_payload + payload_size;
    while (res_payload < res_payload_end) {
        RecievedMessageHeader* ptr = reinterpret_cast<RecievedMessageHeader*>(res_payload);
        size_t size_of_next_message = ptr->content_size + sizeof(RecievedMessageHeader);
		if (res_payload + size_of_next_message > res_payload_end) {
			std::cout << "Wrong size of payload" << std::endl;
            return false;
		}
        std::vector<uint8_t> message_bytes = std::vector(res_payload, res_payload + size_of_next_message);
		messages.emplace_back(message_bytes.data(), size_of_next_message);
        res_payload += size_of_next_message;
    }
    return true;
}

bool Client::sendMessage(const Message& message) {
    uint8_t buffer[5000] = { 0 };
    message.to_bytes(buffer, message.size_in_bytes());
    Request req(getClientId(), 1, RequestCode::REQ_SEND_MSG, (unsigned long int) message.size_in_bytes(), buffer);
    std::unique_ptr<Response> res = send_request(req);
    uint8_t* res_payload = res.get()->getPayload();
    printBytes(res_payload, res.get()->getPayloadSize());
    return true;
}

bool Client::sendSymmetricKeyReqMessage(const std::string& peer_name) {
    if (!is_peer_known(peer_name)) {
        std::cout << "Client is not in list - please request all peers from server";
        return false;
    }
    const uint8_t* to_client_id = getClientIdOf(peer_name);
    Message message(to_client_id, MessageType::MSG_SYMMETRIC_KEY_REQUEST, "");
	return sendMessage(message);
}

bool Client::sendSymmetricKeyMessage(const std::string& peer_name) {
    if (!is_peer_known(peer_name)) {
        std::cout << "Client is not in list - please request all peers from server";
        return false;
    }
    const uint8_t* to_client_id = getClientIdOf(peer_name);
    const uint8_t symmetric_key[SYMMETRIC_KEY_SIZE] = {0};
	const std::string sk = std::string(reinterpret_cast<const char*>(symmetric_key), SYMMETRIC_KEY_SIZE);
    Message message(to_client_id, MessageType::MSG_SYMMETRIC_KEY_SEND, sk);
    return sendMessage(message);
}

bool Client::sendTextMessage(const std::string &message, const std::string &peer_name){
    if (!is_peer_known(peer_name)) {
        std::cout << "Client is not in list - please request all peers from server";
        return false;
    }
    const uint8_t* to_client_id = getClientIdOf(peer_name);
    Message message_obj(to_client_id, MessageType::MSG_TEXT, message);
	return sendMessage(message_obj);
}

bool Client::sendFileMessage(const std::string& file_content, const std::string& peer_name) {
	return sendTextMessage(file_content, peer_name);
}
