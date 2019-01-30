#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include "wrappers.h"

#include "message.h"
// --------------------------------
// Assignment  : PA02-IPC
// Date        : 10/1/18
//
// Author      : Nathan Chan and Amanda Pearce
// File Name   : factory.c
// ----------------------------------
int main(int argc, char **argv)
{
    if (argc != 4)
	{
	    fprintf(stderr, "WRONG NUMBER OF ARGUMENTS\n");
	    exit(-2);
	}
	
	int factory_ID = atoi(argv[1]);		/* a unique integer */
	int capacity = atoi(argv[2]);		/* the maximum number of parts it can make in one iteration */
	int duration = atoi(argv[3]);		/* The time in milliseconds to do this */
	int parts_made_by_me = 0, iterations = 0;	
	int remaining; 						// remaining parts
    // printf("Factory Line %4d: HI IM ALIVE I HAVE a capacity of    %d, duration of     %d\n", factory_ID, capacity, duration);
	

	// Set up shared memory & initialize its objects
	int        shm_ID ;
   	key_t      shm_key;
   	int        shm_flag;
   	shmData    *data;

   	shm_key = ftok("./wrappers.h" , 0 ) ;
   	shm_flag = IPC_CREAT | S_IRUSR | S_IWUSR ;
   	shm_ID  = Shmget( shm_key , SHMEM_SIZE , shm_flag ) ;
   	data = (shmData *) Shmat( shm_ID , NULL , 0 ); // Attach for R/W

   	// semaphores
  	int sem_flag, sem_mode;
  	sem_t * mutex, *factories_completed, *print_permission, *done_printing;

  	sem_flag = S_IRUSR | S_IWUSR;

  	mutex = Sem_open2("mutex_channh", sem_flag);

   	// Set up the message queue
	key_t 	msg_que_key;
	int 	msg_flag;
	int     que_ID;
	msgBuf	msg;

	msg_flag = IPC_CREAT | 0600 ;
	msg_que_key =  ftok("./message.h" , 0 ) ;
	que_ID = Msgget( msg_que_key , msg_flag);

// HOW CAN I MAKE THIS BETTER????
//	printf("CHILD %d: about to call Sem_wait(mutex)\n", factory_ID);
  	Sem_wait(mutex);
   	remaining = data->not_made ;

  	Sem_post(mutex);


	while (data->not_made != 0)
	{
	    int num_to_make;

		// determine how many to make & update remaining
		Sem_wait(mutex);
   		remaining = data->not_made ;
   		
   		if(remaining < capacity){ /* we are going to make the rest */
            num_to_make = remaining;
   			data->not_made = 0;
   			remaining = 0;   			
   		}
   		else {/* There will still be some left to make after we make these */
   			data->not_made -= capacity;
   			remaining -= capacity;
   			num_to_make = capacity;
   		}

		printf("Factory Line %3d: Going to make %5d parts in %4d milliSecs\n", factory_ID, num_to_make, duration); //using a mutually exclusive semaphore
		Sem_post(mutex);
		
		usleep(duration * 1000); // milliseconds
        // at this point they are both waiting 		
		Sem_wait(mutex);
		data->made_so_far += num_to_make;
		Sem_post(mutex);

		// create & send production message through the message queue to supervisor
		msg.msgType = 1; // this is sending to the supervisor it isn't replying to anything
		msg.body.factory_ID = factory_ID;
		msg.body.duration = duration;
		msg.body.capacity = num_to_make;
		msg.body.production = 1;
		Msgsnd(que_ID, &msg, MSG_INFO_SIZE, 0);
		
		iterations++;
		parts_made_by_me += num_to_make; 
	}

	// create & send completion message
	msg.body.iterations = iterations;
	msg.body.parts_made = parts_made_by_me;
	msg.body.production = 0;
	Msgsnd(que_ID, &msg, MSG_INFO_SIZE, 0);

	printf(">>> Factory Line %3d: Terminating after making total of %6d parts in %4d iterations\n", factory_ID, parts_made_by_me, iterations);

	shmdt(data);
}
