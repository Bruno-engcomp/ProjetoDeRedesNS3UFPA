

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ProjetoRedesUFPA");

int main (int argc, char *argv[])
{
    CommandLine cmd (__FILE__);
    cmd.Parse (argc, argv);

    Time::SetResolution (Time::NS);
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NS_LOG_INFO ("Criando os nós da topologia...");

    // Criando 2 nós para o link Point-to-Point (ex: Roteador -> Servidor)
    NodeContainer p2pNodes;
    p2pNodes.Create (2);

    // Criando mais 3 nós para a rede local CSMA (Clientes)
    NodeContainer csmaNodes;
    csmaNodes.Add (p2pNodes.Get (1)); // O nó 1 do P2P também faz parte do CSMA (Roteador da LAN)
    csmaNodes.Create (3);

    NS_LOG_INFO ("Configurando os canais de comunicação...");

    // Configura o link P2P: 5 Mbps de banda e 2 ms de atraso
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install (p2pNodes);

    // Configura a rede CSMA (LAN): 100 Mbps de banda e 6560 ns de atraso
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install (csmaNodes);

    NS_LOG_INFO ("Instalando a pilha de protocolos da Internet...");
    InternetStackHelper stack;
    stack.Install (p2pNodes.Get (0));
    stack.Install (csmaNodes); // Instala nos clientes e no nó compartilhado

    NS_LOG_INFO ("Atribuindo endereços IP...");
    Ipv4AddressHelper address;

    // Subrede para o link P2P
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign (p2pDevices);

    // Subrede para a LAN CSMA
    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign (csmaDevices);

    NS_LOG_INFO ("Configurando as Aplicações de Tráfego...");

    // Servidor de Echo no nó 0 do P2P (Porta 9)
    UdpEchoServerHelper echoServer (9);
    ApplicationContainer serverApps = echoServer.Install (p2pNodes.Get (0));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));

    // Cliente de Echo no último nó da rede CSMA mandando pacotes para o Servidor
    UdpEchoClientHelper echoClient (p2pInterfaces.GetAddress (0), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (5));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApps = echoClient.Install (csmaNodes.Get (3));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));

    // Configura o roteamento global para os nós saberem como chegar uns nos outros
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    NS_LOG_INFO ("Iniciando a simulação...");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Simulação finalizada com sucesso!");

    return 0;
}