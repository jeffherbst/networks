/* Echo server using UDP */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>

#include "myDefs.h"

void usage(char *pname)
{
    fprintf(stderr, "Usage: %s [droprate] [protocol]\n", pname);
    fprintf(stderr, "1:Stop and Wait, 2:Go Back N, 3:Selective Repeat\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int 	bytes, sd, client_len, port = SERVER_UDP_PORT, protocol, droprate, num_frames, retries=0, gotACK=TRUE;
    int     drop = FALSE, fd, size, go_back_to = 0, window_numb = 10;
	char    but[MAXLEN], *pname;
	struct 	sockaddr_in 	server, client;
    struct  timeval         timeout;
    struct  packet          out_packet;
    struct  packet          in_packet;
	
    switch(argc) 
    {
	case 3:
		pname = argv[0];
        droprate = atoi(argv[1]);
        protocol = atoi(argv[2]);    
		break;
	default:
        usage(argv[0]);
	}
	
    //clear file log	
    FILE *server_log;
    server_log = fopen("server_log.txt", "w+");
    fclose(server_log);

    while (TRUE) //server loop
    {
	    server_log = fopen("server_log.txt", "a+");
        fprintf(server_log, "\n============================================================\n");
        fprintf(server_log, "                      New Connection                        \n"); 
        fprintf(server_log, "============================================================\n");

        fprintf(stdout, "\n============================================================\n");
        fprintf(stdout, "                   Waiting for Connection                   \n"); 
        fprintf(stdout, "============================================================\n");

        /* Create a datagram socket */
	    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		    fail("Can't create a socket");
	    }


	    /* Bind an address to the socket */
	    bzero((char *)&server, sizeof(server));
    	server.sin_family = AF_INET;
    	server.sin_port = htons(port);
    	server.sin_addr.s_addr = htonl(INADDR_ANY);
    	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
    		fail("Can't bind socket");
    	}

        switch(protocol)
        {
        case STOP_AND_WAIT: 
            //receive filename
            fprintf(stdout, "Stop and Wait\n");
            client_len = sizeof(client);
	    	if ((recvfrom(sd, &in_packet, sizeof(in_packet), 0,(struct sockaddr *)&client, &client_len)) < 0) 
            {
		        fail("Can't receive datagram\n");
		    }
            
            fd = open(in_packet.data, O_RDONLY);
            
            if (fd < 0) fail("Failed to open file"); 
            
            //initilize packet
            size = fsize(in_packet.data);
            num_frames = calculate_num_frames(size, MAXLEN);
            out_packet.packet_numb = 0;
            out_packet.total_packets = num_frames;
            out_packet.ACK = FALSE;
            fprintf(stdout, "Frames: %d\nFrame Size: %d Bytes\nTotal Size: %d Bytes\n", num_frames, MAXLEN, size);

            //set timeout
            timeout.tv_sec = TIMEOUT_S;
            timeout.tv_usec = TIMEOUT_US;
            if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
                fail("Set sock opt");

            //send file
            while(TRUE)
            {
                //make packet
                if(gotACK != FALSE)
                {
                    retries = 0;
                    out_packet.packet_numb += 1;
                    bytes = read(fd, out_packet.data, MAXLEN);
                    if(bytes <= 0) break;
                    out_packet.data_length = bytes;
                }	    	    

                //send packet
                drop = drop_packet(droprate);
                if(drop == FALSE || out_packet.packet_numb == 1)//don't drop first packet
                {
                    if (sendto(sd, &out_packet, sizeof(out_packet), 0, (struct sockaddr *)&client, client_len) < 0) 
                    {
		                fail("Failed sendto");
		            }
                    else
                    {
                        fprintf(server_log, "<-- Send  #%i\n", out_packet.packet_numb);
                    }
                }

                //wait for ack               
                if(recvfrom(sd, &in_packet, sizeof(in_packet), 0, (struct sockaddr *)&client, &client_len) < 0)
                {
                    if(retries < MAX_RETRIES)
                    {
                        retries++;
                        fprintf(stdout, "Retry... %i\n", out_packet.packet_numb);
                        fprintf(server_log, "NO ACK, RETRY %i\n", out_packet.packet_numb);
                        gotACK = FALSE;
                        continue;
                    }
                    else
                    {
                        fail("Max Retries timeout");
                    }
                }
                else
                {
                    gotACK = TRUE;
                    fprintf(server_log, "--> ACK   #%i\n", out_packet.packet_numb);
                }
            }
            close(fd);
            close(sd);   
        break;
        case GO_BACK_N:
            fprintf(stdout, "Go Back N\n");
            //receive filename
            client_len = sizeof(client);
	    	if ((recvfrom(sd, &in_packet, sizeof(in_packet), 0,(struct sockaddr *)&client, &client_len)) < 0) 
            {
		        fail("Can't receive datagram\n");
		    }
            
            fd = open(in_packet.data, O_RDONLY);
            
            if (fd < 0) fail("Failed to open file"); 
            
            //initilize packet
            size = fsize(in_packet.data);
            num_frames = calculate_num_frames(size, MAXLEN);
            out_packet.packet_numb = 0;
            out_packet.total_packets = num_frames;
            out_packet.ACK = FALSE;
            fprintf(stdout, "Frames: %d\nFrame Size: %d Bytes\nTotal Size: %d Bytes\n", num_frames, MAXLEN, size);

            //set timeout
            timeout.tv_sec = TIMEOUT_S;
            timeout.tv_usec = TIMEOUT_US;
            if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
                fail("Set sock opt");
            //MSG_DONTWAIT
            //send file
            while(TRUE)
            {
                //make packet
                if(gotACK != FALSE)
                {
                    retries = 0;
                    out_packet.packet_numb += 1;
                    bytes = read(fd, out_packet.data, MAXLEN);
                    if(bytes <= 0) break;
                    out_packet.data_length = bytes;
                }	    	    
                //send packet
                drop = drop_packet(droprate);
                if(drop == FALSE || out_packet.packet_numb == 1)//don't drop first packet
                {
                    if (sendto(sd, &out_packet, sizeof(out_packet), 0, (struct sockaddr *)&client, client_len) < 0) 
                    {
		                fail("Failed sendto");
		            }
                    else
                    {
                        fprintf(server_log, "<-- Send  #%i\n", out_packet.packet_numb);
                    }
                }

                //wait for ack, get acks               
                if(recvfrom(sd, &in_packet, sizeof(in_packet), 0/*MSG_DONTWAIT*/, (struct sockaddr *)&client, &client_len) < 0)
                {
                    if(retries < MAX_RETRIES && window_numb > GBN_WINDOW_SIZE )
                    {
                        retries++;
                        fprintf(stdout, "Retry... %i\n", out_packet.packet_numb);
                        fprintf(server_log, "NO ACK, RETRY %i\n", out_packet.packet_numb);
                        gotACK = FALSE;
                        go_back_to = out_packet.packet_numb;
                        continue;
                    }
                    else
                    {
                        fail("timeout");        
                    }
                }
                else
                {
                    gotACK = TRUE;
                    fprintf(server_log, "--> ACK   #%i\n", out_packet.packet_numb);
                }
            }//while end send file
            
            close(fd);
            close(sd);   
        break;
        case SELECTIVE_REPEAT:
        default:
            usage(pname);
        }//case end
        fclose(server_log);
	}//loop end
	close(sd);
	return(0);
}

