#include "ns3/applications-module.h"
#include "ns3/command-line.h"
#include "ns3/config-store-module.h"
#include "ns3/internet-module.h"
#include "ns3/mmwave-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/ipv4-flow-classifier.h"

using namespace ns3;
using namespace mmwave;

NS_LOG_COMPONENT_DEFINE("EpcMitmExample");

Ptr<PacketSink> sink; // Declare a global variable to hold the PacketSink instance
uint64_t lastTotalRx = 0; // To store the last received bytes count

// Define the callback function for receiving packets
void ReceivePacket(Ptr<Socket> socket) {
    Ptr<Packet> packet;
    while ((packet = socket->Recv())) {
        NS_LOG_UNCOND("MITM: Intercepted packet of size " << packet->GetSize() << " bytes");
    }
}

int main(int argc, char* argv[]){
    uint16_t numEnb = 1;
    uint16_t numUe = 1;
    double simTime = 10.0;
    double minDistance = 10.0;
    double maxDistance = 150.0;
    bool harqEnabled = true;
    bool rlcAmEnabled = false;

    // Command line arguments
    CommandLine cmd;
    cmd.AddValue("numEnb", "Number of eNBs", numEnb);
    cmd.AddValue("numUe", "Number of UEs per eNB", numUe);
    cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
    cmd.AddValue("harq", "Enable Hybrid ARQ", harqEnabled);
    cmd.AddValue("rlcAm", "Enable RLC-AM", rlcAmEnabled);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue(rlcAmEnabled));
    Config::SetDefault("ns3::MmWaveHelper::HarqEnabled", BooleanValue(harqEnabled));

    Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper>();
    Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper>();
    mmwaveHelper->SetEpcHelper(epcHelper);
    mmwaveHelper->SetHarqEnabled(harqEnabled);

    Ptr<Node> pgw = epcHelper->GetPgwNode();

    // Create a single RemoteHost
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);
    InternetStackHelper internet;
    internet.Install(remoteHostContainer);

    // Create the Internet
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
    p2ph.SetDeviceAttribute("Mtu", UintegerValue(1600));
    p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));
    NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
    //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress(1);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

    NodeContainer ueNodes;
    NodeContainer enbNodes;
    enbNodes.Create(numEnb);
    ueNodes.Create(numUe);
    internet.Install(ueNodes);
    internet.Install(enbNodes);

    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> flowMonitor = flowmonHelper.InstallAll();

    // Install Mobility Model
    Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator>();
    enbPositionAlloc->Add(Vector(0.0, 0.0, 0.0));
    MobilityHelper enbmobility;
    enbmobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    enbmobility.SetPositionAllocator(enbPositionAlloc);
    enbmobility.Install(enbNodes);

    MobilityHelper uemobility;
    Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator>();
    Ptr<UniformRandomVariable> distRv = CreateObject<UniformRandomVariable>();
    for (unsigned i = 0; i < numUe; i++){
        double dist = distRv->GetValue(minDistance, maxDistance);
        uePositionAlloc->Add(Vector(dist, 0.0, 0.0));
    }
    uemobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    uemobility.SetPositionAllocator(uePositionAlloc);
    uemobility.Install(ueNodes);

    // Install mmWave Devices to the nodes
    NetDeviceContainer enbmmWaveDevs = mmwaveHelper->InstallEnbDevice(enbNodes);
    NetDeviceContainer uemmWaveDevs = mmwaveHelper->InstallUeDevice(ueNodes);

    // Install the IP stack on the UEs
    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(uemmWaveDevs));
    Ipv4Address gatewayAddr = epcHelper->GetUeDefaultGatewayAddress();
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u){
        Ptr<Node> ueNode = ueNodes.Get(u);
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
        ueStaticRouting->SetDefaultRoute(gatewayAddr, 1);
    }

    mmwaveHelper->AttachToClosestEnb(uemmWaveDevs, enbmmWaveDevs);

    // Create Attacker Node
    NodeContainer attackerNode;
    attackerNode.Create(1);
    internet.Install(attackerNode); // Install Internet stack


    // Set mobility for attacker (between UE and eNB)
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(attackerNode);

    // Configure the attacker 
    uint16_t ueToAttackerPort = 1234; // Port for UE to Attacker traffic
    uint16_t attackerToEnbPort = 5678; // Port for Attacker to eNB traffic

    Ipv4AddressHelper ipHelper;
    ipHelper.SetBase("10.1.1.0", "255.255.255.0");
    NetDeviceContainer attackerDevices = p2ph.Install(attackerNode.Get(0), pgw);
    Ipv4InterfaceContainer attackerIpIfaces = ipHelper.Assign(attackerDevices);

    // Retrieve the correct IPv4 address
    Ipv4Address attackerIpv4 = attackerIpIfaces.GetAddress(0);
    NS_LOG_UNCOND("Attacker IPv4 Address: " << attackerIpv4);


    InetSocketAddress attackerAddress = InetSocketAddress(attackerIpv4, ueToAttackerPort);
    Ptr<Ipv4> ipv4 = attackerNode.Get(0)->GetObject<Ipv4>();

    NS_LOG_UNCOND("Attacker Address: " << attackerAddress);
    NS_LOG_UNCOND("Gateway Address: " << gatewayAddr);

    // UE to Attacker Application
    OnOffHelper ueToAttacker("ns3::UdpSocketFactory", attackerAddress);
    ueToAttacker.SetAttribute("DataRate", StringValue("1Mbps"));
    ueToAttacker.SetAttribute("PacketSize", UintegerValue(1400));
    ApplicationContainer ueApp = ueToAttacker.Install(ueNodes.Get(0));
    ueApp.Start(Seconds(1.0));
    ueApp.Stop(Seconds(simTime));

    // Attacker to eNB Application
    OnOffHelper attackerToEnb("ns3::UdpSocketFactory", InetSocketAddress(gatewayAddr, attackerToEnbPort));
    attackerToEnb.SetAttribute("DataRate", StringValue("1Mbps"));
    attackerToEnb.SetAttribute("PacketSize", UintegerValue(1400));
    ApplicationContainer attackerApp = attackerToEnb.Install(attackerNode.Get(0));
    attackerApp.Start(Seconds(1.1)); // Start slightly later to simulate relay
    attackerApp.Stop(Seconds(simTime));

    uint16_t attackPort = 12345; // Define the attack port

    PacketSinkHelper dosSink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), attackPort));
    ApplicationContainer targetSinkApp = dosSink.Install(enbNodes.Get(0)); // Sink on the eNB
    targetSinkApp.Start(Seconds(0.0));
    targetSinkApp.Stop(Seconds(simTime));

    sink = DynamicCast<PacketSink>(targetSinkApp.Get(0));

    // Set up the attacker socket to intercept packets
    Ptr<Socket> socket = Socket::CreateSocket(attackerNode.Get(0), UdpSocketFactory::GetTypeId());
    if (socket == nullptr) {
        NS_LOG_UNCOND("Failed to create socket.");
        return 1; // Exit with error code
    }	
    socket->SetRecvCallback(MakeCallback(&ReceivePacket));
    int bindResult = socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), ueToAttackerPort));
    if (bindResult < 0) {
        NS_LOG_UNCOND("Failed to bind socket to port " << ueToAttackerPort);
        return 1;
    }


    p2ph.EnablePcapAll("mmwave-mitm-epc");
    p2ph.EnablePcap("pgw_to_remoteHost_mitm", internetDevices.Get(0), true); // PGW to RemoteHost link
    p2ph.EnablePcap("attacker_to_enb_mitm", attackerDevices, true);  // Attacker to eNB link

    mmwaveHelper->EnableTraces();

    // Run the simulation
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    flowMonitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i) {
    uint64_t transmittedBytes = i->second.txBytes;
    uint64_t receivedBytes = i->second.rxBytes;
    uint64_t lostBytes = transmittedBytes - receivedBytes;
    uint32_t packetSize = 1400;  // Example packet size
    uint64_t lostPackets = lostBytes / packetSize;

    NS_LOG_UNCOND("Flow ID " << i->first << " - Transmitted: " << transmittedBytes << " bytes, "
                                << "Received: " << receivedBytes << " bytes, "
                                << "Lost: " << lostBytes << " bytes, "
                                << "Lost Packets: " << lostPackets);
    }
    Simulator::Destroy();

    return 0;
}
