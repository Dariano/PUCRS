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

  void make_ospf(OspfHeader* frame_ospf, OspfHeader* frame_ospf_recebido)
  {
    frame_ospf->version = frame_ospf_recebido->version;
    frame_ospf->type = frame_ospf_recebido->type;
    frame_ospf->len = frame_ospf_recebido->len;
    frame_ospf->source = frame_ospf_recebido->source;
    frame_ospf->area_id = frame_ospf_recebido->area_id;
    frame_ospf->auth_type = frame_ospf_recebido->auth_type;
    frame_ospf->auth_key = frame_ospf_recebido->auth_key;
    frame_ospf->chksum = 0x0;
    frame_ospf->chksum = in_cksum((unsigned short *)frame_ospf, sizeof(OspfHeader));

    printf("Gerado\n");
    printf("version %x\n", frame_ospf->version);
    printf("type %x\n", frame_ospf->type);
    printf("len %d\n", frame_ospf->len);
    printf("source %d\n", frame_ospf->source);
    printf("area_id %d\n", frame_ospf->area_id);
    printf("auth_type %d\n", frame_ospf->auth_type);
    printf("auth_key %x\n", frame_ospf->auth_key);
    printf("chksum %x\n", frame_ospf->chksum);   
    printf("----------------------\n");

    printf("Recebido\n");
    printf("version %x\n", frame_ospf_recebido->version);
    printf("type %x\n", frame_ospf_recebido->type);
    printf("len %d\n", frame_ospf_recebido->len);
    printf("source %d\n", frame_ospf_recebido->source);
    printf("area_id %d\n", frame_ospf_recebido->area_id);
    printf("auth_type %d\n", frame_ospf_recebido->auth_type);
    printf("auth_key %x\n", frame_ospf_recebido->auth_key);
    printf("chksum %x\n", frame_ospf_recebido->chksum);   
    frame_ospf_recebido->chksum = 0x0;
    printf("chksum %x\n", in_cksum((unsigned short *)frame_ospf_recebido, sizeof(OspfHeader)));
    printf("----------------------\n");

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
    frame_ip->Checksum = in_cksum((unsigned short *)frame_ip, sizeof(IpHeader));
/*
    printf("Gerado\n");
    printf("VersionIhl %x\n", frame_ip->VersionIhl);
    printf("DscpEcn %x\n", frame_ip->DscpEcn);
    printf("Length %d\n", frame_ip->Length);
    printf("Id %d\n", frame_ip->Id);
    printf("FlagsOffset %d\n", frame_ip->FlagsOffset);
    printf("Ttl %d\n", frame_ip->Ttl);
    printf("Protocol %d\n", frame_ip->Protocol);
    printf("Source %x\n", frame_ip->Source);   
    printf("Destination %x\n", frame_ip->Destination);
    printf("Checksum %x\n", frame_ip->Checksum);
    printf("----------------------\n");

    printf("Recebido\n");    
    printf("VersionIhl %x\n", frame_ip_recebido->VersionIhl);
    printf("DscpEcn %x\n", frame_ip_recebido->DscpEcn);
    printf("Length %d\n", frame_ip_recebido->Length);
    printf("Id %d\n", frame_ip_recebido->Id);
    printf("FlagsOffset %d\n", frame_ip_recebido->FlagsOffset);
    printf("Ttl %d\n", frame_ip_recebido->Ttl);
    printf("Protocol %d\n", frame_ip_recebido->Protocol);
    printf("Source %x\n", frame_ip_recebido->Source);   
    printf("Destination %x\n", frame_ip_recebido->Destination);

    printf("Checksum %x\n", frame_ip_recebido->Checksum);
    frame_ip_recebido->Checksum = 0x0;
    printf("Checksum %x\n", in_cksum((unsigned short *)frame_ip_recebido, sizeof(IpHeader)));
    printf("----------------------\n");
  */
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
    
    frame_ethernet->Type = frame_ethernet_recebido->Type;
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

void ResponderPacoteHello(OspfHeader* frame_ospf_recebido, EthernetHeader* frame_ethernet_recebido, IpHeader* frame_ip_recebido, unsigned char* buffer_recebido) {

  int sockFd = 0, retValue = 0;
  char buffer[BUFFER_LEN], dummy[50];
  struct sockaddr_ll destAddr;
  short int etherTypeT = htons(0x8200);

  EthernetHeader* ethernet = malloc(sizeof(EthernetHeader));
  make_ethernet(ethernet, frame_ethernet_recebido);

  IpHeader* ip = malloc(sizeof(IpHeader));
  make_ip(ip, frame_ip_recebido);

  OspfHeader* ospfHeader = malloc(sizeof(OspfHeader));
  make_ospf(ospfHeader, frame_ospf_recebido);

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
  memcpy((buffer+ETHERTYPE_LEN+(2*MAC_ADDR_LEN)), ip, 78);
  memcpy((buffer+ETHERTYPE_LEN+(2*MAC_ADDR_LEN)) + 20, ospfHeader, sizeof(ip));

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
