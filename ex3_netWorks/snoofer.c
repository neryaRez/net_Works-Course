#include <stdio.h> //For standard things
#include <stdlib.h>
#include <string.h>		 //memset
#include <netinet/tcp.h> //Provides declarations for tcp header
#include <netinet/ip.h>	 //Provides declarations for ip header
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pcap.h>
#include <net/ethernet.h>


// Our ICMP Header
struct icmpheader {
	unsigned char icmp_type;
	unsigned char icmp_code;
	unsigned short int icmp_chksum;
	unsigned short int icmp_id;
	unsigned short int icmp_seq;
};

// Our IP Header
struct ipheader {
	unsigned char iph_ihl:4, iph_ver:4;
	unsigned char iph_tos;
	unsigned short int iph_len;
	unsigned short int iph_ident;
	unsigned short int iph_flag:3, iph_offset:13;
	unsigned char iph_ttl;
	unsigned char iph_protocol;
	unsigned short int iph_chksum;
	struct in_addr iph_sourceip;
	struct in_addr iph_destip;
};
//Function to sniff and then spoof the packet
void got_packet(u_char *, const struct pcap_pkthdr *, const u_char *);

//Function which used to send the spoofed-packet after the sniffing and spoofing in 
//the function of "got_packet"
void send_raw_ip_packet (struct ipheader *);

//Function which used to initialize the check_sum field of each packet, that we spoof.
unsigned short in_chksum(unsigned short *buf, int length);


int main()
{

	pcap_t *handle;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;
	char filter_exp[] = "icmp";
	// The program needs to sniff from the interface of the docker-compose.
	handle = pcap_open_live("br-b90b834477c8", BUFSIZ, 1, 1000, errbuf);
	if(handle == NULL){
		printf("pcap_open_live failed\n");
		return -1;
	}
	// The program should only sniff icmp packets
	int compi = pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN);
	if(compi == -1){
		printf("pcap_compile failed\n");
		return -1;
	}

	int seti = pcap_setfilter(handle, &fp);
	if(seti == -1){
		printf("pacap_setfilter failed\n");
		return -1;
	}

	pcap_loop(handle, -1, got_packet, NULL);

	pcap_close(handle);
	
	return 0;
}
/**@got_packet
 * Stores the data of the sniffed packet, with the params of:
 * @param iph_old_packet
 * @param icmph_old_packet
 * 
 * Then, creates new packet with the same values of the sniffed-packet, but the source-ip
 * and the dest-ip are swithced, compares to the sniffed-packet.
 * 
 * The params that the function uses to store the data from the sniffed-packet, and initialize
 * the new packet, are:
 * @param iph_new_packet
 * @param icmph_new_packet
 * 
 * At the end, sends the spoofed packet which created in this function.
*/
void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *buffer){	
	
	//Stores the data from the packet which is sniffed (old packet), in order to 
	// initialize the new packet that the function creates((new packet)),
	// with the values from the old packet, exept: source-ip, dest-ip, and the icmp_type.
	struct ipheader *  iph_old_packet   = (struct ipheader *) (buffer+sizeof(struct ethhdr));
	struct icmpheader *icmph_old_packet = (struct icmpheader *)(buffer +sizeof(struct ethhdr)+ sizeof(struct ipheader));	
	
    if(icmph_old_packet->icmp_type == 8){
		
    char Buffer[1500];
    memset(Buffer, 0, 1500);
	memcpy(Buffer,buffer,strlen(buffer));

    struct ipheader *  iph_new_packet   = (struct ipheader *)(Buffer+sizeof(struct ethhdr));
    struct icmpheader *icmph_new_packet = (struct icmpheader *)(Buffer+sizeof(struct ethhdr) + sizeof(struct ipheader));	

    // Filling in the ICMP header of the new packet.
	// The type should be 0("icmp Echo-reply").
	icmph_new_packet->icmp_type = 0;
	icmph_new_packet->icmp_chksum = 0;
   	icmph_new_packet->icmp_chksum = in_chksum((unsigned short *)icmph_new_packet, sizeof(struct icmpheader));    	// Filling in the IP header
    // Filling in the IP header of the new packet.
	iph_new_packet->iph_ver = 4;
    iph_new_packet->iph_ihl = 5;
    iph_new_packet->iph_ttl = 20;
	//Initailize the source-ip of the the new packet with the dest-ip from the old packet.
	// The dest-ip of the new packet should be the source-ip of the old packet.
    iph_new_packet->iph_sourceip.s_addr =  (iph_old_packet->iph_destip.s_addr);
    iph_new_packet->iph_destip.s_addr   =  (iph_old_packet->iph_sourceip.s_addr);
	
    iph_new_packet->iph_protocol = IPPROTO_ICMP; 
    iph_new_packet->iph_len = htons(sizeof(struct ipheader) + sizeof(struct icmpheader));

	// Here we send the spoofed packet
	send_raw_ip_packet(iph_new_packet);
    }
}

void send_raw_ip_packet(struct ipheader* ip)
{
    struct sockaddr_in dest_info;
    int enable = 1;

    // Step 1: Create a raw network socket.
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    // Step 2: Set socket option.
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, 
                     &enable, sizeof(enable));

    // Step 3: Provide needed information about destination.
    dest_info.sin_family = AF_INET;
    dest_info.sin_addr = ip->iph_destip;

    // Step 4: Send the packet out.
    sendto(sock, ip, ntohs(ip->iph_len), 0, 
           (struct sockaddr *)&dest_info, sizeof(dest_info));
    close(sock);
}
/*
* Function which used to calculate the check_sum field of each packet that 
* the function "got_packet" creates.
*/
unsigned short in_chksum(unsigned short *buf, int length) {
	unsigned short *w = buf;
	int nleft = length;
	int sum = 0;
	unsigned short temp = 0;
	while(nleft > 1) {
		sum+= *w++;
		nleft -=2;
	}
	if (nleft == 1) {
		*(u_char *)(&temp) = *(u_char *)w;
		sum+=temp;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum>>16);
	return (unsigned short)(~sum);
}

