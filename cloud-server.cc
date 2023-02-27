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
#include <random>
#include <time.h>
#include <sys/time.h>

static double GetWallTime();
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


    CloudServer::CloudServer(void): RsuNode(),  m_timeStart(0), m_fistToMine(false)

    {
        NS_LOG_FUNCTION(this);
        m_minerAverageBlockGenInterval = 0;
        m_previousBlockGenerationTime = 0;
        m_meanNumberofTransactions = 0;
        m_minerGeneratedBlocks = 0;
        // HARDCODE
        m_fixedBlockTimeGeneration = 15;

        std::random_device rd;
        m_generator.seed(rd());

        if(m_fixedBlockTimeGeneration > 0)
        {
            m_nextBlockTime = m_fixedBlockTimeGeneration;
        }
        else
        {
            m_nextBlockTime = 0;
        }

        if(m_fixedBlockSize > 0)
        {
            m_nextBlockSize = m_fixedBlockSize;
        }
        else
        {
            m_nextBlockSize = 0;
        }


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
        // ScheduleNextMiningEvent();
        
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


                std::cout << std::endl;
                switch(d["message"].GetInt())
                {
                    case REQUEST_BLOCK:
                    {
                        rapidjson::Value& trx = d["transactions"];
                        int rsuNodeId = trx["rsuNodeId"].GetInt();
                        int transId = trx["transId"].GetInt();

                        int responseFrom = d["responseFrom"].GetInt();
                        std::cout << "Node " << GetNode()->GetId() << " receives REQUEST_BLOCK from Node " << rsuNodeId << std::endl;
                        std::cout << parsedPacket << std::endl;
                        bool isSigned = trx["isSigned"].GetBool();
                        if (isSigned) {
                            std::cout << "Verifying signature for transaction id " << transId << " of Rsu Node id " 
                                                        << rsuNodeId << " requesting from Rsu Node id " << responseFrom << std::endl;
                            bool isValidSignature = ECDSA::verifySignature(trx["hashMsg"].GetInt64(),
                                                                           trx["publicKey"]["p"].GetInt64(),
                                                                           trx["publicKey"]["a"].GetInt64(),
                                                                           trx["publicKey"]["n"].GetInt64(),
                                                                           trx["publicKey"]["xG"].GetInt64(),
                                                                           trx["publicKey"]["yG"].GetInt64(),
                                                                           trx["publicKey"]["xQ"].GetInt64(),
                                                                           trx["publicKey"]["yQ"].GetInt64(),
                                                                           trx["signature"]["r"].GetInt64(),
                                                                           trx["signature"]["s"].GetInt64()
                                                                           );
                            if (isValidSignature) {
                                std::cout << "This transaction is verified by the cloud server.\n";
                                trx.AddMember("verified", true, d.GetAllocator());

                                int height = m_blockchain.GetCurrentTopBlock()->GetBlockHeight() + 1;
                                if(height == 1)
                                {
                                    m_fistToMine = true;
                                    m_timeStart = GetWallTime();
                                }

                                if(m_fixedBlockSize > 0)
                                {
                                    m_nextBlockSize = m_fixedBlockSize;
                                    
                                }
                                else
                                {
                                    std::normal_distribution<double> dist(23.0, 2.0);
                                    m_nextBlockSize = (int)(dist(m_generator)*1000);
                                }
                                Block newBlock(height, GetNode()->GetId(), 0, m_blockchain.GetCurrentTopBlock()->GetMinerId(), m_nextBlockSize,
                                                Simulator::Now().GetSeconds(), Simulator::Now().GetSeconds(), Ipv4Address("127.0.0.1"));
                                
                                /*
                                * Push transactions to new Blocks
                                */

                                std::vector<Transaction> addedTransaction;
                                Transaction newTrans; 
                                newTrans.SetTransId(transId);
                                newTrans.SetPayment(trx["payment"].GetDouble());
                                newTrans.SetRsuNodeId(rsuNodeId);
                                newTrans.SetWinnerId(trx["winnerId"].GetInt());
                                newTrans.SetTransTimeStamp(trx["timestamp"].GetDouble());

                                addedTransaction.push_back(newTrans);
                                
                                newBlock.SetTransactions(addedTransaction);
                                newBlock.PrintAllTransaction();
                                m_blockchain.AddBlock(newBlock);



                                rapidjson::Document blockD;
                                blockD.SetObject();

                                rapidjson::Value value;
                                rapidjson::Value array(rapidjson::kArrayType);
                                rapidjson::Value transInfo(rapidjson::kObjectType);

                                value.SetString("block");
                                blockD.AddMember("type", value, blockD.GetAllocator());

                                value = BROADCAST_BLOCK;
                                blockD.AddMember("message", value, blockD.GetAllocator());

                                value = height;
                                blockD.AddMember("blockHeight", value, blockD.GetAllocator());

                                value = newTrans.GetRsuNodeId();
                                transInfo.AddMember("rsuNodeId", value, blockD.GetAllocator());

                                value = newTrans.GetTransId();
                                transInfo.AddMember("transId", value, blockD.GetAllocator());

                                value.SetDouble(newTrans.GetTransTimeStamp());
                                transInfo.AddMember("timestamp", value, blockD.GetAllocator());

                                value = newTrans.GetPayment();
                                transInfo.AddMember("payment", value, blockD.GetAllocator());

                                value = newTrans.GetWinnerId();
                                transInfo.AddMember("winnerId", value, blockD.GetAllocator());

                                transInfo.AddMember("validation", (bool)true, blockD.GetAllocator());

                                array.PushBack(transInfo, blockD.GetAllocator());
                                blockD.AddMember("block", array, blockD.GetAllocator());

                                rapidjson::StringBuffer blockInfo;
                                rapidjson::Writer<rapidjson::StringBuffer> blockWriter(blockInfo);
                                blockD.Accept(blockWriter);
                                // send to peers 

                                for(std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i)
                                {

                                    SendMessage(REQUEST_BLOCK, BROADCAST_BLOCK, blockD, m_peersSockets[*i]);
                                    // const uint8_t delimiter[] = "#";
                                    // m_peersSockets[*i]->Send(reinterpret_cast<const uint8_t*>(blockInfo.GetString()), blockInfo.GetSize(), 0);
                                    // m_peersSockets[*i]->Send(delimiter, 1, 0);
                                
                                }
                                
                            } 
                            else {
                                std::cout << "This transaction is not verified by the cloud server.\n";
                                trx.AddMember("verified", false, d.GetAllocator());
                            }
                        }
                    }
            
                }
            }
        }
        
    }
}

static double GetWallTime()
{
    struct timeval time;
    if(gettimeofday(&time, NULL))
    {
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}