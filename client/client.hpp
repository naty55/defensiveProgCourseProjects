#pragma once
#include "config.hpp"
#include "RSAWrapper.hpp"
#include "objects.hpp"
#include "protocol.hpp"
#include <unordered_map>
#include <string>
#include <vector>

class Client {
    
    typedef struct Peer {
        uint8_t clientId[HEADER_CLIENT_ID_SIZE] = {0};
        uint8_t publicKey[HEADER_CLIENT_PUBLIC_KEY_SIZE] = {0};
        uint8_t symmetricKey[SYMMETRIC_KEY_SIZE] = { 0 };
        bool isPublicKeySet = false;
        bool isSymmetricKeySet = false;
    } Peer;

    private:
    std::string name;
    uint8_t _clientId[HEADER_CLIENT_ID_SIZE] = {0};
    uint8_t _clientPublicKey[HEADER_CLIENT_PUBLIC_KEY_SIZE] = {0};
    bool _isRegistered = false;
    RSAPrivateWrapper rsapriv;
    std::unordered_map<std::string, Peer> peers;

    bool sendMessage(const Message& message);

    public:
    Client();
    Client(std::string &filename);
    void setClientId(const uint8_t (&clientId)[HEADER_CLIENT_ID_SIZE]);
    void setClientName(const std::string &clientName);
    void setRegistered(bool isRegistered);
    bool isRegistered();
    const uint8_t* getClientId();
    const uint8_t* getPublicKeyOfSelf();
    void clearKnownPeers ();
    void addPeer(const std::string &name, const uint8_t clientId[HEADER_CLIENT_ID_SIZE]);
    void printPeers();
    const uint8_t* getClientIdOf(const std::string &peer_name);
    bool is_peer_known(const std::string &peer_name);
    void setPublicKey(const std::string &peer_name, const uint8_t public_key[HEADER_CLIENT_PUBLIC_KEY_SIZE]);
    void setSymmetricKey(const std::string& peer_name, const uint8_t symmetric_key[SYMMETRIC_KEY_SIZE]);
	bool registerClient(const std::string& client_name);
    bool getPeers();
	bool requestPublicKey(const std::string& peer_name);
    bool requestPendingMessages(std::vector<ReceivedMessage> messages);
    bool sendSymmetricKeyReqMessage(const std::string& peer_name);
    bool sendSymmetricKeyMessage(const std::string& peer_name);
    bool sendTextMessage(const std::string& message, const std::string& peer_name);
    bool sendFileMessage(const std::string& message, const std::string& peer_name);
};