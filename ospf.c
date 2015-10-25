#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
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

void make_ip(IpHeader* frame_ip)
{
  frame_ip->VersionIhl = 0x4;
  frame_ip->DscpEcn = 0x00;
  frame_ip->Length = 0x14;
  frame_ip->Id = 0x01;
  frame_ip->FlagsOffset = 0x00;
  frame_ip->Ttl = 0x80;
  frame_ip->Protocol = 0x11;
  frame_ip->Checksum = 0x00;
  frame_ip->Source = 0x00;
  frame_ip->Destination = 0xFFFFFFFF;
}

void make_ethernet(EthernetHeader* frame_ethernet)
{
  frame_ethernet->Destination[0] = 0xFF;
  frame_ethernet->Destination[1] = 0xFF;
  frame_ethernet->Destination[2] = 0xFF;
  frame_ethernet->Destination[3] = 0xFF;
  frame_ethernet->Destination[4] = 0xFF;
  frame_ethernet->Destination[5] = 0xFF;
  frame_ethernet->Source[0] = 0x00;
  frame_ethernet->Source[1] = 0x00;
  frame_ethernet->Source[2] = 0x00;
  frame_ethernet->Source[3] = 0x00;
  frame_ethernet->Source[4] = 0x00;
  frame_ethernet->Source[5] = 0x01;
  frame_ethernet->Type = 0x0800;
}

void ResponderPacoteHello(OspfHeader* frame, unsigned char* buffer){

  int sock, i;
  struct ifreq ifr;
  struct sockaddr_ll to;
  socklen_t len;
  unsigned char addr[6];
  memset(&ifr, 0, sizeof(ifr));

  if((sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0){
    printf("Erro na criacao do socket.\n");
    exit(1);
  }
  
  to.sll_protocol= htons(ETH_P_ALL);
  to.sll_ifindex = 1;
  
  // Montar buffer e responder pacote 

  sendto(sock, (char *) buffer, sizeof(buffer), 0, (struct sockaddr*) &to,len);
  printf("Respondi o pacote hello\n");
}

void stat_ip(IpHeader* frame, unsigned char* buffer, EthernetHeader* frame_ethernet)
{
  if(frame->Protocol == 89) {

    OspfHeader* ospf = (OspfHeader*)(buffer + 20);

    if(ospf->type == 1) {
      // Hello packet - Responder o pacote hello
      printf("Tentando responder pacote hello\n");
      ResponderPacoteHello(ospf, buffer);
    }

    /*printf("Version: %d\n",ospf->version );
    printf("Type: %d\n",ospf->type );
    printf("Source: %x\n",ospf->source );
    printf("Len: %d\n",ospf->len );
    printf("Area_id: %d\n",ospf->area_id );
    printf("Chksum: %x\n",ospf->chksum );
    printf("Auth_type: %d\n",ospf->auth_type );
    printf("------------------------\n");*/
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
