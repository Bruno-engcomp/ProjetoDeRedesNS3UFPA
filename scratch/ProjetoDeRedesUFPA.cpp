//
// Simulação de uma rede de escritório utilizando ns-3
// Autor: Allana, Bruno e Daniela
//

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

#include <iostream>
#include <map>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RedeEscritorioUFPA");

int main(int argc, char *argv[]) // Declaracao de funcao main
{
    CommandLine cmd(__FILE__); // Funcao utilizada para receber parametros pela linha de comando
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS); // Define que toda a simulação utilizará nanosegundos como unidade interna de tempo.

    // 1. CRIAÇÃO DOS NÓS

    NS_LOG_INFO("Criando os departamentos...");

    Ptr<Node> roteadorCentral = CreateObject<Node>(); // Responsável pela criação de um nó individual, no caso o roteador central

    NodeContainer nosEscritorio; // Container que guarda vários nós, neste caso seria um escritório
    nosEscritorio.Create(3); // funcão responsável por criar os computadores do escritório

    NodeContainer nosVendas; // Container que guarda vários nós, neste caso seria o departamento de vendas
    nosVendas.Create(4); // funcão responsável por criar os computadores do departamento de vendas

    NodeContainer nosDevs; // Container que guarda vários nós, neste caso seria o departamento de desenvolvimento
    nosDevs.Create(4); // funcão responsável por criar os computadores do departamento de desenvolvimento

    // Cada rede contém o roteador e os computadores
    NodeContainer redeEscritorio(roteadorCentral, nosEscritorio);
    NodeContainer redeVendas(roteadorCentral, nosVendas);
    NodeContainer redeDevs(roteadorCentral, nosDevs);

    // 2. CANAIS CSMA

    NS_LOG_INFO("Configurando enlaces CSMA...");

    // Escritório Geral
    CsmaHelper csmaEscritorio; // É responsável por criar uma rede do tipo CSMA/Ethernet: cabo, switch lógico e etc
    csmaEscritorio.SetChannelAttribute("DataRate", StringValue("10Mbps")); // Define a velocidade da rede
    csmaEscritorio.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10))); // Define o atraso de propagação.

    NetDeviceContainer devsEscritorio =
        csmaEscritorio.Install(redeEscritorio); // Instala placas de rede e conecta todos os computadores ao mesmo canal CSMA

    // Sala de Vendas
    CsmaHelper csmaVendas; // É responsável por criar uma rede do tipo CSMA/Ethernet: cabo, switch lógico e etc
    csmaVendas.SetChannelAttribute("DataRate", StringValue("100Mbps")); // Define a velocidade da rede
    csmaVendas.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2))); // Define o atraso de propagação.

    NetDeviceContainer devsVendas =
        csmaVendas.Install(redeVendas); // Instala placas de rede e conecta todos os computadores ao mesmo canal CSMA

    // Sala de Desenvolvimento

    CsmaHelper csmaDevs; // É responsável por criar uma rede do tipo CSMA/Ethernet: cabo, switch lógico e etc
    csmaDevs.SetChannelAttribute("DataRate", StringValue("1Gbps")); // Define a velocidade da rede
    csmaDevs.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1))); // Define o atraso de propagação.

    NetDeviceContainer devsDevs =
        csmaDevs.Install(redeDevs); // Instala placas de rede e conecta todos os computadores ao mesmo canal CSMA


    // 3. PILHA TCP/IP

    NS_LOG_INFO("Instalando pilha de Internet...");

    InternetStackHelper stack; // Adiciona a pilha TCP/IP aos nós.

    // install é uma função do objeto stack para instalar: IPv4, udp, tcp e icmp nos computadores
    stack.Install(roteadorCentral);
    stack.Install(nosEscritorio);
    stack.Install(nosVendas);
    stack.Install(nosDevs);

    // 4. ENDEREÇAMENTO IP

    Ipv4AddressHelper address; // distribui endereços ip

    address.SetBase("192.168.10.0", "255.255.255.0"); // Define qual será a rede, 192.168.10.x
    Ipv4InterfaceContainer ipsEscritorio =
        address.Assign(devsEscritorio); // Entrega um endereço IP para cada placa de rede

    address.SetBase("192.168.20.0", "255.255.255.0"); // Define qual será a rede, 192.168.20.x
    Ipv4InterfaceContainer ipsVendas =
        address.Assign(devsVendas); // Entrega um endereço IP para cada placa de rede

    address.SetBase("192.168.30.0", "255.255.255.0"); // Define qual será a rede, 192.168.30.x
    Ipv4InterfaceContainer ipsDevs =
        address.Assign(devsDevs); // Entrega um endereço IP para cada placa de rede

    // TABELAS DE ROTEAMENTO

    Ipv4GlobalRoutingHelper::PopulateRoutingTables(); // Calcula as tabelas de roteamento

    // 5. APLICAÇÕES

    //servidor 1

    UdpEchoServerHelper servidorDev(9); // Cria um servidor udp que escuta a porta 9

    ApplicationContainer serverApp =
        servidorDev.Install(nosDevs.Get(0));

    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));


    //servidor 2

    UdpEchoServerHelper servidorArquivos(10); // Cria um servidor udp que escuta a porta 10, servidor de arquivos

    ApplicationContainer serverApp2 =
        servidorArquivos.Install(nosDevs.Get(1));

    serverApp2.Start(Seconds(1.0));
    serverApp2.Stop(Seconds(10.0));

    //servidor 3

    UdpEchoServerHelper servidorEscritorio(11);// Cria um servidor udp que escuta a porta 11, servidor de arquivos

    ApplicationContainer serverApp3 =
        servidorEscritorio.Install(nosEscritorio.Get(1));

        serverApp3.Start(Seconds(1.0));
        serverApp3.Stop(Seconds(10.0));

    // CLIENTE 1 =============================

    UdpEchoClientHelper clienteVendas( // Cria um cliente que envia pacotes para porta 9
        ipsDevs.GetAddress(1),
        9);

    clienteVendas.SetAttribute( // Configura o cliente com a quantia maxima de pacotes
        "MaxPackets",
        UintegerValue(10));

    clienteVendas.SetAttribute( // Configura o cliente com o intervalo de 0.5 segundos
        "Interval",
        TimeValue(Seconds(0.5)));

    clienteVendas.SetAttribute( // Configura o tamanho do pacote do cliente
        "PacketSize",
        UintegerValue(1024));

    ApplicationContainer clientApp =  // Guarda as aplicações instaladas
        clienteVendas.Install(
            nosVendas.Get(3));

    clientApp.Start(Seconds(2.0)); // Define quando a aplicação começa e quando termina
    clientApp.Stop(Seconds(10.0));

    // CLIENTE 2 =============================

    UdpEchoClientHelper clienteVendas2(
    ipsDevs.GetAddress(2), // endereço IP do servidor de arquivos
    10);

    clienteVendas2.SetAttribute("MaxPackets", UintegerValue(15));
    clienteVendas2.SetAttribute("Interval", TimeValue(MilliSeconds(300)));
    clienteVendas2.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer client2 =
        clienteVendas2.Install(nosVendas.Get(1));

    client2.Start(Seconds(2.5));
    client2.Stop(Seconds(10.0));

    // CLIENTE 3 =============================

    UdpEchoClientHelper clienteEscritorio(
    ipsDevs.GetAddress(1),
    9);

    clienteEscritorio.SetAttribute("MaxPackets", UintegerValue(20));
    clienteEscritorio.SetAttribute("Interval", TimeValue(MilliSeconds(200)));
    clienteEscritorio.SetAttribute("PacketSize", UintegerValue(256));

    ApplicationContainer client3 =
        clienteEscritorio.Install(nosEscritorio.Get(0));

    client3.Start(Seconds(3.0));
    client3.Stop(Seconds(10.0));

    // CLIENTE 4 ========================================
    UdpEchoClientHelper clienteEscritorio2(
        ipsDevs.GetAddress(1), 
        9);                    

  
    clienteEscritorio2.SetAttribute(
        "MaxPackets",
        UintegerValue(12));

  
    clienteEscritorio2.SetAttribute(
        "Interval",
        TimeValue(MilliSeconds(400)));

    clienteEscritorio2.SetAttribute(
        "PacketSize",
        UintegerValue(400));

    ApplicationContainer client4 =
        clienteEscritorio2.Install(nosEscritorio.Get(2));

    client4.Start(Seconds(3.2));
    client4.Stop(Seconds(10.0));

    // CLIENTE 5 ========================================
    UdpEchoClientHelper clienteVendas3(
        ipsDevs.GetAddress(2), 
        10);                   

    clienteVendas3.SetAttribute(
        "MaxPackets",
        UintegerValue(18));

    clienteVendas3.SetAttribute(
        "Interval",
        TimeValue(MilliSeconds(250)));

    clienteVendas3.SetAttribute(
        "PacketSize",
        UintegerValue(600));

    ApplicationContainer client5 =
        clienteVendas3.Install(nosVendas.Get(0));

    client5.Start(Seconds(2.8));
    client5.Stop(Seconds(10.0));

    // CLIENTE 6 ========================================
    UdpEchoClientHelper clienteVendas4(
        ipsDevs.GetAddress(1),
        9);

    clienteVendas4.SetAttribute(
        "MaxPackets",
        UintegerValue(25));

    clienteVendas4.SetAttribute(
        "Interval",
        TimeValue(MilliSeconds(150)));

    clienteVendas4.SetAttribute(
        "PacketSize",
        UintegerValue(300));

    ApplicationContainer client6 =
        clienteVendas4.Install(nosVendas.Get(2));

    client6.Start(Seconds(3.5));
    client6.Stop(Seconds(10.0));

    // CLIENTE 7 ========================================
    UdpEchoClientHelper clienteDev(
        ipsEscritorio.GetAddress(2), 
        11);          

    clienteDev.SetAttribute(
        "MaxPackets",
        UintegerValue(16));

    clienteDev.SetAttribute(
        "Interval",
        TimeValue(MilliSeconds(350)));

    clienteDev.SetAttribute(
        "PacketSize",
        UintegerValue(700));

    ApplicationContainer client7 =
        clienteDev.Install(nosDevs.Get(2));

    client7.Start(Seconds(2.2));
    client7.Stop(Seconds(10.0));

    // CAPTURA PCAP, salva todos os pacotes em um arquivo .pcap, que pode ser analisado no wireshark

    csmaEscritorio.EnablePcapAll("escritorio");
    csmaVendas.EnablePcapAll("vendas");
    csmaDevs.EnablePcapAll("devs");

    // FLOW MONITOR

    FlowMonitorHelper flowmon;  // cria um monitor de rede
    Ptr<FlowMonitor> monitor =
        flowmon.InstallAll(); // instala o monitor em todos os nos

    NS_LOG_INFO("Iniciando simulação...");

    Simulator::Stop(Seconds(10.0)); // Define quando a simulação termina

    AnimationInterface anim ("simulacao.xml");

    // Roteador
    anim.SetConstantPosition(roteadorCentral, 50, 50);

    // Escritório
    anim.SetConstantPosition(nosEscritorio.Get(0), 15, 20);
    anim.SetConstantPosition(nosEscritorio.Get(1), 15, 50);
    anim.SetConstantPosition(nosEscritorio.Get(2), 15, 80);

    // Vendas
    anim.SetConstantPosition(nosVendas.Get(0), 50, 20);
    anim.SetConstantPosition(nosVendas.Get(1), 50, 40);
    anim.SetConstantPosition(nosVendas.Get(2), 50, 60);
    anim.SetConstantPosition(nosVendas.Get(3), 50, 80);

    // Desenvolvimento
    anim.SetConstantPosition(nosDevs.Get(0), 85, 20);
    anim.SetConstantPosition(nosDevs.Get(1), 85, 40);
    anim.SetConstantPosition(nosDevs.Get(2), 85, 60);
    anim.SetConstantPosition(nosDevs.Get(3), 85, 80);

    Simulator::Run(); // Executa toda a simulação

    // RESULTADOS DA SIMULAÇÃO

    monitor->CheckForLostPackets(); // Conta quantos pacotes foram perdidos

    // Salva estatísticas completas em XML
    monitor->SerializeToXmlFile(
        "flowmon.xml",
        true,
        true);

    Ptr<Ipv4FlowClassifier> classifier =  // Converte o objeto para o tipo correto, para permitir a visualização de ip de origem e destino
        DynamicCast<Ipv4FlowClassifier>(
            flowmon.GetClassifier());

    std::map<FlowId, FlowMonitor::FlowStats> stats =
        monitor->GetFlowStats(); // obtem todas as estatisticas do fluxo de rede

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

    Simulator::Destroy(); // Libera toda a memória utilizada pela simulação.

    return 0;
}
