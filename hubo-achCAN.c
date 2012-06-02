#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

#include <string.h>
#include <inttypes.h>

// for timer
#include <time.h>
#include <sched.h>
#include <sys/io.h>

#include "ach.h"

// For CAN
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
 
#include <linux/can.h>
#include <linux/can/raw.h>


// for serial
#include <termios.h>
#define NSEC_PER_SEC    1000000000


// More for CAN
/* At time of writing, these constants are not defined in the headers */
#ifndef PF_CAN
#define PF_CAN 29
#endif
 
#ifndef AF_CAN
#define AF_CAN PF_CAN
#endif



// For Serial
int 	fd1;
int 	fd2;
char	*buff, *buffer, *bufptr;
int	wr, rd, nbytes, tries;
 

// shared memory
typedef	int	d[1];	// data xfer for testing

// ach channels
ach_channel_t chan_num;
// 
 

int open_can(void)
{
      	// create a socket
	int skt = socket( PF_CAN, SOCK_RAW, CAN_RAW );
	
	// locate the interface you wish to use
	struct ifreq ifr;
	strcpy(ifr.ifr_name, "can0");
	ioctl(skt, SIOCGIFINDEX, &ifr); // ifr.ifr_ifindex gets filled with that devices index
	
	// select that CAN interface and bind the socket to it
	struct sockaddr_can addr;
	addr.can_family = AF_CAN;
	addr.canifindex = ifr.ifr_ifindex;
	bind( skt, (struct sockaddr*)&addr, sizeof(addr) );
	
      	return (skt);
}




static inline void tsnorm(struct timespec *ts){
   	while (ts->tv_nsec >= NSEC_PER_SEC) {
      		ts->tv_nsec -= NSEC_PER_SEC;
      		ts->tv_sec++;
	}
}


void mainLoop(void){
	int r = ach_open(&chan_num, "getNum", NULL);
	assert( ACH_OK == r );


	struct timespec t;
	int interval = 100000000;
	
	// open can
	int skt = open_can();

	// get current time
        clock_gettime(0,&t);

        // start one second after
        t.tv_sec++;


	d D = {0};
	while(1){
		// wait until next shot
                clock_nanosleep(0,TIMER_ABSTIME,&t, NULL);

		//------------------------------
                //------[ do sutff start ]------
		//------------------------------
		
		// get info from ach
		size_t fs;
		r = ach_get( &chan_num, D, sizeof(D), &fs, NULL, ACH_O_WAIT );

		//printf("num2 = %f\n", (float)D[0]);
//		printf("Bytes sent are %d \n",wr);
		//------------------------------
		//-----[ do stuff stop ]--------
		//------------------------------
		

		// calculate next shot
                t.tv_nsec+=interval;
                tsnorm(&t);
		
	}
}

int main(int argc, char **argv){
	(void) argc; (void)argv;
	int r;

	// create chanels
	//r = ach_unlink("getNum");
//	r = ach_unlink("getNum");
//	assert( ACH_OK == r || ACH_ENOENT == r );

//	r = ach_create("getNum", 10ul, 256ul, NULL);
//	assert( ACH_OK == r);


	r = ach_open(&chan_num, "getNum", NULL);
	assert(ACH_OK == r);

	mainLoop();

	pause();
	return 0;

}


