#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

# define RECEIVERPORT 5555
# define  Auto  "1111110111111100"



/**@SendAuto
 * Function that sends the authontication to the sender.
*/
  void SendAuto( char* aut  , int len , int socket_1  ){
    int bytesSent = send(socket_1, aut , len, 0);
    if (bytesSent == -1) {
        printf("send() Authentication has failed with error code : %d", errno);
    } else if (bytesSent == 0) {
        printf("peer has closed the TCP connection prior to send().\n");
    } else if (bytesSent < len) {
        printf("sent only %d bytes from the required %d.\n", len, bytesSent);
    }else{
        printf("Authentication  was successfully sent.\n");
    }
  }
/**@Recieved
 * The function returns the time it takes to receiver, to get the whole message- therefore it's
 * in the type of long
 * The function gets:
 * sizefile - the length of the file. The function divides this length by 2- in order to get just 1 
 * half of the file.
 * sock - the socket, that function uses to get data.
 * Listensock - the origional socket of the receiver.
 * 
 * if the function recv() that we use, in this function returns us 0, then we return 0, and 
 * this is a sign, that the sender wants to stop sending data.
 * 
*/

  long Recieved (long sizefile , int sock , int listenSock) 
  {
    // buffer- the buffer that we use, to get the message ,from the sender.
        char buffer[(sizefile/2)+1];
        memset(buffer, '\0', (sizefile/2)+1);
        // sum_recv- a variable to sum all of the bytes, that we get during all of our loop,
        // until we get the whole message. 
        int sum_recv = 0;
        //structutures that we use, to calculate the amount of the time, it takes to get the whole
        // message in micro-seconds.
        struct timeval start,end;
        gettimeofday(&start , NULL);
        while (sum_recv < sizefile/2)
        {
           //bytesReceived - how many bytes we get, in one chunk
           //sum-recv - how many bytes, we get so far.
            int bytesReceived = recv(sock, buffer, (sizefile/2)+1, 0);
              
            if(bytesReceived == (sizefile/2) - 1)
            {
                sum_recv+=bytesReceived;
                printf (" bytesReceived  =   %d  , sum_recv =  %d \n " , bytesReceived  , sum_recv);
                break;
            }
            else{
                sum_recv+=bytesReceived;
                printf (" bytesReceived  =   %d  , sum_recv =  %d \n " , bytesReceived  , sum_recv);
            }
          
           
        }
        //this is the calculation of time:
         gettimeofday(&end , NULL);
         long TimeSpend = (end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec);
    
        if (sum_recv == -1) {
            printf("recv failed with error code : %d", errno);
            // close the sockets
            close(listenSock);
            close(sock);
        }
        //calculate and check- how many percent of the message we have missed:
        else
        {
            double pr = ((double)(sum_recv)/(double)(sizefile/2))*100;
            int pr1 = (int) pr;
            char* percent= "%";
        
            printf("received %d (%d %s)  bytes from the required %ld.\n",sum_recv, pr1 , percent,(sizefile/2));
            
        }  
         //returns the time-calcution, to get the whole message.
        return TimeSpend;
  }

  
