#include "client.hpp"
#include "utils.hpp"
#include <cstring>
#include <string>
#include <iostream>

Client::Client() {
    rsapriv.getPublicKey(_clientPublicKey, RSAPublicWrapper::KEYSIZE);
}

Client::Client(std::string &filename) {

}

void Client::setClientId(const uint8_t (&clientId)[HEADER_CLIENT_ID_SIZE]) {
    std::memcpy(_clientId, clientId, HEADER_CLIENT_ID_SIZE);
}

void Client::setClientName(std::string &clientName) {
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

bool Client::is_peer_known(std::string &peer_name) {
    return peers.find(peer_name) != peers.end();
}

const uint8_t* Client::getClientIdOf(std::string &peer_name) {
    return peers[peer_name].clientId;
}

void Client::setPublicKey(std::string &peer_name, const uint8_t public_key[HEADER_CLIENT_PUBLIC_KEY_SIZE]) {
    std::memcpy(peers[peer_name].publicKey, public_key, HEADER_CLIENT_PUBLIC_KEY_SIZE);
    peers[peer_name].isPublicKeySet = true;
}

