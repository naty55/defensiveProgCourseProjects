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
        char publicKey[HEADER_CLIENT_PUBLIC_KEY_SIZE] = {0};
        uint8_t symmetricKey[SYMMETRIC_KEY_SIZE] = { 0 };
        bool isPublicKeySet = false;
        bool isSymmetricKeySet = false;
        bool askedForSymmetricKey = false;
    } Peer;
private:
    std::string SERVER_HOST;
    std::string SERVER_PORT;
    std::string name;
    uint8_t _clientId[HEADER_CLIENT_ID_SIZE] = {0};
    char _clientPublicKey[HEADER_CLIENT_PUBLIC_KEY_SIZE] = {0};
    bool _is_registered = false;
    RSAPrivateWrapper rsapriv;
    std::unordered_map<std::string, Peer> peers;

    void read_server_info();
    void read_me_info();
    bool sendMessage(const Message& message);
    bool handleMessage(const RecievedMessageHeader* header, const uint8_t* payload, std::vector<ReceivedMessage>& messages);
    const std::string get_peer_by_client_id(const uint8_t[HEADER_CLIENT_ID_SIZE]) const;
    bool sendTextMessage(const std::string& message, const std::string& peer_name, bool isFile);

public:
    Client();
    Client(std::string &filename);
    void setClientId(const uint8_t* clientId);
    void setClientName(const std::string &clientName);
    const std::string get_client_name() const;
    void setRegistered(bool isRegistered);
    bool isRegistered() const;
    const uint8_t* getClientId() const;
    void save_me_info() const;
    const char* getPublicKeyOfSelf();
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
    bool requestPendingMessages(std::vector<ReceivedMessage>& messages);
    bool sendSymmetricKeyReqMessage(const std::string& peer_name);
    bool sendSymmetricKeyMessage(const std::string& peer_name);
    bool sendTextMessage(const std::string& message, const std::string& peer_name);
    bool sendFileMessage(const std::string& message, const std::string& peer_name);
    void assertPeerIsKnown(const std::string& peer_name);
};