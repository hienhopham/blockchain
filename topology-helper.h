#ifndef TOPOLOGY_HELPER_H
#define TOPOLOGY_HELPER_H

#include "ns3/internet-stack-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/ipv6-interface-container.h"
#include "ns3/net-device-container.h"
#include <random>
#include <vector>
#include "ipv4-address-helper-custom.h"


namespace ns3 {
    class TopologyHelper {
        public: 


        TopologyHelper (uint32_t numberOfRsu, uint32_t m_cloudServerId);

        ~TopologyHelper ();

        Ptr<Node> GetNode (uint32_t id);

        void InstallStack (InternetStackHelper stack);
        void AssignIpv4Addresses (Ipv4AddressHelperCustom ip);
        Ipv4InterfaceContainer GetIpv4InterfaceContainer (void) const;
        std::map<uint32_t, std::vector<Ipv4Address>> GetNodeToPeerConnectionsIps (void) const;
        std::map<uint32_t, Ipv4Address> GetNodeToCloudServerConnectionsIp (void) const;


        uint32_t     m_numberOfRsu;                  //!< The total number of nodes
        uint32_t     m_cloudServerId;
        uint32_t     m_totalNoLinks; 
        std::vector<Ipv4InterfaceContainer>             m_interfaces;  
        std::map<uint32_t, std::vector<uint32_t>>       m_nodesConnections;        //!< key = nodeId
        std::map<uint32_t, std::vector<Ipv4Address>>    m_nodeToPeerConnectionsIps;     //!< key = nodeId
        std::map<uint32_t, Ipv4Address>   m_nodeToCloudServerConnectionsIp;     //!< key = nodeId
        std::vector<NodeContainer>                      m_nodes;                   //!< all the nodes in the network
        std::vector<NetDeviceContainer>                 m_devices;                 //!< NetDevices in the network

    };

}


#endif /* TOPOLOGY_HELPER_H */