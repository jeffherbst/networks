//Jeffrey Herbst
//CS 4220
//PROJ 1
//Functions and definitions for UDP protocal
#ifndef MYDEFS_DOT_H
#define MYDEFS_DOT_H

#include <sys/stat.h>

#define SERVER_UDP_PORT         5920
#define MAXLEN                  4096
#define DEFLEN                  64
#define ACKSIZE                 8
#define TIMEOUT_S               1
#define TIMEOUT_US  		    20 		// Timeout in millisec
#define GBN_WINDOW_SIZE			5 		// How many outstanding frames are allowed?
#define MAX_RETRIES				20

// Id of protocols
#define STOP_AND_WAIT			1
#define GO_BACK_N				2
#define SELECTIVE_REPEAT		3

#define FALSE	(0)
#define TRUE 	(!FALSE)

//structure for packet. Put in buffer and send. 
struct packet
{
    uint32_t packet_numb;
    uint32_t total_packets;
    uint32_t data_length;
    uint32_t ACK;           //is this an ack
    char data[MAXLEN];
};

//macro for exiting with a statement why
void fail(char *s)
{
	fprintf(stderr, "EXIT - %s \n\n", s);
	exit(EXIT_FAILURE);
}

//calculate delay
long delay(struct timeval t1, struct timeval t2)
{
	long d;
	d = (t2.tv_sec - t1.tv_sec) * 1000;
	d += ((t2.tv_usec - t1.tv_usec + 500) / 1000);
	return d;
}

//drop this packet or not
int drop_packet(int probability)
{
	int drop = FALSE;
	
	int random_number = rand() % 99;
	if(random_number < probability) drop = TRUE;

	return drop;
}

//calculate total number of frames we will use
int calculate_num_frames(int bytes, int data_size)
{
	int num_frames;
	num_frames = bytes/data_size;
	if(bytes % data_size != 0) num_frames++;
	return num_frames;
}


off_t fsize(const char *filename)
{
    struct stat st;
    
    if(stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

void reverse(char *str)
{
    int c, i, j;

    for(i = 0, j = strlen(str) - 1; i < j; i++, j--)
    {
        c = str[i];
        str[i] = str[j];
        str[j] = c;
    }
}

void itoa(int num, char *str, int base)
{
    int i = 0, rem;
    
    while(num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem-10) + 'a' : rem + '0';
        num = num / base;
    }
    str[i] = '\0'; 
    reverse(str);
}


#endif
