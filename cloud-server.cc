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
// #include "transaction.h"
#include "cloud-server.h"
#include "blockchain.h"
#include "rsu-node.h"
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
        
        std::pair<PublicKey, long> keyPair = ECDSA::generateKey();
        publicKey = keyPair.first;
        privateKey = keyPair.second;

        std::cout << "===============================================\n";
        std::cout << "generating ECDSA key pair for current cloud server node id " << GetNode()->GetId() << ":\n";
        publicKey.printKey();
        std::cout << "private key = " << privateKey << "\n";
        std::cout << "===============================================\n";

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
                        // TODO: Create new block
                        // Create Block: Input(List of ordered transaction), Output(Block)
                        // Need to define Block class
                        // TODO: Broadcast 
                        // Broadcast: Input(List of rsu nodes, block), Output(None)

                        uint32_t requestBlockFrom = (uint32_t) d["requestBlockFrom"].GetInt();

                        std::cout<<"Node " << GetNode()->GetId() << " receives REQUEST_BLOCK from: " << requestBlockFrom << "\n" << parsedPacket << "\n";
                    }
            
                }
            }
        }
        
    }
    // void
    // CloudServer::MineBlock(void)
    // {   
    //     NS_LOG_FUNCTION(this);
    //     //std::cout<< "Start MineBlock function\n";
    //     rapidjson::Document inv;
    //     rapidjson::Document block;

    //     std::vector<Transaction>::iterator      trans_it;
    //     int height = m_blockchain.GetCurrentTopBlock()->GetBlockHeight() + 1;
    //     int minerId = GetNode()->GetId();
    //     int nonce = 0;
    //     int parentBlockMinerId = m_blockchain.GetCurrentTopBlock()->GetMinerId();
    //     double currentTime = Simulator::Now().GetSeconds();
    //     std::ostringstream stringStream;
    //     std::string blockHash;

    //     stringStream << height << "/" << minerId;
    //     blockHash = stringStream.str();

    //     inv.SetObject();
    //     block.SetObject();

    //     if(height == 1)
    //     {
    //         m_fistToMine = true;
    //         m_timeStart = GetWallTime();
    //     }

    //     if(m_fixedBlockSize > 0)
    //     {
    //         m_nextBlockSize = m_fixedBlockSize;
            
    //     }
    //     else
    //     {
    //         std::normal_distribution<double> dist(23.0, 2.0);
    //         //m_nextBlockSize = dist(m_generator);
    //         m_nextBlockSize = (int)(dist(m_generator)*1000);
    //         //std::cout <<(int)(dist(m_generator)*1000) <<"\n";
    //     }

    //     if(m_nextBlockSize < m_averageTransacionSize)
    //     {
    //         m_nextBlockSize = m_averageTransacionSize + m_headersSizeBytes;
    //     }

    //     Block newBlock(height, minerId, nonce, parentBlockMinerId, m_nextBlockSize,
    //                     currentTime, currentTime, Ipv4Address("127.0.0.1"));
        
    //     /*
    //      * Push transactions to new Blocks
    //      */
        
    //     for(trans_it = m_notValidatedTransaction.begin(); trans_it < m_notValidatedTransaction.end(); trans_it++)
    //     {
    //         trans_it->SetValidation();
    //         m_totalOrdering++;
    //         m_meanOrderingTime = (m_meanOrderingTime*static_cast<double>(m_totalOrdering-1) + (Simulator::Now().GetSeconds() - trans_it->GetTransTimeStamp()))/static_cast<double>(m_totalOrdering);

    //     }
    //     //std::cout<<m_notValidatedTransaction.size()<<"\n";
    //     m_meanNumberofTransactions = (m_meanNumberofTransactions*static_cast<double>(m_minerGeneratedBlocks) + m_notValidatedTransaction.size())/static_cast<double>(m_minerGeneratedBlocks+1);
    //     newBlock.SetTransactions(m_notValidatedTransaction);
    //     m_notValidatedTransaction.clear();

    //     //newBlock.PrintAllTransaction();
        
    //     rapidjson::Value value;
    //     rapidjson::Value array(rapidjson::kArrayType);
    //     //rapidjson::Value blockInfor(rapidjson::kObjectType);

    //     value.SetString("block");
    //     inv.AddMember("type", value, inv.GetAllocator());

    //     if(m_protocolType == STANDARD_PROTOCOL)
    //     {
    //         value = INV;
    //         inv.AddMember("message", value, inv.GetAllocator());

    //         value.SetString(blockHash.c_str(), blockHash.size(), inv.GetAllocator());
    //         array.PushBack(value, inv.GetAllocator());
    //         inv.AddMember("inv", array, inv.GetAllocator());
    //     }

    //     m_meanBlockReceiveTime = (m_blockchain.GetTotalBlocks() - 1)/static_cast<double>(m_blockchain.GetTotalBlocks())*m_meanBlockReceiveTime
    //                             + (currentTime - m_previousBlockReceiveTime)/(m_blockchain.GetTotalBlocks());
    //     m_previousBlockReceiveTime = currentTime;

    //     m_meanBlockPropagationTime = (m_blockchain.GetTotalBlocks() - 1)/static_cast<double>(m_blockchain.GetTotalBlocks())*m_meanBlockPropagationTime;

    //     m_meanBlockSize = (m_blockchain.GetTotalBlocks() - 1)/static_cast<double>(m_blockchain.GetTotalBlocks())*m_meanBlockSize     
    //                         + (m_nextBlockSize)/static_cast<double>(m_blockchain.GetTotalBlocks());
        
    //     m_blockchain.AddBlock(newBlock);

    //     rapidjson::StringBuffer invInfo;
    //     rapidjson::Writer<rapidjson::StringBuffer> invWriter(invInfo);
    //     inv.Accept(invWriter);

    //     rapidjson::StringBuffer blockInfo;
    //     rapidjson::Writer<rapidjson::StringBuffer> blockWriter(blockInfo);
    //     block.Accept(blockWriter);

    //     //std::cout<< "MineBlock function : Add a new block in packet\n";

    //     for(std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i)
    //     {
            
    //         const uint8_t delimiter[] = "#";

    //         m_peersSockets[*i]->Send(reinterpret_cast<const uint8_t*>(invInfo.GetString()), invInfo.GetSize(), 0);
    //         m_peersSockets[*i]->Send(delimiter, 1, 0);
            
    //         m_nodeStats->invSentBytes += m_blockchainMessageHeader + m_countBytes + inv["inv"].Size()*m_inventorySizeBytes;
    //         //std::cout<< "Node : " << GetNode()->GetId() <<" complete minning and send packet to " << *i << " \n" ;
    //         NS_LOG_INFO("At time " << Simulator::Now().GetSeconds()
    //                     << " s blockchain miner " << GetNode()->GetId()
    //                     << " sent a packet " << invInfo.GetString()
    //                     << " to " << *i);
            

    //     }
        
    //     m_minerAverageBlockGenInterval = m_minerGeneratedBlocks/static_cast<double>(m_minerGeneratedBlocks+1)*m_minerAverageBlockGenInterval
    //                                     + (Simulator::Now().GetSeconds() - m_previousBlockGenerationTime)/(m_minerGeneratedBlocks+1);

    //     m_minerAverageBlockSize = m_minerGeneratedBlocks/static_cast<double>(m_minerGeneratedBlocks+1)*m_minerAverageBlockSize
    //                             + static_cast<double>(m_nextBlockSize)/(m_minerGeneratedBlocks+1);
    //     m_previousBlockGenerationTime = Simulator::Now().GetSeconds();
    //     m_minerGeneratedBlocks++;

    //     ScheduleNextMiningEvent();
    //     //std::cout<< "MineBlock function : finish MinBLock\n";

    // }
}
