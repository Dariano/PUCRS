  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <string.h>
  #include <netdb.h>

  #include <sys/signal.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <sys/socket.h>
  #include <sys/ioctl.h>

  #include <netinet/in_systm.h>
  #include <netinet/in.h>
  #include <netinet/ip.h>
  #include <netinet/udp.h>
  #include <netinet/ip_icmp.h>
  #include <netinet/tcp.h>

  #include <linux/if_ether.h>
  #include <netpacket/packet.h>
  #include <net/ethernet.h>
  #include <netinet/ether.h>
  #include <arpa/inet.h>
  #include <time.h>
  #include <unistd.h>
  #include <pthread.h>
  #include <net/if.h>
  #include <netinet/ether.h>
  #include <netinet/in.h>
  #include <netinet/in_systm.h>
  #include "netstructs.h"

  #define BUFFER_LEN 1518
  #define ETHERTYPE_LEN 2
  #define MAC_ADDR_LEN 6

  // Atencao!! Confira no /usr/include do seu sisop o nome correto
  // das estruturas de dados dos protocolos.

typedef unsigned char MacAddress[MAC_ADDR_LEN];
extern int errno;

  unsigned char buff[BUFFER_LEN]; // buffer de recepcao enlace

  unsigned short in_cksum(unsigned short *addr, int len)
  {
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1) {
      sum += *w++;
      nleft -= 2;
    }

    if (nleft == 1) {
      *(unsigned char *) (&answer) = *(unsigned char *) w;
      sum += answer;
    }
    
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
  }

  void make_ip(IpHeader* frame_ip, IpHeader* frame_ip_recebido)
  {
    frame_ip->VersionIhl = frame_ip_recebido->VersionIhl;
    frame_ip->DscpEcn = frame_ip_recebido->DscpEcn;
    frame_ip->Length = frame_ip_recebido->Length;
    frame_ip->Id = frame_ip_recebido->Id + 1;
    frame_ip->FlagsOffset = frame_ip_recebido->FlagsOffset;
    frame_ip->Ttl = frame_ip_recebido->Ttl;
    frame_ip->Protocol = frame_ip_recebido->Protocol;
    frame_ip->Source = frame_ip_recebido->Destination;
    frame_ip->Destination = frame_ip_recebido->Source;
    frame_ip->Checksum = 0X0;
    frame_ip->Checksum = in_cksum((unsigned short *)&frame_ip, sizeof(&frame_ip));
  }

  void make_ethernet(EthernetHeader* frame_ethernet, EthernetHeader* frame_ethernet_recebido)
  {
    frame_ethernet->Destination[0] = frame_ethernet_recebido->Source[0];
    frame_ethernet->Destination[1] = frame_ethernet_recebido->Source[1];
    frame_ethernet->Destination[2] = frame_ethernet_recebido->Source[2];
    frame_ethernet->Destination[3] = frame_ethernet_recebido->Source[3];
    frame_ethernet->Destination[4] = frame_ethernet_recebido->Source[4];
    frame_ethernet->Destination[5] = frame_ethernet_recebido->Source[5];

    frame_ethernet->Source[0] = frame_ethernet_recebido->Destination[0];
    frame_ethernet->Source[1] = frame_ethernet_recebido->Destination[1];
    frame_ethernet->Source[2] = frame_ethernet_recebido->Destination[2];
    frame_ethernet->Source[3] = frame_ethernet_recebido->Destination[3];
    frame_ethernet->Source[4] = frame_ethernet_recebido->Destination[4];
    frame_ethernet->Source[5] = frame_ethernet_recebido->Destination[5];
    
    frame_ethernet->Type = 0x0800;
/*
    printf("MAC Source Recebido %x:%x:%x:%x:%x:%x\n",frame_ethernet_recebido->Source[0] , frame_ethernet_recebido->Source[1],frame_ethernet_recebido->Source[2],
      frame_ethernet_recebido->Source[3], frame_ethernet_recebido->Source[4],frame_ethernet_recebido->Source[5]);

    printf("MAC Destination Recebido %x:%x:%x:%x:%x:%x\n",frame_ethernet_recebido->Destination[0] , frame_ethernet_recebido->Destination[1],frame_ethernet_recebido->Destination[2],
      frame_ethernet_recebido->Destination[3], frame_ethernet_recebido->Destination[4],frame_ethernet_recebido->Destination[5]);

    printf("MAC Destination Hello %x:%x:%x:%x:%x:%x\n",frame_ethernet->Destination[0] , frame_ethernet->Destination[1],frame_ethernet->Destination[2],
      frame_ethernet->Destination[3], frame_ethernet->Destination[4],frame_ethernet->Destination[5]);

    printf("MAC Source Hello %x:%x:%x:%x:%x:%x\n",frame_ethernet->Source[0] , frame_ethernet->Source[1],frame_ethernet->Source[2],
      frame_ethernet->Source[3], frame_ethernet->Source[4],frame_ethernet->Source[5]);
*/
  }

  void ResponderPacoteHello(OspfHeader* frame, EthernetHeader* frame_ethernet_recebido, IpHeader* frame_ip_recebido, unsigned char* buffer_recebido) {

    int sockFd = 0, retValue = 0;
    char buffer[BUFFER_LEN], dummy[64];
    struct sockaddr_ll destAddr;
    short int etherTypeT = htons(0x8200);

    EthernetHeader* ethernet = malloc(sizeof(EthernetHeader));
    make_ethernet(ethernet, frame_ethernet_recebido);

    IpHeader* ip = malloc(sizeof(IpHeader));
    make_ip(ip, frame_ip_recebido);
    
    if((sockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
      printf("Erro na criacao do socket.\n");
      exit(1);
    }

    destAddr.sll_family = htons(PF_PACKET);
    destAddr.sll_protocol = htons(ETH_P_ALL);
    destAddr.sll_halen = 6;
    destAddr.sll_ifindex = 2;  /* indice da interface pela qual os pacotes serao enviados. Eh necess�rio conferir este valor. */
    memcpy(&(destAddr.sll_addr), ethernet->Destination, MAC_ADDR_LEN);

    /* Cabecalho Ethernet */
    memcpy(buffer, ethernet, sizeof(EthernetHeader));
    memcpy((buffer+ETHERTYPE_LEN+(2*MAC_ADDR_LEN)), ip, 64);

    retValue = sendto(sockFd, buffer, 78, 0, (struct sockaddr *)&(destAddr), sizeof(struct sockaddr_ll));

    if(retValue < 0)
      printf("Não respondi o pacote hello\n");
    else
      printf("Respondi pacote hello (%d)\n", retValue);

  }

  void stat_ip(IpHeader* frame, unsigned char* buffer, EthernetHeader* frame_ethernet)
  {
    if(frame->Protocol == 89) {

      OspfHeader* ospf = (OspfHeader*)(buffer + 20);

      if(ospf->type == 1) {
        // Hello packet - Responder o pacote hello
        printf("Tentando responder pacote hello\n");
     
        ResponderPacoteHello(ospf, frame_ethernet, frame, buffer);
      }

    }
    
  }

  void stat_ethernet(EthernetHeader* frame, unsigned char* buffer)
  {
    if(ntohs(frame->Type) == 0x0800)
    {
      IpHeader* ip = (IpHeader*)(buffer + 14);
      int size = 14 + ntohs(ip->Length);
      stat_ip(ip, (buffer+14), frame);
    }
  }


  int main(int argc, char **argv)
  {
    printf("Iniciando programa\n");
    int socket_raw = 0;
    struct ifreq ifr;

    if((socket_raw = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
      printf("Error: Socket did not initialize. \n");
      exit(1);
    }

    ioctl(socket_raw, SIOCGIFFLAGS, &ifr);
    ifr.ifr_flags |= IFF_PROMISC;
    ioctl(socket_raw, SIOCSIFFLAGS, &ifr);
    
    unsigned char buffer[BUFFER_LEN];

    while (1)
    {
      recv(socket_raw,(char *) &buffer, BUFFER_LEN, 0x0);
      
      EthernetHeader* ethheader = (EthernetHeader*)buffer;

      stat_ethernet(ethheader, buffer);
    }

    printf("Finalizando programa\n");

    return 0;
  }
