#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/buildings-module.h"
#include "ns3/lte-module.h"

using namespace ns3;

int
main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (30.0));

  Time simulationLength = Seconds (60);

  uint16_t packetSize = 750; // Bytes
  Time interPacketInterval = MicroSeconds (1000); // useconds

  // Buildings
  // Define a building
  Ptr<Building> building1 = CreateObject<Building> ();
  building1->SetBoundaries (Box (-50.0, -30.0, -50.0, -30.0, 0.0, 20.0));
  building1->SetBuildingType (Building::Residential);
  building1->SetExtWallsType (Building::ConcreteWithWindows);
  building1->SetNFloors (1);

  // Define second building
  Ptr<Building> building2 = CreateObject<Building> ();
  building2->SetBoundaries (Box (40.0, 80.0, 40.0, 80.0, 0.0, 60.0));
  building2->SetBuildingType (Building::Residential);
  building2->SetExtWallsType (Building::ConcreteWithWindows);
  building2->SetNFloors (3);


  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (50));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (50));

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  InternetStackHelper internetHelper;
  internetHelper.Install (remoteHostContainer.Get (0));

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));

  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHostContainer.Get (0));
  Ipv4AddressHelper ipv4AddressHelper;
  ipv4AddressHelper.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpv4Ifaces = ipv4AddressHelper.Assign (internetDevices);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHostContainer.Get (0)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (2);

  Ptr<ListPositionAllocator> positionAllocator = CreateObject<ListPositionAllocator> ();
  positionAllocator->Add (Vector (0, 0, 0));
  positionAllocator->Add (Vector (-10, 0, 0));
  positionAllocator->Add (Vector (10, 0, 0));

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.SetPositionAllocator (positionAllocator);
  mobility.Install (enbNodes);
  mobility.Install (ueNodes);

  enbNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, 0, 0));
  ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, 0, 0));
  ueNodes.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, 0, 0));

  BuildingsHelper::Install (enbNodes);
  BuildingsHelper::Install (ueNodes);
  BuildingsHelper::MakeMobilityModelConsistent ();

  NetDeviceContainer enbNetDeviceContainer;
  NetDeviceContainer uesNetDeviceContainer;
  enbNetDeviceContainer = lteHelper->InstallEnbDevice (enbNodes);
  uesNetDeviceContainer = lteHelper->InstallUeDevice (ueNodes);

  internetHelper.Install (ueNodes);
  Ipv4InterfaceContainer ueIpv4Ifaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (uesNetDeviceContainer));
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      lteHelper->Attach (uesNetDeviceContainer.Get (u), enbNetDeviceContainer.Get (0));
    }


  // Applications: One DL UDP CBR application from the remote server to each UE
  ApplicationContainer serverApps;
  ApplicationContainer clientApps;

  PacketSinkHelper packetSinkH ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 1000));
  serverApps.Add (packetSinkH.Install (ueNodes));

  UdpClientHelper udpClientH (Ipv4Address::GetAny (), 1000);
  udpClientH.SetAttribute ("Interval", TimeValue (interPacketInterval));
  udpClientH.SetAttribute ("PacketSize", UintegerValue (packetSize));
  udpClientH.SetAttribute ("MaxPackets", UintegerValue (1e9)); // To make sure the application does not stop during the simualtion

  udpClientH.SetAttribute ("RemoteAddress", AddressValue (ueIpv4Ifaces.GetAddress (0)));
  udpClientH.SetAttribute ("RemotePort", UintegerValue (1000));
  clientApps.Add (udpClientH.Install (remoteHostContainer.Get (0)));

  udpClientH.SetAttribute ("RemoteAddress", AddressValue (ueIpv4Ifaces.GetAddress (1)));
  udpClientH.SetAttribute ("RemotePort", UintegerValue (1000));
  clientApps.Add (udpClientH.Install (remoteHostContainer.Get (0)));

  serverApps.Start (Seconds (5));
  clientApps.Start (Seconds (10));
  serverApps.Stop (simulationLength - Seconds (5));
  clientApps.Stop (simulationLength - Seconds (10));

  Simulator::Stop (simulationLength);
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
