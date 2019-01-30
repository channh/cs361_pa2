#include "wrappers.h"
#include <time.h>
#include <stdio.h>
#include <unistd.h>     // fork(), exec*()
#include <sys/types.h>  // wait(), waitpid()
#include <sys/wait.h>   // wait(), waitpid()

// --------------------------------
// Assignment  : PA02-IPC
// Date        : 10/1/18
//
// Author      : Nathan Chan and Amanda Pearce
// File Name   : parent.c
// ----------------------------------

int to_be_made;
#define	ARG_MAX 20

// Receives the desired number of factory lines N as argv[1] and order_size (total number of parts ordered) as argv[2]
int main(int argc, char* argv[])
{

// in supervisor class: how do i discard unsupported messages?
	if (argc != 3){
	    fprintf(stderr, "WRONG NUMBER OF ARGUMENTS\n");
	    exit(-2);
	}

	int num_lines = atoi(argv[1]);
	int order_size = atoi(argv[2]);

	pid_t 	super_ID;
	char	cmnd[STR_SIZE];
    int     super_status;
    int 	child_status[num_lines];

	// Set up shared memory & initialize its objects
    int        shm_ID ;
   	key_t      shm_key;
   	int        shm_flag;
   	shmData    *data;

   	shm_key = ftok("./wrappers.h" , 0 ) ;
   	shm_flag = IPC_CREAT | S_IRUSR | S_IWUSR ;
   	shm_ID  = Shmget( shm_key , SHMEM_SIZE , shm_flag ) ;
   	data = (shmData *) Shmat( shm_ID , NULL , 0 ); // Attach for R/W

   		/* Initialize data in the shared memory */
	data->not_made = order_size;
    data->order_size = order_size;
	data->made_so_far = 0;

   	// semaphore
   	int sem_flag, sem_mode;
   	sem_t * mutex, *factories_completed, *print_permission, *done_printing, *ordered , *made ;;

   	sem_flag = O_CREAT;
   	sem_mode = S_IRUSR | S_IWUSR;

   	mutex = Sem_open("mutex_channh", sem_flag, sem_mode, 1);
   	factories_completed = Sem_open("factories_completed_channh", sem_flag, sem_mode, 0);
   	print_permission = Sem_open("print_permission_channh", sem_flag, sem_mode, 0);
   	done_printing = Sem_open("done_printing_channh", sem_flag, sem_mode, 0);
   	
	// Set up the message queue
	key_t 	msg_que_key;
	int 	msg_flag;
	int     que_ID;

	msg_flag = IPC_CREAT | IPC_EXCL | 0600 ;
	msg_que_key =  ftok("./message.h" , 0 ) ;
	que_ID = Msgget( msg_que_key , msg_flag);
	
	// Fork/Execute Supervisor process to run in a separate terminal
	super_ID = Fork();
	if (super_ID == 0) 		 //the supervisor child process 
	{

		snprintf(cmnd, STR_SIZE,"./supervisor %d ; exec bash", num_lines);

		 if ( execlp("/usr/bin/gnome-terminal", "gnome-terminal" , 
		 	"--", "/bin/bash" , "-c", cmnd, NULL) < 0 )
		 {
		 	perror("PARENT: execlp Supervisor Failed");
		 	exit(-1);
		 }
	}

	// Create / Execute all Factory Line processes
    pid_t   id[num_lines];
    srandom(time(NULL));
    printf("PARENT: Will Manufacture an Order of Size = %3d parts\nCreating %d Factory Lines\n", order_size, num_lines);
    
	for(int i = 1; i < num_lines+1; i++){
	    int factory_ID = i;
	    int capacity = random();
	    int duration = random();
        char arg0[ARG_MAX], arg1[ARG_MAX], arg2[ARG_MAX];

	    // capacity is a random integer in the range 10 to 50 inclusive
	    if(capacity > 50)
	        capacity = capacity % 50;
	    if(capacity < 10)
	        capacity += 10;
	    
	    //duration is a random integer in the range 500 to 1200 inclusive
	     if(duration > 1200) 
	         duration = duration % 1200;
	     if(duration < 500)
	         duration += 500;

		snprintf(arg0, ARG_MAX, "%d", factory_ID);
        snprintf(arg1, ARG_MAX, "%d", capacity);
        snprintf(arg2, ARG_MAX, "%d", duration);

        printf("PARENT: Factory Line %3d Created with Capactiy %5d Duration %5d\n", factory_ID, capacity, duration);
	    if(id[i] = Fork() == 0){    /* child */
	        if(execlp("./factory", "factory", arg0, arg1, arg2, NULL) < 0){
                perror("PARENT: execlp Factory Failed");   
                exit(-1);         
            }   
	    } 
	      
	} // end for
	// wait for supervisor to collect aggregates from all factory lines
	Sem_wait(factories_completed);

	puts("PARENT: Supervisor says all lines have completed");

	// give permission to Supervisor to print final report via a rendezvous semaphore
	Sem_post(print_permission);

	// wait for supervisor to finish printing;
	waitpid(super_ID, &super_status, 0);

    printf("PARENT: Shutting Down Factory Lines\n");
	// clean up after zombie processes (supervisor + all factory lines
    for (int i = 0; i < num_lines; i++)
	    waitpid(id[i], &child_status[i], 0);

	// destroy any objects in shared memory
	shmdt(data);

	// destroy the shared memory
	shmctl(shm_ID , IPC_RMID, NULL);

	Sem_close(mutex); Sem_unlink("mutex_channh");
	Sem_close(factories_completed); Sem_unlink("factories_completed_channh");
	Sem_close(print_permission); Sem_unlink("print_permission_channh");
	Sem_close(done_printing); Sem_unlink("done_printing_channh");


	// destroy the message queue
	msgctl(que_ID, IPC_RMID, NULL);

}
