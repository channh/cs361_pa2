#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>

#include "message.h"
#include "wrappers.h"
// --------------------------------
// Assignment  : PA02-IPC
// Date        : 10/1/18
//
// Author      : Nathan Chan and Amanda Pearce
// File Name   : supervisor.c
// ----------------------------------

// recieve production and completion messages from the N factory line processes, and synchronizes with the parent via rendezvous-type names semaphores

int main(int argc, char **argv)
{
	if (argc != 2)
	{
	    fprintf(stderr, "WRONG NUMBER OF ARGUMENTS\n");
	    exit(-2);
	}
	printf("SUPER: Starting\n");
	int lines_active = atoi(argv[1]);
	
	msgBuf msg;
	int msg_flag = 0600;
	key_t msg_que_key =  ftok("./message.h" , 0 ) ;
	int que_ID ;
	// char array holding the final report for all the lines when they complete
	int parts[lines_active];
	int iter[lines_active];
    int num_lines = lines_active;

    for (int i = 1; i <= lines_active; i++) {
        parts[i-1] = 0;
        iter[i-1] = 0;
    }

    // Shared Memory.
    int        shm_ID ;
   	key_t      shm_key;
   	int        shm_flag;
   	shmData    *data;

   	shm_key = ftok("./wrappers.h" , 0 ) ;
   	shm_flag = IPC_CREAT | S_IRUSR | S_IWUSR ;
   	shm_ID  = Shmget( shm_key , SHMEM_SIZE , shm_flag ) ;
   	data = (shmData *) Shmat( shm_ID , NULL , 0 ); // Attach for R/W

    // semaphore
   	int sem_flag, sem_mode;
   	sem_t * mutex, *factories_completed, *print_permission, *done_printing, *ordered , *made ;;

   	sem_flag = O_CREAT;
   	sem_mode = S_IRUSR | S_IWUSR;

   	mutex = Sem_open("mutex_channh", sem_flag, sem_mode, 1);
   	factories_completed = Sem_open("factories_completed_channh", sem_flag, sem_mode, 0);
   	print_permission = Sem_open("print_permission_channh", sem_flag, sem_mode, 0);
   	done_printing = Sem_open("done_printing_channh", sem_flag, sem_mode, 0);	

	// find mailbox from parent process
	que_ID = Msgget( msg_que_key , msg_flag);
	
	while (lines_active > 0){

	    // recieve a message from the message Queue created by Parent process
        Msgrcv( que_ID , &msg , MSG_INFO_SIZE , 1, 0);

	    if(msg.body.production == 1){ // if its a production message
           printf("SUPER: Factory Line %3d produced %4d parts in %5d milliSecs\n", msg.body.factory_ID, msg.body.capacity, msg.body.duration);
		   parts[msg.body.factory_ID - 1] += msg.body.capacity;
		   iter[msg.body.factory_ID - 1] += 1;
		    
       } else if(msg.body.production == 0) { /* its a completion message */
       	lines_active --;
       	printf("SUPER: Factory line %3d Completed its task\n", msg.body.factory_ID);
       } else{
    // How do i discard unsupported messages
            continue;
	    }
	    // when parent tells us we can
    }
    Sem_post(factories_completed);

    Sem_wait(print_permission);
    printf("\n****** SUPER: Final Report ******\n");
    for (int i = 1; i <= num_lines; i++) {
        printf("Line %4d made total of %4d parts in %5d iterations\n", i, parts[i-1], iter[i-1]);
    }
    printf("==============================\n");
    printf("Grand total parts made = %5d   vs order size of %5d\n\n", data->made_so_far, data->order_size);

    printf(">>> Supervisor Terminated\n");
    Sem_post(done_printing);
    exit(1);
}
