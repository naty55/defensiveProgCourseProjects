#pragma once
#include "config.hpp"
#include "protocol.hpp"
#include <unordered_map>
#include <string>

class Client {
    
    typedef struct Peer {
        uint8_t clientId[HEADER_CLIENT_ID_SIZE] = {0};
        uint8_t publicKey[HEADER_CLIENT_PUBLIC_KEY_SIZE] = {0};
        bool isPublicKeySet = false;
        bool isSymmetricKeySet = false;
    } Peer;

    private:
    std::string name;
    uint8_t _clientId[HEADER_CLIENT_ID_SIZE] = {0};
    uint8_t _clientPublicKey[HEADER_CLIENT_PUBLIC_KEY_SIZE] = {0};
    bool _isRegistered = false;
    std::unordered_map<std::string, Peer> peers;

    public:
    Client() = default;
    void setClientId(const uint8_t (&clientId)[HEADER_CLIENT_ID_SIZE]);
    void setClientName(std::string &clientName);
    void setRegistered(bool isRegistered);
    bool isRegistered();
    const uint8_t* getClientId();
    void clearKnownPeers ();
    void addPeer(const std::string &name, const uint8_t clientId[HEADER_CLIENT_ID_SIZE]);
    void printPeers();
    const uint8_t* getClientIdOf(std::string &peer_name);
    bool is_peer_known(std::string &peer_name);
    void setPublicKey(std::string &peer_name, const uint8_t public_key[HEADER_CLIENT_PUBLIC_KEY_SIZE]);
};