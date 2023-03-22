# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <errno.h>
# include <fcntl.h>
# include <unistd.h>
# include <ctype.h>



 #define PORT 5555

  /**@SendFile
  * 
  * function support to send the file.
  * The function gets:
  * c = a string that we want to send
  * len= the length of the string
  * socket_1 the socket that we use to send this message
  @SendFile
 */

  void SendFile(char* c , int len , int socket_1 ){
  
 int byteSent = send(socket_1, c, len, 0 );
   
    if (byteSent == -1) {
        printf("send() failed with error code : %d", errno);
    } else if (byteSent == 0) {
        printf("peer has closed the TCP connection prior to send().\n");
    } 
    
    else{
      double pr = (byteSent/len)*100;
      int pr1 = (int) pr;
      char* pr2 = "%";
      printf("sent %d (%d %s) bytes from the required %d.\n",byteSent, pr1 ,pr2, len);
      
    }
  }


/**@Autentication
 * support function to see if the string of our authentication
 * of our id's is correct.
 * 
*/
  int Authentication (char* s ){
    char* Auto = "1111110111111100";
    for(int i = 0; i < 16; i++ )
    {
        if(s [i] != Auto[i])
        {
            return 0;
        }
    }
    return 1;
  }


/**@CheckAuthentication
 * this is the function when we get our authentication message from our receiver.
 * 
*/
  void CheckAuthentication( int socket_2) {
        char bufferReply[17] = {'\0'};
        int bytesReceived = recv(socket_2, bufferReply, 17 , 0);
        if (bytesReceived == -1) {
         printf("autentication() failed with error code : %d \n", errno);
        } else if (bytesReceived == 0) {
        printf("peer has closed the TCP connection prior to recv().\n");
        } else if(Authentication(bufferReply) == 0){
         printf("The Receiver didn't got the correct file. \n");
        }else{
         printf("the Authentication is correct. \n");
        }
  }

 int main(){

// a variable who count the times when we send the text
    int times = 1;
// a pointer to the file who's opened in a write mode 
    FILE* file;
    file  =  fopen("file.txt" , "r");
    // we open the soket
    int sock = socket(AF_INET , SOCK_STREAM , 0);
      if (sock == -1) {
        printf("Could not create socket ");
        return -1;
    }
    else{
        printf("The Socket has Succefuly Created \n" );
    }
    
    struct sockaddr_in ReceiverAdress;
    memset((char *)&ReceiverAdress, 0, sizeof(ReceiverAdress));
    ReceiverAdress.sin_family = AF_INET;
    ReceiverAdress.sin_port = htons(PORT); 
    ReceiverAdress.sin_addr.s_addr = INADDR_ANY ; 

// finishing open. now we coonect to the receiver address.
    int connectResult = connect(sock , (struct sockaddr *)&ReceiverAdress , sizeof(ReceiverAdress));
    if (connectResult == -1) {
        printf("connect() failed with error code : %d", errno);
        // cleanup the socket;
        close(sock);
        return -1;
    }
     else{
         printf("The Connection has Succefully eshtablished \n");
     }
    // start counting our rounds.
    // Each round is a time when we send the file.
    printf("\n");
    printf("round %d\n", times);
    // now we calculate the size of our file and keep his data in 2 strings: BUUFER1 and BUFFER2
    long FileSize;
    fseek(file , 0 , SEEK_END);
    FileSize =  ftell(file);
    rewind(file);
    char* BUFFER1 = malloc((FileSize/2)+1);
    BUFFER1 [FileSize/2] = '\0';

    for(int i = 0; i < (FileSize/2) ; i++ )
    {
     char a1 = fgetc(file);
     BUFFER1 [i] = a1 ;
    }

    char* BUFFER2 = malloc((FileSize/2)+1);
    BUFFER2 [FileSize/2] = '\0';
    for(int i = 0; i < (FileSize/2) ; i++ )
    {
     char a2 = fgetc(file);
     BUFFER2 [i] = a2 ;
    }
     // now we have our two strings. 
    // so we can send them with our parameters. we can also check our authentication 
    
    
    int messageLen = strlen(BUFFER1) ;
    
    printf("Sending the First Part \n");
    
    SendFile(BUFFER2 , messageLen , sock );
    CheckAuthentication(sock);
    
    char Change[5] = {'r' , 'e' , 'n' , 'o' , '\0'};
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, Change , 5) != 0)
    {
        printf("Error of  changing the cc Algorethim : %d \n" , errno);
        return -1;
    }
   
    int messageLen2 = strlen(BUFFER2);
    printf("Sending the Second Part \n");

    SendFile(BUFFER2 , messageLen2 , sock );

    printf("\n");
    // That was the first time when we sent the file. now it's all about the descision of the user.
    // So, this is our while-loop. We can send our file as long as the user wants.
  
    while(1){
      printf("Do You Want to send the file again? if yes, Type Y.\notherwise, type N \n");
      char y ;
      scanf(" %c" , &y);

      if(y == 'N'){
        char end [5] = {'e' , 'x' , 'i' , 't' ,'\0'};
        int byteSent = send(sock, end , 4 , 0);
        printf ("send an exit message\n");
        break;
        }
      else if(y == 'Y')
      {
        char cont [9] = {'c' , 'o' , 'n' , 't' , 'i' , 'n'  , 'u' , 'e' , '\0'};
        int byteSent = send(sock, cont , 8 , 0);
        char con[7] = {'\0'};
        int Rec = recv (sock ,con ,6 ,0);
        // Here is the next time when we send our message
        times++;
        printf("\n");
        printf("round %d\n", times);
       
       // We change our cc algo
       // Then, we send our 2 parts of file according to our orders.
       // Each part with his cc algo.
      
        char Change2[6] = {'c' , 'u' , 'b' , 'i' ,'c' ,'\0'};
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, Change2 , 6) != 0)
        {
            printf("Error of  changing the cc Algorethim : %d \n" , errno);
           return -1;
        }


        printf("Sending the First Part \n");
        SendFile(BUFFER1 , messageLen , sock);


        CheckAuthentication(sock);


         char Change3[5] = {'r' , 'e' , 'n' , 'o' , '\0'};
         if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, Change3 , 5) != 0)
         {
           printf("Error of  changing the cc Algorethim : %d \n" , errno);
           return -1;
         }



         printf("Sending the Second Part \n");
         SendFile(BUFFER2 , messageLen2 , sock);
      }
    }
  close(sock);
  return 0;
 }