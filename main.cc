#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/string.h"
#include "ns3/vector.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include <algorithm>
#include <fstream>
#include <time.h>
#include <sys/time.h>
#include "topology-helper.h"
#include "rsu-node.h"
#include "ipv4-address-helper-custom.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Blockchain");

int main(int argc, char *argv[])
{

	LogComponentEnable ("Blockchain", LOG_LEVEL_INFO);
	LogComponentEnable ("RsuNode", LOG_LEVEL_INFO);
	LogComponentEnable ("TopologyHelper", LOG_LEVEL_INFO);

	uint32_t numOfRsu = 2;
	uint32_t numOfIov = 2;

	const uint16_t blockchainPort = 8333;

	CommandLine cmd (__FILE__);
	cmd.AddValue ("numOfRsu", "Number of rsu nodes", numOfRsu);
	cmd.AddValue ("numOfIov", "Number of iov nodes", numOfIov);
	cmd.Parse (argc, argv);

	NS_LOG_INFO("\nNumber of Rsu nodes:" << numOfRsu);
	NS_LOG_INFO("\nNumber of Iov nodes:" << numOfIov);

	Ipv4InterfaceContainer  ipv4Interfacecontainer;
    std::map<uint32_t, std::vector<Ipv4Address>> nodesConnections;

	//Initialize the topology
	TopologyHelper topologyHelper(numOfRsu, numOfIov);

	//Install internet stack on every node then assign ips for them
	InternetStackHelper stack;
	topologyHelper.InstallStack(stack);
	topologyHelper.AssignIpv4Addresses(Ipv4AddressHelperCustom("1.0.0.0", "255.255.255.0", false));

	ipv4Interfacecontainer = topologyHelper.GetIpv4InterfaceContainer();
    nodesConnections = topologyHelper.GetNodesConnectionsIps();

	NS_LOG_INFO("Start creating Rsu node");
	ApplicationContainer rsuNodes;
	ObjectFactory factory;
	const std::string typeId = "ns3::RsuNode";
	factory.SetTypeId(typeId);
	factory.Set("Ip", AddressValue(InetSocketAddress(Ipv4Address::GetAny(), blockchainPort)));

    for(auto &node : nodesConnections)
    {
        Ptr<Node> targetNode = topologyHelper.GetNode(node.first);
		Ptr<RsuNode> rsuNode = factory.Create<RsuNode>();

		rsuNode->SetPeersAddresses(node.second);
		targetNode->AddApplication(rsuNode);

        rsuNodes.Add(rsuNode);
    }

    rsuNodes.Start(Seconds(0));
    rsuNodes.Stop(Minutes(1500));


	Simulator::Run ();
  	Simulator::Destroy ();
  	return 0;
}
