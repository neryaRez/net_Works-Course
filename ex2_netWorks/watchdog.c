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
#include <netinet/tcp.h>
#include <signal.h>


#define PORT 3000
int main()
{

    //create the tcp-server to connect with the better_ping(tcp-client)
    int listeningSocket = -1;
    listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    if (listeningSocket == -1) {
        printf("Could not create listening socket : %d", errno);
        return -1;
    }

    printf("create tcp_socket\n");

    struct sockaddr_in Watch_Adress;
    memset(&Watch_Adress, 0, sizeof(Watch_Adress));
    Watch_Adress.sin_family = AF_INET;
    Watch_Adress.sin_port = htons(PORT); 
    Watch_Adress.sin_addr.s_addr = INADDR_ANY ;

    //bind() function
    int bindResult = bind(listeningSocket, (struct sockaddr *)&Watch_Adress, sizeof(Watch_Adress));
    if (bindResult == -1) {
        printf("Bind failed with error code ");
    // close the socket
        close(listeningSocket);
        return -1;
    }
    printf("bind\n");
    //listen function//
    int listenResult = listen(listeningSocket, 3);
    if (listenResult == -1) {
        printf("listen() failed with error code ");
    // close the socket
        close(listeningSocket);
        return -1;
    }
    printf("listen\n");

    // accepting the connection from the sender.
    // accept() returns the MSGsocket- to coonect and get data from the sender.
    struct sockaddr_in MSGAddress;  
    socklen_t MSGAddressLen = sizeof(MSGAddress);

   
    memset(&MSGAddress, 0, sizeof(MSGAddress));
    MSGAddressLen = sizeof(MSGAddress);
    int MSGSocket = accept(listeningSocket, (struct sockaddr *)&MSGAddress, &MSGAddressLen);
    if (MSGSocket== -1) {
        printf("listen failed with error code : %d", errno);
        // close the sockets
        close(listeningSocket);
        return -1;
        } 
    printf("accept\n");    
    float timer = 0;
    struct timeval start , end;

    /////// The ping - pong process with the better_ping///////////
    while (1)
    {
        ///receive "start" from the better_ping, and send to him : "accept";
        
        char rep_start[6] = {'\0'};
        recv(MSGSocket, rep_start , 6 , 0);
        char* acc = "accept";
        send(MSGSocket , acc, 7 , 0);

        
       /// if everything is going well then , I probably will get a message from the
       // better_ping that will tell me: "sucess". Then, I know the ping-process is fine
        char rep_suc[7] = {'\0'};
        // while I don't get "sucess" message from the better_ping and the time of 10 seconds
        // is over yet, keep waiting
        while (timer<10000 && strlen(rep_suc)==0)
        {
        gettimeofday(&start , NULL);

        recv(MSGSocket, rep_suc , 7 , MSG_DONTWAIT);

        gettimeofday(&end , NULL);

        timer +=((end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f);

        }
       // if 10 seconds left break from the while loop - in order to say goodbye to the
       //better_ping.
       //otherwise, send to the better_ping a message of "accept", because he sent a message
       //of "sucess".
        if(timer == 10000) break;
        char* acc2 = "accept";
        send(MSGSocket , acc2 , 7 , 0);
        //timer is zero again.
        timer = 0;

    }
    
    ///////send -> "bye" which is our "timeout-message" to the better_ping.
    char* b1 = "bye";
    send(MSGSocket, b1, 3 , 0);
    close(MSGSocket);
    close(listeningSocket);

}