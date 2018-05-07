//Jeffrey Herbst
//CS 4220
//PROJ 1
//UDP Client
//
//First ACKSIZE chars are the hex representation of the packet number

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <fcntl.h>

#include "myDefs.h"

void usage(char *pname)
{
    fprintf(stderr, "Usage: %s [host] [filename] [protocol]\n", pname);
    fprintf(stderr, "1:Stop and Wait, 2:Go Back N, 3:Selective Repeat\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int     data_size = DEFLEN, port = SERVER_UDP_PORT;
	int     c, i, j, sd, server_len, protocol, bytes, num_frames;
    char    *pname, *host, *filename, rbuf[MAXLEN], sbuf[MAXLEN];
    struct  hostent         *hp;
    struct  sockaddr_in     server;
    struct  timeval         start, end;
    struct  packet          in_packet, out_packet;
	unsigned long address;

    //read inputs
    switch(argc)
    {
    case 4:
        pname = argv[0];
        host = argv[1];
        filename = argv[2];
        protocol = atoi(argv[3]);
        break;
    default:
        usage(argv[0]);
    }

    FILE *client_log;
    client_log = fopen("client_log.txt", "w+");

    //Create socket
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
    {
        fail("Can't create a socket\n");
    }

    bzero((char *)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    //get servers IP
    if ((hp = gethostbyname(host)) == NULL) 
    {
        fail("Can't get server's IP address\n");
    }
    
    bcopy(hp->h_addr, (char *) &server.sin_addr, hp->h_length); 

    
    //send name of file
    server_len = sizeof(server);
    strcpy(out_packet.data, filename); 
    out_packet.packet_numb = 1;
    out_packet.total_packets = 1;
    out_packet.data_length = strlen(filename);
    out_packet.ACK = FALSE;
    fprintf(client_log, "Asking for filename: %s\n", filename);   
    if (sendto(sd, &out_packet, sizeof(out_packet), 0, (struct sockaddr *) &server, server_len) == -1) 
    {
        fail("sendto error\n");    
    }
    
    //open write file
    char *filename_path = malloc(strlen(argv[2]) + 10);
    strcpy(filename_path, "./subdir/");
    strcat(filename_path, filename);
    fprintf(stderr, "\n%s\n", filename_path);
    
     
    int fd = open(filename_path, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR| S_IRGRP | S_IWGRP);
    if(fd < 0) fail("Failed to open file");

    switch(protocol)
    {
    case STOP_AND_WAIT:
        //first packet get info
        if (recvfrom(sd, &in_packet, sizeof(in_packet), 0, (struct sockaddr *)&server, &server_len) < 0) 
        {
            fail("recvfrom error\n");
        }
        fprintf(client_log, "<-- RECV #%i\n", in_packet.packet_numb);
        num_frames = in_packet.total_packets;
        write(fd, in_packet.data, in_packet.data_length);
        //ack it
        
        out_packet.packet_numb = in_packet.packet_numb;
        out_packet.total_packets = in_packet.total_packets;
        out_packet.data_length = 0;
        out_packet.ACK = TRUE;
        if(sendto(sd, &out_packet, sizeof(out_packet), 0, (struct sockaddr *)&server, server_len) < 0)
        {
            fail("send ack error");
        }
        else
        {
            fprintf(client_log, "--> ACK  #%i\n", out_packet.packet_numb);
        }

        while(TRUE)
        {
    	    //get packet
            if (recvfrom(sd, &in_packet, sizeof(in_packet), 0, (struct sockaddr *)&server, &server_len) < 0) 
            {
                fail("recvfrom error\n");
            }
            
            out_packet.packet_numb = in_packet.packet_numb;
            fprintf(client_log, "<-- RECV #%i\n", in_packet.packet_numb);
            
            //write to file
            write(fd, in_packet.data, in_packet.data_length);
            
            out_packet.packet_numb = in_packet.packet_numb;
            fprintf(client_log, "--> ACK  #%i\n", out_packet.packet_numb);
            //send ack
            if (sendto(sd, &out_packet, sizeof(out_packet), 0, (struct sockaddr *) &server, server_len) < 0)
            {
                fail("ACK sendto error");
            }
            
            //check for final packet
            if(in_packet.packet_numb == in_packet.total_packets)
            {    
                fprintf(stdout, "File received\n");
                break;
            }
        }
        break;
    case GO_BACK_N:
        //first packet get info
        if (recvfrom(sd, &in_packet, sizeof(in_packet), 0, (struct sockaddr *)&server, &server_len) < 0) 
        {
            fail("recvfrom error\n");
        }
        fprintf(client_log, "<-- RECV #%i\n", in_packet.packet_numb);
        num_frames = in_packet.total_packets;
        write(fd, in_packet.data, in_packet.data_length);
        //ack it
        
        out_packet.packet_numb = in_packet.packet_numb;
        out_packet.total_packets = in_packet.total_packets;
        out_packet.data_length = 0;
        out_packet.ACK = TRUE;
        if(sendto(sd, &out_packet, sizeof(out_packet), 0, (struct sockaddr *)&server, server_len) < 0)
        {
            fail("send ack error");
        }
        else
        {
            fprintf(client_log, "--> ACK  #%i\n", out_packet.packet_numb);
        }

        while(TRUE)
        {
    	    //get packet
            if (recvfrom(sd, &in_packet, sizeof(in_packet), 0, (struct sockaddr *)&server, &server_len) < 0) 
            {
                fail("recvfrom error\n");
            }
            
            out_packet.packet_numb = in_packet.packet_numb;
            fprintf(client_log, "<-- RECV #%i\n", in_packet.packet_numb);
            
            //write to file
            write(fd, in_packet.data, in_packet.data_length);
            
            out_packet.packet_numb = in_packet.packet_numb;
            fprintf(client_log, "--> ACK  #%i\n", out_packet.packet_numb);
            //send ack
            if (sendto(sd, &out_packet, sizeof(out_packet), 0, (struct sockaddr *) &server, server_len) < 0)
            {
                fail("ACK sendto error");
            }
            
            //check for final packet
            if(in_packet.packet_numb == in_packet.total_packets)
            {    
                fprintf(stdout, "File received\n");
                break;
            }
        }
        break;
    case SELECTIVE_REPEAT:
        fprintf(stdout, "Selective Repeat Not Implemented\n");
    default:
        usage(pname);     
    }        
    close(fd); 
    close(sd);
    return(EXIT_SUCCESS);
}
