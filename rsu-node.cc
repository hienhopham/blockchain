#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "rsu-node.h"
// #include "transaction.h"
#include "blockchain.h"
#include <fstream>


namespace ns3 {


    NS_LOG_COMPONENT_DEFINE("RsuNode");
    NS_OBJECT_ENSURE_REGISTERED(RsuNode);

    TypeId
    RsuNode::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::RsuNode")
        .SetParent<Application>()
        .SetGroupName("Applications")
        .AddConstructor<RsuNode>()
        .AddAttribute("Ip",
                        "The Address on which to Bind the rx socket." ,
                        AddressValue(),
                        MakeAddressAccessor(&RsuNode::m_nodeIp),
                        MakeAddressChecker())
        ;
        return tid;
    }

    RsuNode::RsuNode(void) : m_blockchainPort(8333)

    {
        NS_LOG_FUNCTION(this);
        m_listenSocket = 0;
        m_cloudServerSocket = 0;
        m_numberOfPeers = m_peersAddresses.size();
        m_transactionId = 1;
    }

    RsuNode::~RsuNode(void)
    {
        NS_LOG_FUNCTION (this);
    }

    Address
    RsuNode::GetNodeIp (void) const
    {
        NS_LOG_FUNCTION (this);
        return m_nodeIp;
    }

    Ptr<Socket>
    RsuNode::GetListeningSocket (void) const
    {
        NS_LOG_FUNCTION (this);
        return m_listenSocket;
    }


    std::vector<Ipv4Address>
    RsuNode::GetPeersAddresses (void) const
    {
        NS_LOG_FUNCTION (this);
        return m_peersAddresses;
    }


    void
    RsuNode::SetPeersAddresses (const std::vector<Ipv4Address> &peers)
    {
        NS_LOG_FUNCTION (this);
        m_peersAddresses = peers;
        m_numberOfPeers = m_peersAddresses.size();
    }

    void
    RsuNode::SetCloudServerAddress (const Ipv4Address &cloudServerAddr)
    {
        NS_LOG_FUNCTION (this);
        m_cloudServerAddr = cloudServerAddr;
    }

    void
    RsuNode::StartApplication ()    // Called at time specified by Start
    {
        NS_LOG_FUNCTION (this);
        // Create the socket if not already

        srand(time(NULL) + GetNode()->GetId());
        // NS_LOG_INFO ("Node " << GetNode()->GetId() << ": ip = " << m_nodeIp);
        // NS_LOG_INFO ("Node " << GetNode()->GetId() << ": m_numberOfPeers = " << m_numberOfPeers);

        // Set up the listening socket, listen on every ip
        if (!m_listenSocket)
        {
            m_listenSocket = Socket::CreateSocket (GetNode(), TcpSocketFactory::GetTypeId ());
            m_listenSocket->Bind (m_nodeIp);
            m_listenSocket->Listen ();
            m_listenSocket->ShutdownSend ();
        }

        m_listenSocket->SetRecvCallback (MakeCallback (&RsuNode::HandleRead, this));
        m_listenSocket->SetAcceptCallback (
                MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                MakeCallback (&RsuNode::HandleAccept, this));
        m_listenSocket->SetCloseCallbacks (
                MakeCallback (&RsuNode::HandlePeerClose, this),
                MakeCallback (&RsuNode::HandlePeerError, this));

        // Set up the sending socket for every peer
        for (std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i)
        {
            m_peersSockets[*i] = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
            m_peersSockets[*i]->Connect (InetSocketAddress (*i, m_blockchainPort));
        }

        //Set up the sending socket for cloud server
        m_cloudServerSocket = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
        m_cloudServerSocket->Connect (InetSocketAddress (m_cloudServerAddr, m_blockchainPort));

        std::pair<PublicKey, long> keyPair = ECDSA::generateKey();
        publicKey = keyPair.first;
        privateKey = keyPair.second;

        std::cout << "===============================================\n";
        std::cout << "generating ECDSA key pair for current rsu node id " << GetNode()->GetId() << ":\n";
        publicKey.printKey();
        std::cout << "private key = " << privateKey << "\n";
        std::cout << "===============================================\n";

        CreateTransaction(0, GetNode()->GetId());
    }

    void
    RsuNode::StopApplication ()     // Called at time specified by Stop
    {
        NS_LOG_FUNCTION (this);

        for (std::vector<Ipv4Address>::iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i) //close the outgoing sockets
        {
            m_peersSockets[*i]->Close ();
        }

        if (m_listenSocket)
        {
            m_listenSocket->Close ();
            m_listenSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
        }

        if (m_cloudServerSocket)
        {
            m_cloudServerSocket->Close ();
            m_cloudServerSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
        }

    }

    void
    RsuNode::HandleRead(Ptr<Socket> socket)
    {
        Ptr<Packet> packet;
        Address from;
        //double newBlockReceiveTime = Simulator::Now().GetSeconds();

        while((packet = socket->RecvFrom(from)))
        {
            if(packet->GetSize() == 0)
            {
                break;
            }

            if(InetSocketAddress::IsMatchingType(from))
            {
                std::string delimiter = "#";
                std::string parsedPacket;
                char *packetInfo = new char[packet->GetSize() + 1];
                std::ostringstream totalStream;
                
                packet->CopyData(reinterpret_cast<uint8_t *>(packetInfo), packet->GetSize());
                packetInfo[packet->GetSize()] = '\0';

                totalStream << m_bufferedData[from] << packetInfo;
                std::string totalReceivedData(totalStream.str());
                size_t pos = totalReceivedData.find(delimiter);
                parsedPacket = totalReceivedData.substr(0,pos);

                rapidjson::Document d;
                d.Parse(parsedPacket.c_str());

                if(!d.IsObject())
                {
                    NS_LOG_WARN("The parsed packet is corrupted");
                    totalReceivedData.erase(0, pos + delimiter.length());
                    continue;
                }

                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                d.Accept(writer);


                switch(d["message"].GetInt())
                {
                    case REQUEST_TRANS:
                    {
                        // TODO: verify the transaction using smart contract - Tuan

                        // int trx_id = d["transactions"]["transId"].GetInt();
                        // int trx_rsu_id = d["transactions"]["rsuNodeId"].GetInt();
                        // int trx_winner_id = d["transactions"]["winnerId"].GetInt();
                        // double trx_timestamp = d["transactions"]["timestamp"].GetDouble();
                        // double trx_payment = d["transactions"]["payment"].GetDouble();

                        // // TODO: If valid, sign transaction - Tuan

                        // if (trx_payment >= 0 && trx_timestamp >= 0) {
                        //     // sign
                        //     // TODO
                        // }

                        // After signing, send response
                        d.AddMember("responseFrom", GetNode()->GetId(), d.GetAllocator());
                        SendMessage(REQUEST_TRANS, RESPONSE_TRANS, d, from);
                        std::cout<<"Node " << GetNode()->GetId() << " - REQUEST_TRANS \n";
                    }

                    case RESPONSE_TRANS:
                    {
    
                        uint32_t responseFrom = (uint32_t) d["responseFrom"].GetInt();
                        uint32_t requestTransFrom = (uint32_t) d["transactions"][0]["rsuNodeId"].GetInt();
                        if (requestTransFrom == GetNode()->GetId()) {
                             // TODO: Handle response, if get response valid from all peers then send the valid transaction to cloud sever - Tien
                            std::cout<<"Node " << GetNode()->GetId() << " receives - RESPONSE_TRANS from " << responseFrom << "\n";

                            SendMessage(RESPONSE_TRANS, REQUEST_BLOCK, d, m_cloudServerSocket);
                        }
                    }
                }
            }
        }
        
    }

    void
    RsuNode::HandleAccept(Ptr<Socket> socket, const Address& from)
    {
        NS_LOG_FUNCTION(this);
        socket->SetRecvCallback (MakeCallback(&RsuNode::HandleRead, this));
    }

    void
    RsuNode::HandlePeerClose(Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this);
    }

    void
    RsuNode::HandlePeerError(Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this);
    }

    void
    RsuNode::CreateTransaction(double payment, int winnerId)
    {
        NS_LOG_FUNCTION(this);

        rapidjson::Document transD;

        int transId = m_transactionId;
        double tranTimestamp = Simulator::Now().GetSeconds();
        transD.SetObject();

        Transaction newTrans(GetNode()->GetId(), transId, tranTimestamp, payment, winnerId);

        rapidjson::Value value;
        rapidjson::Value array(rapidjson::kArrayType);
        rapidjson::Value transInfo(rapidjson::kObjectType);

        value.SetString("transaction");
        transD.AddMember("type", value, transD.GetAllocator());

        value = REQUEST_TRANS;
        transD.AddMember("message", value, transD.GetAllocator());

        value = newTrans.GetRsuNodeId();
        transInfo.AddMember("rsuNodeId", value, transD.GetAllocator());

        value = newTrans.GetTransId();
        transInfo.AddMember("transId", value, transD.GetAllocator());

        value = newTrans.GetTransTimeStamp();
        transInfo.AddMember("timestamp", value, transD.GetAllocator());

        value = newTrans.GetPayment();
        transInfo.AddMember("payment", value, transD.GetAllocator());

        value = newTrans.GetWinnerId();
        transInfo.AddMember("winnerId", value, transD.GetAllocator());

        array.PushBack(transInfo, transD.GetAllocator());
        transD.AddMember("transactions", array, transD.GetAllocator());

        m_transaction.push_back(newTrans);
        //m_notValidatedTransaction.push_back(newTrans);

        rapidjson::StringBuffer transactionInfo;
        rapidjson::Writer<rapidjson::StringBuffer> tranWriter(transactionInfo);
        transD.Accept(tranWriter);

        for(std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i)
        {
            const uint8_t delimiter[] = "#";
            m_peersSockets[*i]->Send(reinterpret_cast<const uint8_t*>(transactionInfo.GetString()), transactionInfo.GetSize(), 0);
            m_peersSockets[*i]->Send(delimiter, 1, 0);
        
        }
        //std::cout<< "time : "<<Simulator::Now().GetSeconds() << ", Node type: "<< m_committerType <<" - NodeId: " <<GetNode()->GetId() << " created and sent transaction\n";
        m_transactionId++;

        // ScheduleNextTransaction();

    }

    void
    RsuNode::SendMessage(enum Messages receivedMessage, enum Messages responseMessage, rapidjson::Document &d, Ptr<Socket> outgoingSocket)
    {
        NS_LOG_FUNCTION(this);

        const uint8_t delimiter[] = "#";

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        d["message"].SetInt(responseMessage);
        d.Accept(writer);
        
        outgoingSocket->Send(reinterpret_cast<const uint8_t*>(buffer.GetString()), buffer.GetSize(), 0);
        outgoingSocket->Send(delimiter, 1, 0);

    }

    void
    RsuNode::SendMessage(enum Messages receivedMessage, enum Messages responseMessage, rapidjson::Document &d, Address &outgoingAddress)
    {
        NS_LOG_FUNCTION(this);
        
        const uint8_t delimiter[] = "#";

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        d["message"].SetInt(responseMessage);
        d.Accept(writer);
        
        Ipv4Address outgoingIpv4Address = InetSocketAddress::ConvertFrom(outgoingAddress).GetIpv4();
        std::map<Ipv4Address, Ptr<Socket>>::iterator it = m_peersSockets.find(outgoingIpv4Address);

        if(it == m_peersSockets.end())
        {
            m_peersSockets[outgoingIpv4Address] = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
            m_peersSockets[outgoingIpv4Address]->Connect(InetSocketAddress(outgoingIpv4Address, m_blockchainPort));
        }

        m_peersSockets[outgoingIpv4Address]->Send(reinterpret_cast<const uint8_t*>(buffer.GetString()), buffer.GetSize(), 0);
        m_peersSockets[outgoingIpv4Address]->Send(delimiter, 1, 0);
        
    }


}