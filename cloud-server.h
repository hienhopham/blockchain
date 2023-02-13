#include "rsu-node.h"

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
            // virtual void MineBlock(void);
        
    };
    

}
#endif