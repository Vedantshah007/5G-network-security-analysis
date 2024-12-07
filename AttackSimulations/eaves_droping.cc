#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mmwave-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/trace-helper.h"

using namespace ns3;

int main(int argc, char *argv[])
{
  // Set up default simulation parameters
  uint32_t nEnb = 1;  // Number of eNBs
  uint32_t nUe = 1;   // Number of UEs
  double simulationTime = 10.0; // Simulation time in seconds

  CommandLine cmd;
  cmd.AddValue("nEnb", "Number of eNBs", nEnb);
  cmd.AddValue("nUe", "Number of UEs", nUe);
  cmd.AddValue("simTime", "Simulation time in seconds", simulationTime);
  cmd.Parse(argc, argv);

  // Create mmWave helpers
  Ptr<mmWaveHelper> mmWaveHelper = CreateObject<mmWaveHelper>();
  Ptr<NetDevice> enbmmWaveDevs, uemmWaveDevs;
  Ptr<mmWaveEpcHelper> epcHelper = CreateObject<mmWaveEpcHelper>();

  // Create nodes
  NodeContainer ueNodes, enbNodes;
  enbNodes.Create(nEnb);
  ueNodes.Create(nUe);

  // Set mobility model
  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX", DoubleValue(0.0),
                                "MinY", DoubleValue(0.0),
                                "DeltaX", DoubleValue(5.0),
                                "DeltaY", DoubleValue(5.0),
                                "GridWidth", UintegerValue(3),
                                "LayoutType", StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);

  // Install internet stack
  InternetStackHelper internet;
  internet.Install(enbNodes);
  internet.Install(ueNodes);

  // Install mmWave devices and attach to the nodes
  mmWaveHelper->SetEpcHelper(epcHelper);
  enbmmWaveDevs = mmWaveHelper->InstallEnbDevice(enbNodes);
  uemmWaveDevs = mmWaveHelper->InstallUeDevice(ueNodes);

  // Attach UE to eNB
  Ptr<Node> ueNode = ueNodes.Get(0);
  Ptr<Node> enbNode = enbNodes.Get(0);
  mmWaveHelper->AttachToClosestEnb(uemmWaveDevs.Get(0), enbmmWaveDevs.Get(0));

  // Install applications (simple packet sink for example)
  uint16_t port = 9;
  Address sinkAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
  PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", sinkAddress);
  ApplicationContainer sinkApp = sinkHelper.Install(ueNodes);
  sinkApp.Start(Seconds(1.0));
  sinkApp.Stop(Seconds(simulationTime));

  // Install UdpClient application on eNB side
  UdpClientHelper udpClientHelper(ueNode->GetObject<Ipv4>()->GetAddress(1).GetLocal(), port);
  udpClientHelper.SetAttribute("MaxPackets", UintegerValue(1000));
  udpClientHelper.SetAttribute("Interval", TimeValue(MilliSeconds(100)));
  udpClientHelper.SetAttribute("PacketSize", UintegerValue(1024));
  ApplicationContainer clientApp = udpClientHelper.Install(enbNodes);
  clientApp.Start(Seconds(1.0));
  clientApp.Stop(Seconds(simulationTime));

  // Set up tracing to generate pcap files for the devices
  PointToPointHelper p2ph;
  p2ph.EnablePcap("mmwave-enb", enbmmWaveDevs.Get(0));
  p2ph.EnablePcap("mmwave-ue", uemmWaveDevs.Get(0));

  // Start simulation
  Simulator::Stop(Seconds(simulationTime));
  Simulator::Run();

  // Clean up and finish
  Simulator::Destroy();

  return 0;
}
