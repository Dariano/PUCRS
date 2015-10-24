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

void stat_ip(IpHeader* frame, unsigned char* buffer, EthernetHeader* frame_ethernet)
{

  printf("%d\n", frame->Protocol);

  if(frame->Protocol == 89) {
    printf("Protocolo ospf; Se for um hello responder com um hello e estabelecer o roteamento; depois enviar os dados pro router\n");
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
