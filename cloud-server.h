#include "rsu-node.h"
#include <random>
#ifndef CLOUD_SERVER_H
#define CLOUD_SERVER_H

namespace ns3{

    class Address;
    class Socket;
    class Packet;

    class CloudServer : public RsuNode
    {

        public:

            static TypeId GetTypeId(void);

            CloudServer();

            ~CloudServer(void);


        protected:

            virtual void StartApplication(void);
            virtual void StopApplication(void);
            virtual void HandleRead (Ptr<Socket> socket);
            void ScheduleNextMiningEvent (void);
            virtual void MineBlock(void);


            uint32_t m_fixedBlockSize;
            int m_nextBlockSize;
            std::default_random_engine  m_generator;
            bool    m_fistToMine;
            double  m_timeStart;
            double  m_meanNumberofTransactions;
            double  m_fixedBlockTimeGeneration;
            double  m_nextBlockTime;
            int m_minerGeneratedBlocks;
            double  m_minerAverageBlockGenInterval;
            double  m_previousBlockGenerationTime;
            double  m_minerAverageBlockSize;
            EventId m_nextMiningEvent;
        
    };
    

}
#endif