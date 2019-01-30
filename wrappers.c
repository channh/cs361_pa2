#include "wrappers.h"
// --------------------------------
// Assignment  : PA02-IPC
// Date        : 10/1/18
//
// Author      : Nathan Chan and Amanda Pearce from csapp.c
// File Name   : wrappers.c
// ----------------------------------




/*********************************************
 * Wrappers for Unix process control functions
 ********************************************/
void unix_error(char *err){
	fprintf(stderr, "%s\n", err);
	exit(-2);
}
/* $begin forkwrapper */
pid_t Fork(void) 
{
    pid_t pid;

    if ((pid = fork()) < 0)
		unix_error("Fork error\n");
    return pid;
}
/* $end forkwrapper */

/*******************************
 * Wrappers for Posix semaphores
 *******************************/
int     Shmget(key_t key, size_t size, int shmflg){
	int id;
	if((id = shmget(key, size, shmflg)) < 0)
		unix_error("Shmget error");
	return id;
}

void   *Shmat(int shmid, const void *shmaddr, int shmflg){
	void* address;
	if((address = shmat(shmid, shmaddr, shmflg)) < 0)
		unix_error("Shmat error");
	return address;
}

int     Shmdt( const void *shmaddr ){
	if(shmdt(shmaddr)<0)
		unix_error("shmdt error");
	return 0;
}

sem_t  *Sem_open( const char *name, int oflag, mode_t mode, unsigned int value ){
	sem_t* addr;
	if((addr = sem_open(name, oflag, mode, value)) == SEM_FAILED )
		unix_error("sem_open error");
	return addr;
}

sem_t  *Sem_open2( const char *name, int oflag ) {
	sem_t* addr;
	if((addr = sem_open(name, oflag)) < 0)
		unix_error("sem_open2 error");

	return (sem_t*) addr;
}

int     Sem_close( sem_t *sem ){
	if(sem_close(sem) <0)
		unix_error("Sem_close error");
	return 0;
}

int     Sem_unlink( const char *name ){
	if(sem_unlink(name)<0)
		unix_error("sem_unlink error");
	return 0;
}

int     Sem_wait( sem_t *sem ) {
	if(sem_wait(sem) < 0)
		unix_error("sem_wait error");
	return 0;
}

int     Sem_post( sem_t *sem ) {
	if(sem_post(sem) < 0){
		unix_error("sem_post error");
		printf("SEM POST FAILED. Error code=%d\n", errno ) ;
	}
	return 0;
}

int Sem_init(sem_t *sem, int pshared, unsigned int value) 
{
    if (sem_init(sem, pshared, value) < 0)
		unix_error("Sem_init error");
	return 0;
}

int     Sem_destroy( sem_t *sem ) {
	if(sem_destroy(sem) < 0)
		unix_error("sem_destroy error");
	return 0;
}


/*******************************
 * Wrappers for message queues
 *******************************/
int Msgget( key_t msgquekey , int msgflag){
	int queID; 
	char msg[200];

	if ((queID = msgget(msgquekey, msgflag)) < 0){
		snprintf(msg, 200, "WRAPPER: User Failed to find message queue key %d\n", msgquekey);
	    unix_error(msg);
	}
	return queID;
}

ssize_t	Msgrcv(int msqid, void* msgp, size_t msgsz, long msgtyp, int msgflg){
	ssize_t msgStatus;
	if (msgStatus = msgrcv( msqid , msgp , msgsz , msgtyp, msgflg) < 0 ){
		printf("Supervisor failed to recieve message on queID %d\n", msqid);
		exit(-2);
	}
	return msgStatus;
}

int Msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflag){
	if(msgsnd(msqid, msgp, msgsz, msgflag) < 0)
		unix_error("msgsnd error");
	return 0;
}


