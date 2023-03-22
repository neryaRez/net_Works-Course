#include <stdio.h> //For standard things
#include <stdlib.h>
#include <string.h>		 
#include <netinet/tcp.h> //Provides declarations for tcp header
#include <netinet/ip.h>	 //Provides declarations for ip header
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pcap.h>
#include <net/ethernet.h>

//Funcions which used to print the details of each packet to the txt file.
void print_tcp_packet(u_char *, const struct pcap_pkthdr *, const u_char *);
void PrintData(const u_char *, int);
//Pointer to the txt file
FILE *file;
//Structs to point and then print, the source-ip and the dest-ip more easily.
struct sockaddr_in source, dest;
//Special struct to print the special-header of each packet that we sniff from EX2 runing
struct data_head
{
	uint32_t time_unix;
	uint16_t total;
	uint16_t flags;
	uint16_t cache;
};

int main()
{
	//Sets up the sniffer program with filter of tcp-protocol
	pcap_t *handle;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;
	char filter_exp[] = "tcp";

	//The program needs to sniff from the loopback interface - "lo"
	handle = pcap_open_live("lo", BUFSIZ, 1, 1000, errbuf);
	if(handle==NULL){
		printf("pcap_open_live failed\n");
		return -1;
	}
	// Set-up and install the tcp-filter
	int compi = pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN);
	if(compi==-1){
		printf("pcap_compile failed\n");
		return -1;
	}

	int seti = pcap_setfilter(handle, &fp);
	if(seti==-1){
		printf("pcap_setfilter failed\n");
		return -1;
	}
	// Create the txt file to print.
	file = fopen("316276989_212689475.txt", "w");
	if(file == NULL){
		printf("couldn't open the file\n");
	}
	// Sniffing the packets. "print_tcp_packet" is used to print the details of each packet,
	// that we sniff.
	pcap_loop(handle, -1, print_tcp_packet, NULL);

	pcap_close(handle);
	fclose(file);
	return 0;
}
/**@print_tcp_packet
 *  Sets up our structure-pointers to point on the fields of each packet.
 * 	iphdr  - is used to print the data-fields of each packet's header.
 *	tcphdr - is used to print the data-fields of each packet's tcp-header
 *	p      - is used to print the special data fields from the EX2-header, of each packet.
*/
void print_tcp_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *buffer)
{
	fprintf(file, "                 NEW TCP PACKET:           \n\n");

	unsigned short iphdrlen;
	struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ethhdr));
	iphdrlen = iph->ihl * 4;
	struct tcphdr *tcph = (struct tcphdr *)(buffer + iphdrlen + sizeof(struct ethhdr));
	int header_size = sizeof(struct ethhdr) + iphdrlen + tcph->doff * 4;
	struct data_head *p = (struct data_head *)(buffer + header_size);
	int whole_size = header->len;

	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = iph->saddr;
	memset(&dest, 0, sizeof(dest));
	dest.sin_addr.s_addr = iph->daddr;

	fprintf(file, "   Source IP            : %s\n",  inet_ntoa(source.sin_addr));
	fprintf(file, "   Destination IP       : %s\n",  inet_ntoa(dest.sin_addr));
	fprintf(file, "   Source Port          : %u\n",  ntohs(tcph->source));
	fprintf(file, "   Destination Port     : %u\n",  ntohs(tcph->dest));
	fprintf(file, "   Time_stamp           : %u\n",  ntohl(p->time_unix));
	fprintf(file, "   Total_length of data : %u\n",  ntohs(p->total));
	fprintf(file, "   Cashe_flag           : %u\n", (ntohs(p->flags) & 0x1000)>> 12);
	fprintf(file, "   Step_flag            : %u\n", (ntohs(p->flags) & 0x800 )>> 11);
	fprintf(file, "   Type_flag            : %u\n", (ntohs(p->flags) & 0x400 )>> 10);
	fprintf(file, "   Status               : %u\n",  ntohs(p->flags) & 0x3ff);
	fprintf(file, "   Cache_control        : %u\n",  ntohs(p->cache));
	fprintf(file, "\n");
	fprintf(file, "                        DATA                         ");
	fprintf(file, "\n");


	PrintData(buffer + header_size, whole_size - header_size);

	fprintf(file, "\n\n");
	fflush(file);
}
/**@printData
 * Prints the data from the application-layer, after printing the data from 
 * the headers(ip header, tcp header, and data header),in the function of "print_tcp_packet".
*/
void PrintData(const u_char *data, int Size)
{
	int i;
	for (i = 0; i < Size; i++)
	{

		if (i % 16 == 0)
			fprintf(file, "\n");

		fprintf(file, " %02X", (unsigned int)data[i]);
	}
}
