#include "ns3/internet-stack-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/string.h"
#include "ns3/vector.h"
#include "ns3/log.h"
#include "ns3/ipv6-address-generator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/double.h"
#include <algorithm>
#include <fstream>
#include <time.h>
#include <sys/time.h>
#include "topology-helper.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TopologyHelper");

TopologyHelper::TopologyHelper (uint32_t numberOfRsu, uint32_t numberOfIov): m_numberOfRsu(numberOfRsu), m_numberOfIov (numberOfIov), m_totalNoLinks (0)
	{
	
		std::vector<uint32_t> rsuNodes;    //nodes contain the ids of the nodes

		for (uint32_t i = 0; i < m_numberOfRsu; i++)
		{
			rsuNodes.push_back(i);
		}

		for(uint32_t i = 0; i < m_numberOfRsu; i++)
		{
			for(uint32_t j = 0; j < m_numberOfRsu; j++)
			{
				if (j != i)
				{
					// m_nodesConnections[j].push_back(i);
					m_nodesConnections[i].push_back(j);
				}
			}
			NS_LOG_INFO("\nThe total number of connections for rsu "<< i << " is: " << m_nodesConnections[i].size());

		}

		PointToPointHelper pointToPoint;
		pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  		pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

		for (uint32_t i = 0; i < m_numberOfRsu; i++)
		{
			NodeContainer currentNode;
			currentNode.Create(1);
			m_nodes.push_back (currentNode);
		}

		for(auto &node : m_nodesConnections)  
		{
			for(std::vector<uint32_t>::const_iterator it = node.second.begin(); it != node.second.end(); it++)
    		{
				m_totalNoLinks++;
				NetDeviceContainer newDevices;
				newDevices.Add (pointToPoint.Install (m_nodes.at(node.first).Get(0),m_nodes.at (*it).Get (0)));
		    	m_devices.push_back (newDevices);
				NS_LOG_INFO("\nDevice " << m_totalNoLinks << " has: " << newDevices.Get(0) << " and " << newDevices.Get(1));
			}
		}

		NS_LOG_INFO("\nThe total number of links is: " << m_totalNoLinks);
	}

	TopologyHelper::~TopologyHelper ()
	{
		NS_LOG_FUNCTION(this);
	}

	void
	TopologyHelper::InstallStack (InternetStackHelper stack)
	{
		NS_LOG_FUNCTION(this);
		
		for (uint32_t i = 0; i < m_nodes.size (); ++i)
		{
			NodeContainer currentNode = m_nodes[i];
			for (uint32_t j = 0; j < currentNode.GetN (); ++j)
			{
				stack.Install (currentNode.Get (j));
			}
		}
			
	}

	void
	TopologyHelper::AssignIpv4Addresses (Ipv4AddressHelper ip)
	{
		NS_LOG_FUNCTION(this);
		
		// Assign addresses to all devices in the network.
		// These devices are stored in a vector. 
		for (uint32_t i = 0; i < m_devices.size (); ++i)
		{
			Ipv4InterfaceContainer newInterfaces; 
			NetDeviceContainer currentContainer = m_devices[i];
			
			newInterfaces.Add (ip.Assign (currentContainer.Get (0))); 
			newInterfaces.Add (ip.Assign (currentContainer.Get (1)));
			
			auto interfaceAddress1 = newInterfaces.GetAddress (0);
			auto interfaceAddress2 = newInterfaces.GetAddress (1);
			uint32_t node1 = (currentContainer.Get (0))->GetNode()->GetId();
			uint32_t node2 = (currentContainer.Get (1))->GetNode()->GetId();
						
			m_nodesConnectionsIps[node1].push_back(interfaceAddress2);
			m_nodesConnectionsIps[node2].push_back(interfaceAddress1);

			ip.NewNetwork ();
			m_interfaces.push_back (newInterfaces);
		}
		for(auto &node : m_nodesConnectionsIps)
		{
			for(std::vector<Ipv4Address>::const_iterator it = node.second.begin(); it != node.second.end(); it++)
    		{
				NS_LOG_INFO("\nNode " << node.first << " has connection ip: " << *it);
			}
		}

		NS_LOG_INFO("\nThe total connection ips for node 0: " << m_nodesConnectionsIps[0].size());

	}

	Ipv4InterfaceContainer
	TopologyHelper::GetIpv4InterfaceContainer (void) const
	{
		Ipv4InterfaceContainer ipv4InterfaceContainer;
		
		for (auto container = m_interfaces.begin(); container != m_interfaces.end(); container++)
			ipv4InterfaceContainer.Add(*container);

		return ipv4InterfaceContainer;
	}


	std::map<uint32_t, std::vector<Ipv4Address>> 
	TopologyHelper::GetNodesConnectionsIps (void) const
	{
		return m_nodesConnectionsIps;
	}

	Ptr<Node> 
	TopologyHelper::GetNode (uint32_t id)
	{
		if (id > m_nodes.size () - 1 ) 
		{
			NS_FATAL_ERROR ("Index out of bounds in BlockchainTopologyHelper::GetNode.");
		}

		return (m_nodes.at (id)).Get (0);
	}



};