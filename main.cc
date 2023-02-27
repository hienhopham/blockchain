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
#include "blockchain.h"

using namespace ns3;

static double GetWallTime();
void PrintTotalStats(nodeStatistics *stats, uint32_t totalNodes, double tStart, double tFinish);	

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
	const double transThreshold = 2;
	double tStart = 0;
	double tFinish = 0;

	Time::SetResolution(Time::NS);

	CommandLine cmd (__FILE__);
	cmd.AddValue ("numOfRsu", "Number of rsu nodes", numOfRsu);
	cmd.AddValue ("transThreshold", "The threshold for the payments", numOfRsu);
	cmd.Parse (argc, argv);

	nodeStatistics *stats = new nodeStatistics[numOfRsu];

	NS_LOG_INFO("\nNumber of Rsu nodes:" << numOfRsu);


	uint32_t numOfRsuFromFile = 0;
	// get winner id for each rsu from file
	std::map<uint32_t, uint32_t> rsuMapWinner;
	std::ifstream infileWinner(winnersPath);

	// get payment for each winner from file
	std::map<uint32_t, double> rsuMapPayment;
	std::ifstream infilePayment(paymentsPath);


	if (!infileWinner || !infilePayment) {
		std::cerr << "Error: could not open one of the files: " << winnersPath << ", \n";
		std::cerr << paymentsPath << "\n";
	} else {
		std::cout << "Use input from files " << "\n";
	
		uint32_t winnerId = 1;
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
		
		uint32_t rsuId = 1;
		std::string linePayment;
		while (std::getline(infilePayment, linePayment)) {
			if (linePayment.empty()) {
				continue; // skip empty line
			}

			double payment = std::stod(linePayment);
			rsuMapPayment[rsuId] = payment;
			rsuId++;
		}
		
		infilePayment.close();


		numOfRsu = numOfRsuFromFile;
	}

	
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
			double payment = rsuMapPayment[node.first];
			const std::string typeId = "ns3::RsuNode";
			factory.SetTypeId(typeId);
			factory.Set("Ip", AddressValue(InetSocketAddress(Ipv4Address::GetAny(), blockchainPort)));
			factory.Set("WinnerId", UintegerValue(winnerId));
			factory.Set("Payment", DoubleValue(payment));
			factory.Set("TransThreshold", DoubleValue(transThreshold));

			Ptr<RsuNode> rsuNode = factory.Create<RsuNode>();

			rsuNode->SetPeersAddresses(node.second);
			rsuNode->SetNodeStats(stats);
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

    rsuNodes.Start(Seconds(0.1));
    rsuNodes.Stop(MilliSeconds(4000));

	cloudServerContainer.Start(Seconds(0.1));
	cloudServerContainer.Stop(MilliSeconds(4000));

	tStart = GetWallTime();

    Simulator::Stop(MilliSeconds(6000));
	Simulator::Run();
    Simulator::Destroy();

	tFinish = GetWallTime();
	PrintTotalStats(stats, numOfRsu, tStart, tFinish);

	delete[] stats;
  	return 0;

}


void PrintTotalStats(nodeStatistics *stats, uint32_t totalNodes, double tStart, double tFinish)
{
	double meanLatency = 0.0;

	for (uint32_t it = 0; it < totalNodes; it++ )
    { 

		if(stats[it].responseCount == stats[it].numberOfPeers) {
			meanLatency = (meanLatency*it + stats[it].meanLatency)/static_cast<double>(it+1);
		}
	}

	std::cout << std::endl;
	std::cout << "===============================================\n";
	std::cout << "Average Latency =" << meanLatency <<"s \n";
	std::cout << "Simulator Time =" << tFinish -  tStart<<"s \n";

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
