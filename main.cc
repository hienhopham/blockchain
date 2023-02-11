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
#include "cloud-server.h"
#include "ipv4-address-helper-custom.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Blockchain");

int main(int argc, char *argv[])
{

	LogComponentEnable ("Blockchain", LOG_LEVEL_INFO);
	LogComponentEnable ("RsuNode", LOG_LEVEL_INFO);
	LogComponentEnable ("TopologyHelper", LOG_LEVEL_INFO);

	uint32_t numOfRsu = 2;
	const uint16_t blockchainPort = 8333;
	const uint16_t cloudServerId = 0;

	CommandLine cmd (__FILE__);
	cmd.AddValue ("numOfRsu", "Number of rsu nodes", numOfRsu);
	cmd.Parse (argc, argv);

	NS_LOG_INFO("\nNumber of Rsu nodes:" << numOfRsu);

	Ipv4InterfaceContainer  ipv4Interfacecontainer;
    std::map<uint32_t, std::vector<Ipv4Address>> nodeToPeerConnections;
	std::map<uint32_t, Ipv4Address> nodeToCloudServerConnectionsIp;

	//Initialize the topology
	TopologyHelper topologyHelper(numOfRsu, cloudServerId);

	//Install internet stack on every node then assign ips for them
	InternetStackHelper stack;
	topologyHelper.InstallStack(stack);
	topologyHelper.AssignIpv4Addresses(Ipv4AddressHelperCustom("1.0.0.0", "255.255.255.0", false));

	ipv4Interfacecontainer = topologyHelper.GetIpv4InterfaceContainer();
    nodeToPeerConnections = topologyHelper.GetNodeToPeerConnectionsIps();
	nodeToCloudServerConnectionsIp = topologyHelper.GetNodeToCloudServerConnectionsIp();


	NS_LOG_INFO("Start creating Rsu node and Cloud server");
	ApplicationContainer rsuNodes;
	ApplicationContainer cloudServerContainer;
	ObjectFactory factory;

    for(auto &node : nodeToPeerConnections)
    {
		NS_LOG_INFO("\nNode:" << node.first << "- Num of peer: " << node.second.size());
		Ptr<Node> targetNode = topologyHelper.GetNode(node.first);

		if (node.first != cloudServerId) {
			const std::string typeId = "ns3::RsuNode";
			factory.SetTypeId(typeId);
			factory.Set("Ip", AddressValue(InetSocketAddress(Ipv4Address::GetAny(), blockchainPort)));

			Ptr<RsuNode> rsuNode = factory.Create<RsuNode>();

			rsuNode->SetPeersAddresses(node.second);
			rsuNode->SetCloudServerAddress(nodeToCloudServerConnectionsIp[node.first]);
			targetNode->AddApplication(rsuNode);

			rsuNodes.Add(rsuNode);
		} else {
			const std::string typeId = "ns3::CloudServer";
			factory.SetTypeId(typeId);
			factory.Set("Ip", AddressValue(InetSocketAddress(Ipv4Address::GetAny(), blockchainPort)));

			Ptr<CloudServer> cloudServer = factory.Create<CloudServer>();

			cloudServer->SetPeersAddresses(node.second);
			targetNode->AddApplication(cloudServer);

			cloudServerContainer.Add(cloudServer);
		}
    }

    rsuNodes.Start(Seconds(0));
    rsuNodes.Stop(Minutes(1500));

	cloudServerContainer.Start(Seconds(0));
	cloudServerContainer.Stop(Minutes(1500));


	Simulator::Run ();
  	Simulator::Destroy ();
  	return 0;
}
