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
#include "transaction.h"
#include "cloud-server.h"

#include <fstream>

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("CloudServer");
    NS_OBJECT_ENSURE_REGISTERED(CloudServer);

    TypeId
    CloudServer::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::CloudServer")
        .SetParent<Application>()
        .SetGroupName("Applications")
        .AddConstructor<CloudServer>()
        .AddAttribute("Ip",
                        "The Address on which to Bind the rx socket." ,
                        AddressValue(),
                        MakeAddressAccessor(&CloudServer::m_nodeIp),
                        MakeAddressChecker())
        ;
        return tid;
    }


    CloudServer::CloudServer(void): RsuNode()

    {
        NS_LOG_FUNCTION(this);
    }

    CloudServer::~CloudServer(void)
    {
        NS_LOG_FUNCTION (this);
    }

    void
    CloudServer::StartApplication ()    // Called at time specified by Start
    {
        NS_LOG_FUNCTION (this);

        // Set up the listening socket, listen on every ip
        if (!m_listenSocket)
        {
            m_listenSocket = Socket::CreateSocket (GetNode(), TcpSocketFactory::GetTypeId ());
            m_listenSocket->Bind (m_nodeIp);
            m_listenSocket->Listen ();
            m_listenSocket->ShutdownSend ();
        }

        m_listenSocket->SetRecvCallback (MakeCallback (&CloudServer::HandleRead, this));
        m_listenSocket->SetAcceptCallback (
                MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                MakeCallback (&CloudServer::HandleAccept, this));
        m_listenSocket->SetCloseCallbacks (
                MakeCallback (&CloudServer::HandlePeerClose, this),
                MakeCallback (&CloudServer::HandlePeerError, this));

        // Set up the sending socket for every peer
        for (std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i)
        {
            m_peersSockets[*i] = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
            m_peersSockets[*i]->Connect (InetSocketAddress (*i, m_blockchainPort));
        }
        
    }

    void
    CloudServer::StopApplication ()     // Called at time specified by Stop
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
    CloudServer::HandleRead(Ptr<Socket> socket)
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
                    case REQUEST_BLOCK:
                    {
                        //TODO: verify signature, reorder here - Tuan
                        //TODO: create new block, broadcast to others here - Phuong
                        std::cout<<"Node " << GetNode()->GetId() << " receives REQUEST_BLOCK \n";
                    }
            
                }
            }
        }
        
    }
}