int main(){
  // times - count the times we get all of our file.
    int times=1;
    // 2 long-variables that we use to sum the times it takes to get the messages from 
    // the receiver.
    long sumFirstPartTimes = 0 ;
    long sumSocendpartTimes = 0;
 // create the socket:
    int listeningSocket = -1;
    listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    if (listeningSocket == -1) {
        printf("Could not create listening socket : %d", errno);
        return -1;
    }


   struct sockaddr_in ReceiverAdress;
    memset((char *)&ReceiverAdress, 0, sizeof(ReceiverAdress));
    ReceiverAdress.sin_family = AF_INET;
    ReceiverAdress.sin_port = htons(RECEIVERPORT); 
    ReceiverAdress.sin_addr.s_addr = INADDR_ANY ;

    //bind() function
    int bindResult = bind(listeningSocket, (struct sockaddr *)&ReceiverAdress, sizeof(ReceiverAdress));
    if (bindResult == -1) {
        printf("Bind failed with error code : %d", errno);
    // close the socket
        close(listeningSocket);
        return -1;
    }
    printf("Bind() success\n");

    int listenResult = listen(listeningSocket, 3);
    if (listenResult == -1) {
        printf("listen() failed with error code : %d", errno);
    // close the socket
        close(listeningSocket);
        return -1;
    }
    printf("listen() success\n");

    // accepting the connection from the sender.
    // accept() returns the MSGsocket- to coonect and get data from the sender.

    printf("Waiting for incoming TCP-connections...\n");
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

        printf("The Sender connection accepted\n");

        //calculate the  length of the two messages from Sender___  two parts of the file. 
        FILE* file;
        file  =  fopen("file.txt" , "r");
        long FileSize;
        fseek(file , 0 , SEEK_END);
        FileSize =  ftell(file);
        rewind(file);


        printf("\n");
        printf("round %d\n", times);
        
    //the work on the dynamic - long[] we use to store the times it takes to get the messages
    //from the sender.
        int index = 0;
        int capacity = 2;
        long* ms = (long*)calloc(capacity , sizeof(long));

        printf("Receiveng the First Part  \n");
        long t =  Recieved(FileSize , MSGSocket , listeningSocket);
        sumFirstPartTimes += t;
        SendAuto(Auto , 17 , MSGSocket);
        if(index== capacity)
        {
            ms = realloc(ms , (2*capacity)*sizeof(long));
            capacity *=2 ;
        }
        ms[index] = t ;
        index ++;
        
        char Change[5] = {'r' , 'e' , 'n' , 'o' , '\0'};
        if (setsockopt(MSGSocket, IPPROTO_TCP, TCP_CONGESTION, Change , 5) != 0)
        {
        printf("Error of  changing the cc Algorethim : %d \n" , errno);
        return -1;
        }
       
        printf("Receiveng the  Second Part  \n");
        t = Recieved(FileSize , MSGSocket , listeningSocket);
        sumSocendpartTimes += t;
         if(index== capacity)
        {
            ms = realloc(ms , (2*capacity)*sizeof(long));
            capacity *=2 ;
        }
        ms[index] = t ;
        index ++;
        printf("\n");
        


        
        while(1){

        
          char buffer3[9];
          memset(buffer3, '\0', 9);
          int bytesReceived = recv(MSGSocket, buffer3, 9, 0);
          if (bytesReceived == -1) {
              printf("recv failed with error code : %d", errno);
              // close the sockets
              close(listeningSocket);
              close(MSGSocket);
              return -1;
              }
            //if we got an exit message then, the receiver knows that the sender wants to stop the connection , the we print all the times to get the
            //first part  and the socend part and the average to get the parts.
           else  if(bytesReceived == 4 ){
            printf("get an exit message \n");
            printf ("The Times of the first part : ");
            for(int i =0 ; i < index ; i+=2)
            {
                printf("%ld  ," , ms[i]);
            }
            printf("\n ");
            printf ("The Times of the second part : ");
            for(int i =1 ; i < index ; i+=2)
            {
                printf("%ld  ," , ms[i]);
            }
            printf("\n ");
            printf("the first part average %ld \n" , (sumFirstPartTimes/times));
            printf("the  socend part average %ld \n" , (sumSocendpartTimes/times));
            
            break;
           }
           else{
        // that mean we got message to countinue so we will countinue to get received the file.
        //prints which number of time- that we have to get the whole message - we are now. 
            times++;
            printf("round %d  \n" , times );
            char *p = "accept";
            int s = send(MSGSocket ,  p , 6 , 0);
            //returns back the cc algo to cubic
            char Change2[6] = {'c' , 'u' , 'b' , 'i' ,'c' ,'\0'};
            if (setsockopt(MSGSocket, IPPROTO_TCP, TCP_CONGESTION, Change2 , 6) != 0)
            {
             printf("Error of  changing the cc Algorethim : %d \n" , errno);
             return -1;
            }

        // receiving the First part, and calculate how much time it took to get this part
        // from the sender. 
             
             printf("Receiveng the First Part  \n");
             t =  Recieved(FileSize , MSGSocket , listeningSocket);
        // sum-up the amount of times it took the get the second part, in order to calculate the
        // average, at the end of the program.
            sumFirstPartTimes += t;
            SendAuto(Auto , 17 , MSGSocket);
        // saving the time it takes to get the first part in the dynamic long[].
            if(index== capacity)
            {
              ms = realloc(ms , (2*capacity)*sizeof(long));
              capacity *=2 ;
            }
            ms[index] = t ;
            index ++;
            
      
        //changing the cc-algo   
            char Change3[5] = {'r' , 'e' , 'n' , 'o' , '\0'};
            if (setsockopt(MSGSocket, IPPROTO_TCP, TCP_CONGESTION, Change3 , 5) != 0)
            {
             printf("Error of  changing the cc Algorethim : %d \n" , errno);
             return -1;
            }
        // receiving the second part, and calculate how much time it took to get this part
        // from the sender. 

            printf("Receiveng the  Second Part  \n");
             t = Recieved(FileSize , MSGSocket , listeningSocket);
            sumSocendpartTimes += t;
            if(index== capacity)
            {
              ms = realloc(ms , (2*capacity)*sizeof(long));
              capacity *=2 ;
            }
            ms[index] = t ;
            index ++;
            printf("\n");
           }
        }
// at the end, we should close and set free all of those variables:       
close(listeningSocket);
close(MSGSocket);
fclose(file);
free(ms);
return 0;
}