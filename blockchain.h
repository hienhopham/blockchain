#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <vector>
#include <map>
#include <algorithm>
#include "ns3/address.h"
#include "ipv4-address-helper-custom.h"

namespace ns3 {

    enum Messages
    {
        INV,            //0
        REQUEST_TRANS,  //1
        GET_HEADERS,    //2
        HEADERS,        //3
        GET_DATA,       //4
        BLOCK,          //5    
        NO_MESSAGE,     //6
        REPLY_TRANS,    //7
        MSG_TRANS,      //8
        RESULT_TRANS,   //9
    };

    enum MinerType
    {
        NORMAL_MINER,
        HYPERLEDGER_MINER,
    };

    enum CommitterType
    {
        COMMITTER,  //0
        ENDORSER,   //1
        CLIENT,     //2
        ORDER,      //3
    };
    
    enum ProtocolType
    {
        STANDARD_PROTOCOL,      //default
        SENDHEADERS
    };

    enum Cryptocurrency
    {
        ETHEREUM,
        HYPERLEDGER
    };

    enum BlockchainRegion
    {
        NORTH_AMERICA,
        EUROPE,
        SOUTH_AMERICA,
        KOREA,
        JAPAN,
        AUSTRALIA,
        OTHER
    };

    typedef struct{

        int     nodeId;                         // blockchain node ID
        double  meanBlockReceiveTime;        // average of Block receive time
        double  meanBlockPropagationTime;    // average of Block propagation time
        double  meanBlockSize;               // average of Block Size
        int     totalBlocks;
        int     miner;                          //0-> node, 1->miner
        int     minerGeneratedBlocks;
        double  minerAverageBlockGenInterval;
        double  minerAverageBlockSize;
        double  hashRate;
        long    invReceivedBytes;
        long    invSentBytes;
        long    getHeadersReceivedBytes;
        long    getHeadersSentBytes;
        long    headersReceivedBytes;
        long    headersSentBytes;
        long    getDataReceivedBytes;
        long    getDataSentBytes;
        long    blockReceivedBytes;
        long    blockSentBytes;
        int     longestFork;
        int     blocksInForks;
        int     connections;
        int     minedBlocksInMainChain;
        long    blockTimeouts;
        int     nodeGeneratedTransaction;
        double  meanEndorsementTime;
        double  meanOrderingTime;
        double  meanValidationTime;
        double  meanLatency;
        int     nodeType;
        double  meanNumberofTransactions;
      
    
    } nodeStatistics;

    typedef struct{
        double downloadSpeed;
        double uploadSpeed;
    } nodeInternetSpeed;

    const char* getMessageName(enum Messages m);
    const char* getMinerType(enum MinerType m);
    const char* getCommitterType(enum CommitterType m);
    const char* getProtocolType(enum ProtocolType m);
    const char* getCryptocurrency(enum Cryptocurrency m);
    const char* getBlockchainRegion(enum BlockchainRegion m);
    enum BlockchainRegion getBlockchainEnum(uint32_t n);

    class Transaction
    {
        public:

            Transaction(int nodeId, int transId, double timeStamp);
            Transaction();
            virtual ~Transaction(void);

            int GetTransNodeId(void) const;
            void SetTransNodeId(int nodeId);

            int GetTransId(void) const;
            void SetTransId(int transId);

            int GetTransSizeByte(void) const;
            void SetTransSizeByte(int transSizeByte);

            double GetTransTimeStamp(void) const;
            void SetTransTimeStamp(double timeStamp);

            bool IsValidated(void) const;
            void SetValidation();

            int GetExecution(void) const;
            void SetExecution(int endoerserId);

            Transaction& operator = (const Transaction &tranSource);     //Assignment Constructor

            friend bool operator == (const Transaction &tran1, const Transaction &tran2);
            
        
        protected:
            int m_nodeId;
            int m_transId;
            int m_transSizeByte;
            double m_timeStamp;
            bool m_validatation; 
            int m_execution;

    };





}

#endif