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
    // Valor padrão do cenário.
    // Caso o usuário não informe --cenario na linha de comando,
    // a simulação executará o cenário 1.
    int cenario = 1;

    CommandLine cmd(__FILE__); // Funcao utilizada para receber parametros pela linha de comando

    cmd.AddValue(
    "cenario",
    "Cenário da simulação (1, 2 ou 3)",
    cenario);

    // Lê os argumentos informados na linha de comando
    // e atualiza as variáveis correspondentes
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS); // Define que toda a simulação utilizará nanosegundos como unidade interna de tempo.

    // 1. CRIAÇÃO DOS NÓS
    // Cada NodeContainer representa uma rede local (LAN).
    // O roteador participa das três redes para permitir
    // comunicação entre departamentos.

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

    // A pilha TCP/IP deve ser instalada em todos os dispositivos
    // que irão enviar ou receber pacotes pela rede.
    InternetStackHelper stack; // Adiciona a pilha TCP/IP aos nós.

    // install é uma função do objeto stack para instalar: IPv4, udp, tcp e icmp nos computadores
    stack.Install(roteadorCentral);
    stack.Install(nosEscritorio);
    stack.Install(nosVendas);
    stack.Install(nosDevs);

    // 4. ENDEREÇAMENTO IP
    // Cada rede recebe uma faixa de endereços IP diferente,
    // permitindo que o roteador encaminhe os pacotes entre elas.
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
    // Os servidores UDP Echo recebem um pacote e o devolvem
    // ao cliente, sendo utilizados para testar comunicação
    // entre os departamentos.

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

    // ======================================================
    // CLIENTES
    // O cenário é escolhido pela linha de comando:
    // ======================================================

    if (cenario == 1 || cenario == 4)
    {
        // ==========================================
        // CENÁRIO 1
        // VENDAS ---> DESENVOLVIMENTO
        // ==========================================


        // Os clientes UDP enviam pacotes para os respectivos servidores.
        // Cada cliente possui parâmetros diferentes de quantidade,
        // intervalo e tamanho dos pacotes para representar
        // diferentes padrões de tráfego.
        // Cliente 1
        UdpEchoClientHelper clienteVendas1(
            ipsDevs.GetAddress(1),
            9);

        clienteVendas1.SetAttribute("MaxPackets", UintegerValue(10));
        clienteVendas1.SetAttribute("Interval", TimeValue(Seconds(0.5)));
        clienteVendas1.SetAttribute("PacketSize", UintegerValue(1024));

        ApplicationContainer app1 =
            clienteVendas1.Install(nosVendas.Get(3));

        app1.Start(Seconds(2.0));
        app1.Stop(Seconds(10.0));


        // Cliente 2
        UdpEchoClientHelper clienteVendas2(
            ipsDevs.GetAddress(2),
            10);

        clienteVendas2.SetAttribute("MaxPackets", UintegerValue(15));
        clienteVendas2.SetAttribute("Interval", TimeValue(MilliSeconds(300)));
        clienteVendas2.SetAttribute("PacketSize", UintegerValue(512));

        ApplicationContainer app2 =
            clienteVendas2.Install(nosVendas.Get(1));

        app2.Start(Seconds(2.5));
        app2.Stop(Seconds(10.0));


        // Cliente 3
        UdpEchoClientHelper clienteVendas3(
            ipsDevs.GetAddress(2),
            10);

        clienteVendas3.SetAttribute("MaxPackets", UintegerValue(18));
        clienteVendas3.SetAttribute("Interval", TimeValue(MilliSeconds(250)));
        clienteVendas3.SetAttribute("PacketSize", UintegerValue(600));

        ApplicationContainer app3 =
            clienteVendas3.Install(nosVendas.Get(0));

        app3.Start(Seconds(2.8));
        app3.Stop(Seconds(10.0));


        // Cliente 4
        UdpEchoClientHelper clienteVendas4(
            ipsDevs.GetAddress(1),
            9);

        clienteVendas4.SetAttribute("MaxPackets", UintegerValue(25));
        clienteVendas4.SetAttribute("Interval", TimeValue(MilliSeconds(150)));
        clienteVendas4.SetAttribute("PacketSize", UintegerValue(300));

        ApplicationContainer app4 =
            clienteVendas4.Install(nosVendas.Get(2));

        app4.Start(Seconds(3.5));
        app4.Stop(Seconds(10.0));
    }

    if (cenario == 2 || cenario == 4)
    {
        // ==========================================
        // CENÁRIO 2
        // ADMINISTRAÇÃO ---> VENDAS
        // ==========================================

        // Servidor em Vendas
        UdpEchoServerHelper servidorVendas(12);

        ApplicationContainer serverVendas =
            servidorVendas.Install(nosVendas.Get(0));

        serverVendas.Start(Seconds(1.0));
        serverVendas.Stop(Seconds(10.0));

        // Cliente 5
        UdpEchoClientHelper clienteAdm1(
            ipsVendas.GetAddress(1),
            12);

        clienteAdm1.SetAttribute("MaxPackets", UintegerValue(20));
        clienteAdm1.SetAttribute("Interval", TimeValue(MilliSeconds(200)));
        clienteAdm1.SetAttribute("PacketSize", UintegerValue(256));

        ApplicationContainer app5 =
            clienteAdm1.Install(nosEscritorio.Get(0));

        app5.Start(Seconds(3.0));
        app5.Stop(Seconds(10.0));

        // Cliente 6
        UdpEchoClientHelper clienteAdm2(
            ipsVendas.GetAddress(1),
            12);

        clienteAdm2.SetAttribute("MaxPackets", UintegerValue(12));
        clienteAdm2.SetAttribute("Interval", TimeValue(MilliSeconds(400)));
        clienteAdm2.SetAttribute("PacketSize", UintegerValue(400));

        ApplicationContainer app6 =
            clienteAdm2.Install(nosEscritorio.Get(2));

        app6.Start(Seconds(3.2));
        app6.Stop(Seconds(10.0));
    }

    if (cenario == 3 || cenario == 4)
    {
        // ==========================================
        // CENÁRIO 3
        // DESENVOLVIMENTO ---> ADMINISTRAÇÃO
        // ==========================================

        // Cliente 7
        UdpEchoClientHelper clienteDev(
            ipsEscritorio.GetAddress(2),
            11);

        clienteDev.SetAttribute("MaxPackets", UintegerValue(16));
        clienteDev.SetAttribute("Interval", TimeValue(MilliSeconds(350)));
        clienteDev.SetAttribute("PacketSize", UintegerValue(700));

        ApplicationContainer app7 =
            clienteDev.Install(nosDevs.Get(2));

        app7.Start(Seconds(2.2));
        app7.Stop(Seconds(10.0));
    }

    // CENÁRIO 4
    // TODOS AO MESMO TEMPO
    // VENDAS ---> DESENVOLVIMENTO
    // ADMINISTRAÇÃO ---> VENDAS
    // DESENVOLVIMENTO ---> ADMINISTRAÇÃO

    // CAPTURA PCAP, salva todos os pacotes em um arquivo .pcap, que pode ser analisado no wireshark
    // Habilita a captura de todos os pacotes trafegados.
    // Os arquivos .pcap podem ser abertos posteriormente
    // no Wireshark para análise detalhada.
    csmaEscritorio.EnablePcapAll("escritorio");
    csmaVendas.EnablePcapAll("vendas");
    csmaDevs.EnablePcapAll("devs");

    // FLOW MONITOR
    // O FlowMonitor coleta estatísticas de cada fluxo
    // de comunicação durante toda a simulação.

    FlowMonitorHelper flowmon;  // cria um monitor de rede
    Ptr<FlowMonitor> monitor =
        flowmon.InstallAll(); // instala o monitor em todos os nos

    NS_LOG_INFO("Iniciando simulação...");

    Simulator::Stop(Seconds(10.0)); // Define quando a simulação termina

    // Cria um arquivo XML que poderá ser aberto no NetAnim,
    // permitindo visualizar graficamente a rede e os pacotes.
    AnimationInterface anim ("simulacao.xml");


    // Define posições fixas para os nós apenas para fins
    // de visualização no NetAnim.
    // Essas coordenadas não influenciam na simulação da rede.
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

    // Inicia a execução da simulação.
    // A partir deste ponto todos os eventos agendados
    // começam a ocorrer.
    Simulator::Run(); // Executa toda a simulação

    // RESULTADOS DA SIMULAÇÃO

    // Atualiza as estatísticas de perda de pacotes
    // antes da geração do relatório final.
    monitor->CheckForLostPackets(); // Conta quantos pacotes foram perdidos

    // Salva estatísticas completas em XML
    // Exporta todas as estatísticas do FlowMonitor
    // para um arquivo XML, permitindo análises posteriores.
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

    // Percorre todos os fluxos monitorados e imprime
    // um relatório contendo métricas de desempenho.
    for (const auto &flow : stats)
    {
        Ipv4FlowClassifier::FiveTuple t =
            classifier->FindFlow(flow.first);

        const FlowMonitor::FlowStats &s = flow.second;

        // Quantidade de pacotes que foram enviados,
        // mas não chegaram ao destino.
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
            // Calcula a porcentagem de pacotes perdidos
            // em relação ao total transmitido.
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
                // Jitter representa a variação do atraso entre
                // pacotes consecutivos recebidos.
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
                // Throughput representa a taxa efetiva de dados
                // recebidos durante o tempo de transmissão.
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

    // Encerra completamente a simulação,
    // liberando memória e recursos utilizados.
    Simulator::Destroy(); // Libera toda a memória utilizada pela simulação.

    return 0;
}
