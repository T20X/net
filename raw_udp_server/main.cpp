/*aw UDP sockets
    Silver Moon (m00n.silv3r@gmail.com)
*/
#include<stdio.h> //for printf
#include<string.h> //memset
#include<sys/socket.h>    //for socket ofcourse
#include<stdlib.h> //for exit(0);
#include<errno.h> //For errno - the error number
#include<netinet/udp.h>   //Provides declarations for udp header
#include<netinet/ip.h>    //Provides declarations for ip header
#include<arpa/inet.h>
#include <unistd.h>

#include <iostream>

#define M_GROUP "192.168.1.75"
#define PORT 51000

union  Wow
{
	struct 
	{
		int a;
		int a1;
		char r;
	};
	struct 
	{
		int b;
		int b1;
	}; 
};
 
/*
    96 bit (12 bytes) pseudo header needed for udp header checksum calculation
*/
struct pseudo_header
{
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
};
 
/*
    Generic checksum calculation function
*/

unsigned short csum(unsigned short *buf, int nwords) 
{       
       unsigned long sum; 
        for(sum=0; nwords>0; nwords--) 
                sum += *buf++;

        sum = (sum >> 16) + (sum &0xffff); 
        sum += (sum >> 16);

        return (unsigned short)(~sum);

}

int main (void)
{
	Wow a1, a2;
	a1.a =2; a1.b1=3;
	a2.b =2; a2.r ='A';
	std::cout << a2.r << std::endl;

    //Create a raw socket of type IPPROTO
    int s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
     
    if(s == -1)
    {
        //socket creation failed, may be because of non-root privileges
        perror("Failed to create raw socket");
        exit(1);
    }
     
    //Datagram to represent the packet
    char datagram[4096] , source_ip[32] , *data , *pseudogram;
     
    //zero out the packet buffer
    memset (datagram, 0, 4096);
     
    //IP header
    struct iphdr *iph = (struct iphdr *) datagram;
     
    //UDP header
    struct udphdr *udph = (struct udphdr *) (datagram + sizeof (struct iphdr));
     
    struct sockaddr_in sin;
    struct pseudo_header psh;
     
    //Data part
    data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
    strcpy(data , "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
     
    //some address resolution
    strcpy(source_ip , "192.168.1.75");
     
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = htonl (INADDR_ANY);
     
    //Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof (struct iphdr)  + sizeof (struct udphdr) + strlen(data);
    iph->id = htons(20745); //Id of this packet
    iph->frag_off = 0;
    iph->ttl = 245;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0;      //Set to 0 before calculating checksum
    iph->saddr = inet_addr(source_ip);    //Spoof the source ip address
    iph->daddr = sin.sin_addr.s_addr;
     
    //Ip checksum
    //iph->check = csum ((unsigned short *) datagram, sizeof(struct iphdr));
    //iph->iph_chksum = csum((unsigned short *)datagram, sizeof(struct ipheader) + sizeof(struct udpheader));
    iph->check = csum ((unsigned short *) datagram, sizeof(struct iphdr));
	std::cout << "IP checksum: " << iph->check << std::endl;
	std::cout << "IP header size: " << sizeof(struct ip) << std::endl;
	std::cout << "IP total length: " << iph->tot_len << std::endl;
	std::cout << "IPHDR header size : " << sizeof(struct iphdr) << std::endl;
     
    //UDP header
    udph->source = htons (12);
    udph->dest = htons (PORT);
    udph->len = htons(sizeof(struct udphdr) + strlen(data)); //tcp header size
    udph->check = 0; //leave checksum 0 now, filled later by pseudo header
     
    //Now the UDP checksum using the pseudo header
    psh.source_address = inet_addr(source_ip);
    psh.dest_address = sin.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = htons(sizeof(struct udphdr) + strlen(data) );
     
    size_t psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + strlen(data);
    pseudogram = (char*)malloc(psize);
     
    memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header) , udph , sizeof(struct udphdr) + strlen(data));
     
    udph->check = 0; //csum( (unsigned short*) pseudogram , psize);
//    udph->check = csum( (unsigned short*) pseudogram , psize);
     
  //bind socket to port
     //bind(s , (struct sockaddr*)&sin, sizeof(sin) );
    //loop if you want to flood :)
    while (1)
    {
		/* IP_HDRINCL must be set on the socket so that the kernel does not attempt 
		 *  to automatically add a default ip header to the packet*/
		int optval = 0;
		setsockopt(s, IPPROTO_RAW, IP_HDRINCL, &optval, sizeof(int));
		u_char loop = 0;
		setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

        //try to receive some data, this is a blocking call

        //Send the packet
        if (sendto (s, datagram, iph->tot_len ,  0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
        {
            perror("sendto failed");
        }
        //Data send successfully
        else
        {
            printf ("Packet Send. Length : %d \n" , iph->tot_len);
        }
		usleep(5000);
    }
     
sleep(2);
    return 0;
}
