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
#include <cstdlib>
#include <fstream>
#include <iostream>
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
	LogComponentEnable ("CloudServer", LOG_LEVEL_INFO);

	uint32_t numOfRsu = 2;
	const uint16_t blockchainPort = 8333;
	const uint16_t cloudServerId = 0;
	const std::string currentPath = std::getenv("PWD");
	const std::string winnersPath = currentPath + "/scratch/blockchain/auction/winners.txt";
	const std::string paymentsPath = currentPath + "/scratch/blockchain/auction/payments.txt";

	CommandLine cmd (__FILE__);
	cmd.AddValue ("numOfRsu", "Number of rsu nodes", numOfRsu);
	cmd.Parse (argc, argv);

	NS_LOG_INFO("\nNumber of Rsu nodes:" << numOfRsu);


	uint32_t numOfRsuFromFile = 0;
	// get winner id for each rsu from file
	std::map<uint32_t, uint32_t> rsuMapWinner;
	uint32_t winnerId = 1;
	std::ifstream infileWinner(winnersPath);
	if (!infileWinner) {
		std::cerr << "Error: could not open file: " << winnersPath << std::endl;
		return 1;
	}
	
	std::string lineWinner;
	while (std::getline(infileWinner, lineWinner)) {
		if (lineWinner.empty()) {
			continue; // skip empty line
		}
		int rsuId = std::stoi(lineWinner);
		rsuMapWinner[rsuId] = winnerId;
		winnerId++;
		numOfRsuFromFile++;
	}
	
	infileWinner.close();


	// get payment for each winner from file
	std::map<uint32_t, double> winnerMapPayment;
	winnerId = 1;
	std::ifstream infilePayment(paymentsPath);
	if (!infilePayment) {
		std::cerr << "Error: could not open file: " << paymentsPath << std::endl;
		return 1;
	}
	
	std::string linePayment;
	while (std::getline(infilePayment, linePayment)) {
		if (linePayment.empty()) {
			continue; // skip empty line
		}

		double payment = std::stod(linePayment);
		winnerMapPayment[winnerId] = payment;
		winnerId++;
	}
	
	infilePayment.close();

	numOfRsu = numOfRsuFromFile;
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
		Ptr<Node> targetNode = topologyHelper.GetNode(node.first);

		if (node.first != cloudServerId) {
			uint32_t winnerId = rsuMapWinner[node.first];
			double payment = winnerMapPayment[winnerId];
			const std::string typeId = "ns3::RsuNode";
			factory.SetTypeId(typeId);
			factory.Set("Ip", AddressValue(InetSocketAddress(Ipv4Address::GetAny(), blockchainPort)));
			factory.Set("WinnerId", UintegerValue(winnerId));
			factory.Set("Payment", DoubleValue(payment));

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
    rsuNodes.Stop(MilliSeconds(2500));

	cloudServerContainer.Start(Seconds(0));
	cloudServerContainer.Stop(MilliSeconds(2500));


    Simulator::Stop(MilliSeconds(2500));
	Simulator::Run();
    Simulator::Destroy();


  	return 0;

}
