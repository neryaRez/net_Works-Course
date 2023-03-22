 #include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h> // gettimeofday()
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include <netinet/tcp.h>
#include <wait.h>

// run 2 programs using fork + exec
// command: make clean && make all && ./partb

// IPv4 header len without options
#define IP4_HDRLEN 20

// ICMP header len for echo req
#define ICMP_HDRLEN 8

// Checksum algo
unsigned short calculate_checksum(unsigned short *paddress, int len);

// The port of the tcp - connection between the watchdog and the better_ping.
#define PORT 3000   

int main(int argc , char*argv[]){
    char *args[2];
    // compiled watchdog.c by makefile
    args[0] = "./watchdog";
    args[1] = NULL;
    int status;
    int pid = fork();
    if (pid == 0)
    {
        printf("in child \n");
        execvp(args[0], args);
    }

else{
    sleep(1);
    // Create raw socket for IP-RAW (make IP-header by yourself)
    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        fprintf(stderr, "socket() failed with error: %d", errno);
        fprintf(stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }
     printf("sucess create icmp\n");
  
    // create tcp_socket to connect with the watchdog
    int tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(tcp_socket == -1)
    {
        fprintf(stderr, "socket() failed with error: %d", errno);
        fprintf(stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }  
    printf("sucess create tcp\n");
    struct sockaddr_in Watch;
    memset(&Watch, 0, sizeof(Watch));
    Watch.sin_family = AF_INET;
    Watch.sin_port = htons(PORT); 
    Watch.sin_addr.s_addr = INADDR_ANY ; 

// finishing open. now we coonect to the receiver address.
    int connectResult  = -1;
    connectResult = connect(tcp_socket, (struct sockaddr *)&Watch ,sizeof(Watch));
    if (connectResult == -1) {
        printf("connect() failed with error code : %d", errno);
        // cleanup the socket;
        close(tcp_socket);
        return -1;
    }
    
    //===================
    // ICMP header
    //===================
    struct icmp icmphdr;
    // Sequence Number (16 bits): starts at 0
    icmphdr.icmp_seq = 0;
   

    // Message Type (8 bits): ICMP_ECHO_REQUEST
    icmphdr.icmp_type = ICMP_ECHO;

    // Message Code (8 bits): echo request
    icmphdr.icmp_code = 0;

    // Identifier (16 bits): some number to trace the response.
    // It will be copied to the response packet and used to map response to the request sent earlier.
    // Thus, it serves as a Transaction-ID when we need to make "ping"
    icmphdr.icmp_id = 18;

    int times = 1;
    while (1)
    {
    
    char data[IP_MAXPACKET] = "This is the ping.\n";

    int datalen = strlen(data) + 1;

    
    icmphdr.icmp_seq ++;

    // ICMP header checksum (16 bits): set to 0 not to include into checksum calculation
    icmphdr.icmp_cksum = 0;

    // Combine the packet
    char packet[IP_MAXPACKET];

    // Next, ICMP header
    memcpy((packet), &icmphdr, ICMP_HDRLEN);

    // After ICMP header, add the ICMP data.
    memcpy(packet + ICMP_HDRLEN, data, datalen);

    // Calculate the ICMP header checksum
    icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet), ICMP_HDRLEN + datalen);
    memcpy((packet), &icmphdr, ICMP_HDRLEN);

    struct sockaddr_in dest_in;
    memset(&dest_in, 0, sizeof(struct sockaddr_in));
    dest_in.sin_family = AF_INET;

    // The port is irrelant for Networking and therefore was zeroed.
    // dest_in.sin_addr.s_addr = iphdr.ip_dst.s_addr;
    dest_in.sin_addr.s_addr = inet_addr(argv[1]);
    inet_pton(AF_INET, argv[1], &(dest_in.sin_addr.s_addr));

    
    // send start message to the watchdog- let him know that we start our ping -process:
    // he replies us - "accept".
     char* message = "start";
     int send_dog = send(tcp_socket, message, 6 , 0);
        if(send_dog == -1){
                exit(1);
        }      
     char rep1[7] = {'\0'};   
     recv(tcp_socket, rep1, 7 , 0);    

    //start to calculate times in order to see if we have connection with the destination  
    struct timeval start, end;
    gettimeofday(&start, 0);

     // Send the packet using sendto() for sending datagrams.
    int bytes_sent = sendto(sock, packet, ICMP_HDRLEN + datalen, 0, (struct sockaddr *)&dest_in, sizeof(dest_in));

    if (bytes_sent == -1)
    {
        fprintf(stderr, "sendto() failed with error: %d", errno);
        return -1;
    } 

    // Get the ping response
    bzero(packet, IP_MAXPACKET);
    socklen_t len = sizeof(dest_in);
    ssize_t bytes_received = -1;

    // the rule of "civi" and "b12" is to wait for message from the watchdog
    // if the message is "bye" - message of 3 bytes, and 10 seconds left- it's a sign
    // for us. we have to finish our program.
    int civi = 0;
    char b12[4] = {'\0'};
    while ((bytes_received = recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest_in, &len)))
    {
        civi = recv(tcp_socket, b12, 3 , MSG_DONTWAIT);
        if(civi == 3){
            printf ("get bye from the watchdog\n");
            return -1;
        }
        else if (bytes_received > 0)
        {
            // Check the IP header
            struct iphdr *iphdr = (struct iphdr *)packet;
            struct icmphdr *icmphdr = (struct icmphdr *)(packet + (iphdr->ihl * 4));
            
            ///send "sucess" to the watchdog- beacuuuse we got the ping...
            // The watchdog replies: "accept"
            char* message2 = "sucess";
            int send_dog2 = send(tcp_socket, message2, 7 , 0);
            if(send_dog2 == -1){
                exit(1);
            } 
            char rep2[7] = {'\0'};   
            recv(tcp_socket, rep2, 7 , 0); 

            // prints how many bytes we got in each ping process
            
            printf("Successfuly received one packet with %ld bytes : data length : %d , icmp header : %d , ip header : %d \n", bytes_received, datalen, ICMP_HDRLEN, IP4_HDRLEN);
            times++;

            break;
        }
    }
    gettimeofday(&end, 0);

    // for sure to know if 10 seconds left
    if((end.tv_sec - start.tv_sec)>=10) return -1;

     // prints the icmp_seq, the packet-id, and the RTT
    printf ("The sequence number is: %d\n", icmphdr.icmp_seq);
    
    char reply[IP_MAXPACKET];
    memcpy(reply, packet + ICMP_HDRLEN + IP4_HDRLEN, datalen);
   

    float milliseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
    unsigned long microseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec);
    printf("RTT: %lf milliseconds %ld microseconds\n", milliseconds, microseconds);
    printf("The packet came from %s all the way to %s\n\n", argv[1], "10.0.2.15");
 
    }

    // Close the raw socket descriptor.
    close(sock);
    close(tcp_socket);
    
}
    return 0 ;
}

// Compute checksum (RFC 1071).
unsigned short calculate_checksum(unsigned short *paddress, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = paddress;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *((unsigned char *)&answer) = *((unsigned char *)w);
        sum += answer;
    }

    // add back carry outs from top 16 bits to low 16 bits
    sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
    sum += (sum >> 16);                 // add carry
    answer = ~sum;                      // truncate to 16 bits

    return answer;
}