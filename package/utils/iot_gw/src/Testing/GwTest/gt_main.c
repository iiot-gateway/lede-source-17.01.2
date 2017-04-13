// ====================================================================
//  Simple udp client
//  Silver Moon (m00n.silv3r@gmail.com)
// ====================================================================

/** \file
 * \section gatewaytest Gateway Discovery Testing
 * \brief Gateway Discovery Test
 * Finds the Gateway by sending a UDP broadcast message to the detection port
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
 
#define SERVER   "192.168.1.122"
#define BUFLEN   64               // Max length of buffer
#define PORT     1999             // The port on which to send data
 
void die(char *s)
{
    perror(s);
    exit(1);
}

/**
 * \brief Gateway Discovery Test's main entry point: opens a UDP socket, sends the
 * broadcast message to all Gateways, and prints the response(s)
 */
int main(void)
{
    struct sockaddr_in si_other;
    int s, slen=sizeof(si_other), index;
    char buf[BUFLEN];
    char message[BUFLEN];
 
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
 
    while(1)
    {
        printf("Enter message : ");
        fgets(message, (BUFLEN - 2), stdin);
        index = strlen(message);
        message[index++] = '\n';
        message[index++] = '\0';
         
        //send the message
        if (sendto(s, message, strlen(message), 0,
                   (struct sockaddr *) &si_other, slen)==-1)
        {
            die("sendto()");
        }
#if 1         
        // receive a reply and print it
        // clear the buffer by filling null
        memset(buf,'\0', BUFLEN);
        //try to receive some data, this is a blocking call
        if ( recvfrom( s, buf, BUFLEN, 0,
                       (struct sockaddr *) &si_other,
                       (socklen_t *)&slen) == -1 ) {
            die("recvfrom()");
        }
         
        puts(buf);
#endif
    }
 
    close(s);
    return 0;
}
