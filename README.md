# Projeto da disciplina de Redes de Computadores da Engenharia da Computação - UFPA

## Objetivo

Ao final deste projeto, o aluno deve ter implementado (através de um simulador de redes, como NS-2, NS-3, OPNET ou OMNeT++) uma topologia de rede de computadores contendo tráfego entre os nós (mínimo de 10 nós), demonstrando conhecimentos sobre:

- Introdução à Simulação;
- Ferramentas de Simulação;
- Topologias de Redes de Computadores (lógica e física);
- Modelos de Tráfego.

---

# Descrição do Projeto

Este projeto simula uma rede corporativa utilizando o simulador **NS-3**. O cenário representa uma empresa dividida em três departamentos:

- Escritório Geral;
- Setor de Vendas;
- Setor de Desenvolvimento.

Cada departamento possui sua própria sub-rede Ethernet (CSMA), interligada por um roteador central responsável pelo roteamento entre as redes.

A rede foi construída com **12 nós**, sendo:

- 1 roteador central;
- 3 computadores do Escritório;
- 4 computadores do setor de Vendas;
- 4 computadores do setor de Desenvolvimento.

O projeto utiliza aplicações UDP Echo para simular a comunicação entre os departamentos, permitindo a análise do tráfego através do FlowMonitor e da captura de pacotes (PCAP).

---

# Topologia da Rede

```
                    Roteador Central
                   /       |        \
                  /        |         \
         Escritório     Vendas     Desenvolvimento
      (192.168.10.0) (192.168.20.0) (192.168.30.0)
```

Cada departamento utiliza uma rede CSMA independente, com velocidades e atrasos diferentes para representar diferentes necessidades da empresa.

---

# Configuração das Redes

| Departamento | Rede | Velocidade | Atraso |
|--------------|------|-----------:|--------:|
| Escritório | 192.168.10.0/24 | 10 Mbps | 10 ms |
| Vendas | 192.168.20.0/24 | 100 Mbps | 2 ms |
| Desenvolvimento | 192.168.30.0/24 | 1 Gbps | 1 ms |

---

# Modelo de Tráfego

O tráfego é gerado utilizando aplicações **UDP Echo**.

Um computador do setor de Vendas envia requisições ao servidor localizado no setor de Desenvolvimento.

Durante a simulação são coletadas métricas como:

- quantidade de pacotes enviados;
- quantidade de pacotes recebidos;
- perda de pacotes;
- atraso médio;
- jitter médio;
- throughput;
- quantidade de bytes transmitidos.

As estatísticas são obtidas através do módulo **FlowMonitor**.

---

# Tecnologias Utilizadas

- C++
- NS-3
- CSMA
- IPv4
- UDP Echo
- FlowMonitor
- PCAP

---

# Comentário sobre o funcionamento de funções do NS-3

## `Node`

Representa um dispositivo da rede, como um computador ou um roteador.

---

## `NodeContainer`

Armazena um conjunto de nós, facilitando sua criação e manipulação.

---

## `CreateObject<Node>()`

Cria um nó individual.

No projeto, é utilizado para criar o roteador central.

---

## `CsmaHelper`

Configura uma rede Ethernet baseada no protocolo CSMA.

Também permite definir velocidade de transmissão e atraso do enlace.

---

## `SetChannelAttribute()`

Configura atributos do canal, como:

- DataRate (velocidade da rede);
- Delay (atraso de propagação).

---

## `Install()`

Instala dispositivos de rede nos nós e conecta todos ao mesmo canal CSMA.

---

## `InternetStackHelper`

Instala a pilha de protocolos TCP/IP em todos os computadores da simulação.

---

## `Ipv4AddressHelper`

Responsável por distribuir endereços IPv4 para cada interface da rede.

---

## `PopulateRoutingTables()`

Calcula automaticamente as tabelas de roteamento, permitindo a comunicação entre diferentes sub-redes.

---

## `UdpEchoServerHelper`

Cria um servidor UDP responsável por receber pacotes e devolvê-los ao cliente.

---

## `UdpEchoClientHelper`

Cria um cliente UDP responsável por enviar pacotes ao servidor.

É possível configurar:

- quantidade de pacotes;
- intervalo entre envios;
- tamanho dos pacotes.

---

## `ApplicationContainer`

Armazena as aplicações instaladas em um determinado nó.

---

## `Start()` e `Stop()`

Definem o instante de início e término da execução de cada aplicação durante a simulação.

---

## `EnablePcapAll()`

Gera arquivos no formato **PCAP**, permitindo analisar posteriormente os pacotes utilizando ferramentas como o Wireshark.

---

## `FlowMonitor`

Monitora todos os fluxos da rede, permitindo medir diversas métricas de desempenho.

---

## `CheckForLostPackets()`

Calcula a quantidade de pacotes perdidos durante a simulação.

---

## `SerializeToXmlFile()`

Exporta todas as estatísticas coletadas pelo FlowMonitor para um arquivo XML.

---

## `Simulator::Run()`

Inicia a execução da simulação.

Todos os eventos programados passam a ser executados a partir dessa chamada.

---

## `Simulator::Destroy()`

Libera todos os recursos utilizados pela simulação após sua execução.

---

# Resultados Esperados

Ao executar o projeto é possível observar:

- comunicação entre departamentos;
- funcionamento do roteamento entre sub-redes;
- estatísticas de desempenho da rede;
- arquivos PCAP para análise no Wireshark;
- relatório de fluxo gerado pelo FlowMonitor.

---

# Autores

- Allana
- Bruno Almeida
- Daniela

Disciplina: Redes de Computadores

Universidade Federal do Pará (UFPA)