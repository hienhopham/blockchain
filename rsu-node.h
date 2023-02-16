#include <algorithm>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/boolean.h"
#include "../../rapidjson/document.h"
#include "../../rapidjson/writer.h"
#include "../../rapidjson/stringbuffer.h"
#include "common.h"
#include "blockchain.h"
#include "ecdsa.h"
#include "sha256.h"

#ifndef RSU_NODE_H
#define RSU_NODE_H

namespace ns3 {

class Address;
class Socket;
class Packet;

class RsuNode : public Application{
    public:

        static TypeId GetTypeId (void);
        RsuNode (void);
        RsuNode (int nodeId);

        virtual ~RsuNode (void);

        Address GetNodeIp(void) const;

        Ptr<Socket> GetListeningSocket (void) const;

        std::vector<Ipv4Address> GetPeersAddresses (void) const;

        void SetPeersAddresses (const std::vector<Ipv4Address> &peers);

        void SetCloudServerAddress (const Ipv4Address &cloudServerAddr);

        PublicKey publicKey;
        

    protected:

        virtual void StartApplication (void);    // Called at time specified by Start

        virtual void StopApplication (void); 

        virtual void HandleRead (Ptr<Socket> socket);

        void HandleAccept (Ptr<Socket> socket, const Address& from);

        void HandlePeerClose (Ptr<Socket> socket);

        void HandlePeerError (Ptr<Socket> socket);

        void CreateTransaction();

        /**
         * \brief Sends a message to a peer
         * \param receivedMessage the type of the received message
         * \param responseMessage the type of the response message
         * \param d the rapidjson document containing the info of the outgoing message
         * \param outgoingSocket the socket of the peer
         */
        void SendMessage(enum Messages receivedMessage, enum Messages responseMessage, rapidjson::Document &d, Ptr<Socket> outgoingSocket);
        
        /**
         * \brief Sends a message to a peer
         * \param receivedMessage the type of the received message
         * \param responseMessage the type of the response message
         * \param d the rapidjson document containing the info of the outgoing message
         * \param outgoingAddress the Address of the peer
         */
        void SendMessage(enum Messages receivedMessage, enum Messages responseMessage, rapidjson::Document &d, Address &outgoingAddress);

        Address m_nodeIp;
        Ptr<Node> m_node;
        Ptr<Socket> m_listenSocket;
        Ptr<Socket> m_cloudServerSocket;
        Ipv4Address m_cloudServerAddr;
        int m_numberOfPeers;
        int m_transactionId;
        uint32_t m_winnerId;
        double m_payment;

        std::vector<Ipv4Address> m_peersAddresses;
        std::map<Ipv4Address, Ptr<Socket>> m_peersSockets;
        std::map<Address, std::string> m_bufferedData;
        std::vector<Transaction> m_transaction;
        const int m_blockchainPort;

        long privateKey;
};

}

#endif /* RSU_NODE_H */