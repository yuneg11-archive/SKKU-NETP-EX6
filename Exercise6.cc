#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Exercise6");

int main(int argc, char *argv[]) {
    uint32_t payloadSize = 1472;
    uint64_t simulationTime = 10;
    double distance = 5;

    CommandLine cmd;
    cmd.AddValue("payloadSize", "Payload size in bytes", payloadSize);
    cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
    cmd.AddValue("distance", "Distance in meters between the station and the access point", distance);
    cmd.Parse(argc, argv);

    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(1);
    NodeContainer wifiApNodes;
    wifiApNodes.Create(1);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
    phy.SetChannel(channel.Create());

    Ssid ssid = Ssid("Exercise6");

    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211n_5GHZ);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue("HtMcs7"),
                                 "ControlMode", StringValue("HtMcs0"));

    WifiMacHelper mac;
    mac.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid),
                "ActiveProbing", BooleanValue(false));

    NetDeviceContainer staDevice = wifi.Install(phy, mac, wifiStaNodes);

    mac.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid),
                "BeaconInterval", TimeValue(MicroSeconds(102400)),
                "BeaconGeneration", BooleanValue(true));

    NetDeviceContainer apDevice = wifi.Install(phy, mac, wifiApNodes);

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    positionAlloc->Add(Vector(0.0, 0.0, 0.0));
    positionAlloc->Add(Vector(distance, 0.0, 0.0));

    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(wifiApNodes);
    mobility.Install(wifiStaNodes);

    InternetStackHelper stack;
    stack.Install(wifiApNodes);
    stack.Install(wifiStaNodes);

    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer staInterface = address.Assign(staDevice);
    Ipv4InterfaceContainer apInterface = address.Assign(apDevice);

    UdpServerHelper server(9);

    ApplicationContainer serverApp = server.Install(wifiStaNodes.Get(0));
    serverApp.Start(Seconds(0));
    serverApp.Stop(Seconds(simulationTime + 1));

    UdpClientHelper client(staInterface.GetAddress(0), 9);
    client.SetAttribute("MaxPackets", UintegerValue(4294967295u));
    client.SetAttribute("Interval", TimeValue(Time("0.00002")));
    client.SetAttribute("PacketSize", UintegerValue(payloadSize));

    ApplicationContainer clientApp = client.Install(wifiApNodes.Get(0));
    clientApp.Start(Seconds(1));
    clientApp.Stop(Seconds(simulationTime + 1));

    Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();

    uint32_t totalPacketsRecv = DynamicCast<UdpServer>(serverApp.Get(0))->GetReceived();
    double throughput = totalPacketsRecv * payloadSize * 8 / (double)(simulationTime * 1000000);

    std::cout << "Throughput: " << throughput << " Mbps" << std::endl;

    Simulator::Destroy();

    return 0;
}