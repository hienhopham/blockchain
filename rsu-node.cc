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
#include "transaction.h"

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

    RsuNode::RsuNode()

    {
        NS_LOG_FUNCTION(this);
        m_listenSocket = 0;
        m_numberOfPeers = m_peersAddresses.size();
        m_transactionId = 1;
    }

    RsuNode::RsuNode(int nodeId)

    {
        NS_LOG_FUNCTION(this);
        m_nodeId = nodeId;
        m_listenSocket = 0;
        m_numberOfPeers = m_peersAddresses.size();
        m_transactionId = 1;
    }

    RsuNode::~RsuNode(void)
    {
        NS_LOG_FUNCTION (this);
    }

    int
    RsuNode::GetNodeId (void) const
    {
        NS_LOG_FUNCTION (this);
        return m_nodeId;
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
    RsuNode::StartApplication ()    // Called at time specified by Start
    {
        NS_LOG_FUNCTION (this);
        // Create the socket if not already

        srand(time(NULL) + GetNode()->GetId());
        NS_LOG_INFO ("Node " << GetNode()->GetId() << ": ip = " << m_nodeIp);
        NS_LOG_INFO ("Node " << GetNode()->GetId() << ": m_numberOfPeers = " << m_numberOfPeers);
        NS_LOG_INFO ("Node " << GetNode()->GetId() << ": My peers are");

        for (auto it = m_peersAddresses.begin(); it != m_peersAddresses.end(); it++)
            NS_LOG_INFO("\t" << *it);

        if (!m_listenSocket)
        {
            m_listenSocket = Socket::CreateSocket (GetNode(), UdpSocketFactory::GetTypeId ());
            m_listenSocket->Bind (m_nodeIp);
            m_listenSocket->Listen ();
            m_listenSocket->ShutdownSend ();
            if (addressUtils::IsMulticast (m_nodeIp))
            {
                Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_listenSocket);
                if (udpSocket)
                {
                    // equivalent to setsockopt (MCAST_JOIN_GROUP)
                    udpSocket->MulticastJoinGroup (0, m_nodeIp);
                }
                else
                {
                    NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
                }
            }
        }

        m_listenSocket->SetRecvCallback (MakeCallback (&RsuNode::HandleRead, this));
        m_listenSocket->SetAcceptCallback (
                MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                MakeCallback (&RsuNode::HandleAccept, this));
        m_listenSocket->SetCloseCallbacks (
                MakeCallback (&RsuNode::HandlePeerClose, this),
                MakeCallback (&RsuNode::HandlePeerError, this));

        NS_LOG_INFO("Node " << GetNode()->GetId() << ": Before creating sockets");
        for (std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i)
        {
            m_peersSockets[*i] = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
            m_peersSockets[*i]->Connect (InetSocketAddress (*i, 8333));
        }

        NS_LOG_INFO ("Node " << GetNode()->GetId() << ": After creating sockets");

        CreateTransaction(0,1);
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

    }

    void
    RsuNode::HandleRead(Ptr<Socket> socket)
    {
        NS_LOG_INFO("Recieve transaction");
        NS_LOG_INFO(this << socket);
        Ptr<Packet> packet;
        Address from;
        //std::cout<<"Node : "<< GetNode()->GetId()<< " Receive packet\n";

        //double newBlockReceiveTime = Simulator::Now().GetSeconds();

        while((packet = socket->RecvFrom(from)))
        {
            if(packet->GetSize() == 0)
            {
                break;
            }

            if(InetSocketAddress::IsMatchingType(from))
            {
                /*
                 * We may receive more than one packets simultaneously on the socket,
                 * so we have to parse each one of them.
                 */
                std::string delimiter = "#";
                std::string parsedPacket;
                char *packetInfo = new char[packet->GetSize() + 1];
                std::ostringstream totalStream;
                
                packet->CopyData(reinterpret_cast<uint8_t *>(packetInfo), packet->GetSize());
                packetInfo[packet->GetSize()] = '\0';

                totalStream << m_bufferedData[from] << packetInfo;
                std::string totalReceivedData(totalStream.str());
                NS_LOG_INFO("Node " << m_nodeId << "Total Received Data : " << totalReceivedData);
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

        // value = REQUEST_TRANS;
        // transD.AddMember("message", value, transD.GetAllocator());

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

        NS_LOG_INFO("Transaction: node-" <<newTrans.GetRsuNodeId()<<", paymment-"<<newTrans.GetPayment()<<", winner-"<<newTrans.GetWinnerId());

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


}