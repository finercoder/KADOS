/* Copyright 2016 Rose-Hulman */
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/**
   In the reader/writers problem, any number of readers can access the
   data at the same time.  However, only 1 writer can access the data
   (that is a writer must prevent both all other writers and all other
   readers).  There are 3 different ways to solve the problem, with
   increasing complexity.

   STAGE 1: Readers trump writers.  In this stage readers have
   priority - no writers will be allowed to write if there are any
   readers.  The key thing is detecting if there are any readers...

   STAGE 2: Prevent starvation.  In this case, when a writer arrives,
   all readers currently reading are allowed to finish.  Then the
   writer can go.  Then anybody who's batched up in the meantime can
   go (could be some readers, could be another writer).

   Stage 3: Writers trump readers.  In this case, readers should not
   be scheduled when there are writers to be schedule.

   
   Note that in this example code there's no actual reading/writing
   going on.  Reading/writing is just the way we justify the rule.
**/
sem_t semread;
sem_t semwrite;
sem_t semWriteFeed;

int readnum = 0;


void *reader(void *arg) {
  sem_wait(&semWriteFeed);
  sem_post(&semWriteFeed);

  sem_wait(&semread);
  readnum++;
  if (readnum == 1) {
    sem_wait(&semwrite);
  }
  sem_post(&semread);

  printf("Reader starting\n");
  sleep(1);
  printf("Reader finished\n");

  sem_wait(&semread);
  if (readnum == 1) {
    sem_post(&semwrite);
  }
  readnum--;
  sem_post(&semread);
  return NULL;
}

void *writer(void *arg) {
  sem_wait(&semWriteFeed);

  sem_wait(&semwrite);
  printf("Writer starting\n");
  sleep(1);
  printf("Writer finished\n");
  sem_post(&semwrite);
  
  sem_post(&semWriteFeed);
  return NULL;
}

int main(int argc, char **argv) {
  pthread_t tid[9];
  pthread_attr_t attr;
  int i;

  sem_init(&semwrite, 0, 1);
  sem_init(&semread, 0, 1);
  sem_init(&semWriteFeed, 0, 1);

  pthread_create(&tid[1],NULL,reader,NULL);
  pthread_create(&tid[2],NULL,reader,NULL);

  pthread_create(&tid[6],NULL,writer,NULL);

  pthread_create(&tid[3],NULL,reader,NULL);
  pthread_create(&tid[4],NULL,reader,NULL);

  pthread_create(&tid[7],NULL,writer,NULL);
  
  pthread_create(&tid[5],NULL,reader,NULL);

  for (i = 0; i < 8; i++) {
    pthread_join(tid[i], NULL);
  }
  printf("Everything finished.\n");
}

/**

   HINT #1: So this one works a lot like the barrier we did last time.
   Except, rather than have special code that executes when we we reah
   the max threads, we have special code when we have the first reader
   thread (in this case, lock the mutex that the writers use to
   protect the critical section).

   Final complication: we can't have most readers aquare the writer's
   mutex, because then they won't run in paralell.  But how can we
   prevent them from running when there's a writer in the critical
   section?  Have the first reader thread aquire the writer's mutex
   within mutex code that protects the reader count.  Now, other
   readers will get blocked if the first reader can't aquire the
   writer's mutex.
   
 **/

/**

   HINT #2:

   So this involves a turnstyle:

   sem_wait(&turnstile);
   sem_post(&turnstile);

   The key idea is that it acts like no barrier at all...unless some
   jerk locks the turnstile by doing a wait without posting.  If that
   happens, things will just batch up at the turntile - until it is
   unlocked again.
   
 **/

/**

   HINT #3:

   This is complex one!  Check out the little book of semaphores

   http://www.greenteapress.com/semaphores/

   it has a good discussion of the solution.
   
 **/
