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

int reader_count;
sem_t reader_turnstyle;
sem_t count_mutex;
sem_t critical_section_mutex;

void *reader(void *arg) {
  sem_wait(&reader_turnstyle);
  sem_post(&reader_turnstyle);
  sem_wait(&count_mutex);
  reader_count++;
  if(reader_count == 1)
    sem_wait(&critical_section_mutex);
  sem_post(&count_mutex);
  printf("Reader starting\n");
  sleep(1);
  printf("Reader finished\n");
  sem_wait(&count_mutex);
  reader_count--;
  if(reader_count == 0)
    sem_post(&critical_section_mutex);
  sem_post(&count_mutex);
  return NULL;
}

void *writer(void *arg) {
  sem_wait(&reader_turnstyle);
  sem_wait(&critical_section_mutex);
  printf("Writer starting\n");
  sleep(1);
  printf("Writer finished\n");
  sem_post(&critical_section_mutex);
  sem_post(&reader_turnstyle);
  return NULL;
}

int main(int argc, char **argv) {
  pthread_t tid[8];
  pthread_attr_t attr;
  int i;
  reader_count = 0;
  sem_init(&count_mutex, 0, 1);
  sem_init(&critical_section_mutex, 0, 1);
  sem_init(&reader_turnstyle, 0, 1);

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
