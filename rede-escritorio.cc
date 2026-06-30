//
// Simulação de uma rede de escritório utilizando ns-3
// Autor: Bruno
//

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

#include <iostream>
#include <map>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RedeEscritorioUFPA");

int main(int argc, char *argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    // ===============================
    // 1. CRIAÇÃO DOS NÓS
    // ===============================

    NS_LOG_INFO("Criando os departamentos...");

    Ptr<Node> roteadorCentral = CreateObject<Node>();

    NodeContainer nosEscritorio;
    nosEscritorio.Create(3);

    NodeContainer nosVendas;
    nosVendas.Create(4);

    NodeContainer nosDevs;
    nosDevs.Create(4);

    // Cada rede contém o roteador e os computadores
    NodeContainer redeEscritorio(roteadorCentral, nosEscritorio);
    NodeContainer redeVendas(roteadorCentral, nosVendas);
    NodeContainer redeDevs(roteadorCentral, nosDevs);

    // ===============================
    // 2. CANAIS CSMA
    // ===============================

    NS_LOG_INFO("Configurando enlaces CSMA...");

    // Escritório Geral
    CsmaHelper csmaEscritorio;
    csmaEscritorio.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csmaEscritorio.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));

    NetDeviceContainer devsEscritorio =
        csmaEscritorio.Install(redeEscritorio);

    // Sala de Vendas
    CsmaHelper csmaVendas;
    csmaVendas.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csmaVendas.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    NetDeviceContainer devsVendas =
        csmaVendas.Install(redeVendas);

    // Sala de Desenvolvimento
    CsmaHelper csmaDevs;
    csmaDevs.SetChannelAttribute("DataRate", StringValue("1Gbps"));
    csmaDevs.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));

    NetDeviceContainer devsDevs =
        csmaDevs.Install(redeDevs);

    // ===============================
    // 3. PILHA TCP/IP
    // ===============================

    NS_LOG_INFO("Instalando pilha de Internet...");

    InternetStackHelper stack;

    stack.Install(roteadorCentral);
    stack.Install(nosEscritorio);
    stack.Install(nosVendas);
    stack.Install(nosDevs);

    // ===============================
    // 4. ENDEREÇAMENTO IP
    // ===============================

    Ipv4AddressHelper address;

    address.SetBase("192.168.10.0", "255.255.255.0");
    Ipv4InterfaceContainer ipsEscritorio =
        address.Assign(devsEscritorio);

    address.SetBase("192.168.20.0", "255.255.255.0");
    Ipv4InterfaceContainer ipsVendas =
        address.Assign(devsVendas);

    address.SetBase("192.168.30.0", "255.255.255.0");
    Ipv4InterfaceContainer ipsDevs =
        address.Assign(devsDevs);

    // ===============================
    // TABELAS DE ROTEAMENTO
    // ===============================

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // ===============================
    // 5. APLICAÇÕES
    // ===============================

    UdpEchoServerHelper servidorDev(9);

    ApplicationContainer serverApp =
        servidorDev.Install(nosDevs.Get(0));

    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    UdpEchoClientHelper clienteVendas(
        ipsDevs.GetAddress(1),
        9);

    clienteVendas.SetAttribute(
        "MaxPackets",
        UintegerValue(10));

    clienteVendas.SetAttribute(
        "Interval",
        TimeValue(Seconds(0.5)));

    clienteVendas.SetAttribute(
        "PacketSize",
        UintegerValue(1024));

    ApplicationContainer clientApp =
        clienteVendas.Install(
            nosVendas.Get(3));

    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    // ===============================
    // CAPTURA PCAP
    // ===============================

    csmaEscritorio.EnablePcapAll("escritorio");
    csmaVendas.EnablePcapAll("vendas");
    csmaDevs.EnablePcapAll("devs");

    // ===============================
    // FLOW MONITOR
    // ===============================

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor =
        flowmon.InstallAll();

    NS_LOG_INFO("Iniciando simulação...");

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    // ===============================
    // RESULTADOS DA SIMULAÇÃO
    // ===============================

    monitor->CheckForLostPackets();

    // Salva estatísticas completas em XML
    monitor->SerializeToXmlFile(
        "flowmon.xml",
        true,
        true);

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(
            flowmon.GetClassifier());

    std::map<FlowId, FlowMonitor::FlowStats> stats =
        monitor->GetFlowStats();

    std::cout << "\n=========================================\n";
    std::cout << "   RELATÓRIO DA REDE DO ESCRITÓRIO\n";
    std::cout << "=========================================\n\n";

    for (const auto &flow : stats)
    {
        Ipv4FlowClassifier::FiveTuple t =
            classifier->FindFlow(flow.first);

        const FlowMonitor::FlowStats &s = flow.second;

        uint64_t pacotesPerdidos = s.lostPackets;

        std::cout << "Fluxo " << flow.first << "\n";
        std::cout << "Origem:      " << t.sourceAddress << "\n";
        std::cout << "Destino:     " << t.destinationAddress << "\n";
        std::cout << "Protocolo:   "
                  << (t.protocol == 17 ? "UDP" :
                      t.protocol == 6 ? "TCP" : "Outro")
                  << "\n";

        std::cout << "-----------------------------------------\n";

        std::cout << "Pacotes enviados : "
                  << s.txPackets << "\n";

        std::cout << "Pacotes recebidos: "
                  << s.rxPackets << "\n";

        std::cout << "Pacotes perdidos : "
                  << pacotesPerdidos
                  << "\n";

        if (s.txPackets > 0)
        {
            double perda =
                100.0 *
                pacotesPerdidos /
                static_cast<double>(s.txPackets);

            std::cout << "Perda (%):        "
                      << perda
                      << "\n";
        }

        if (s.rxPackets > 0)
        {
            double atrasoMedio =
                s.delaySum.GetSeconds() /
                s.rxPackets;

            std::cout << "Atraso médio:     "
                      << atrasoMedio
                      << " s\n";

            if (s.rxPackets > 1)
            {
                double jitterMedio =
                    s.jitterSum.GetSeconds() /
                    (s.rxPackets - 1);

                std::cout << "Jitter médio:     "
                          << jitterMedio
                          << " s\n";
            }
            else
            {
                std::cout << "Jitter médio:     0 s\n";
            }

            double tempo =
                s.timeLastRxPacket.GetSeconds() -
                s.timeFirstTxPacket.GetSeconds();

            if (tempo > 0)
            {
                double throughput =
                    (s.rxBytes * 8.0) /
                    tempo /
                    1000000.0;

                std::cout << "Throughput:       "
                          << throughput
                          << " Mbps\n";
            }
            else
            {
                std::cout << "Throughput:       N/A\n";
            }
        }
        else
        {
            std::cout << "Atraso médio:     N/A\n";
            std::cout << "Jitter médio:     N/A\n";
            std::cout << "Throughput:       0 Mbps\n";
        }

        std::cout << "Bytes enviados:   "
                  << s.txBytes << "\n";

        std::cout << "Bytes recebidos:  "
                  << s.rxBytes << "\n";

        std::cout << "=========================================\n\n";
    }

    Simulator::Destroy();

    return 0;
}
