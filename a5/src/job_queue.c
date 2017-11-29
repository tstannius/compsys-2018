#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) { 
  job_queue->capacity = capacity; // MAX
  job_queue->first    = 0;        // use_ptr
  job_queue->num_used = 0;        // count
  job_queue->buffer   = (void**)malloc((capacity * sizeof(void*)));      // pointer to a pointer, pointing to an array of void pointers
  job_queue->dead     = 0;        // 0 when alive, 1 when destoyed
  
  if (pthread_mutex_init(&(job_queue->mutex), NULL) != 0) {
    return 1;
  }
  if (pthread_cond_init(&(job_queue->cond), NULL) != 0) {
    return 1;
  }

  return 0;
}

int job_queue_destroy(struct job_queue *job_queue) {
  // If any threads are blocked on a call to job_queue_pop() when 
  // job_queue_destroy() is called, the threads must be woken with
  // job_queue_pop() returning -1
  printf("destroy-enter\n");
  assert(pthread_mutex_lock(&(job_queue->mutex)) == 0);
  
  if (job_queue->dead){
    assert(pthread_mutex_unlock(&(job_queue->mutex)) == 0);
  	return 1;
  }
  // BUG: Destroy hænger her?

  printf("destroy num_used: %d\n", job_queue->num_used);
  while (job_queue->num_used > 0) {
    // Skal kunne håndtere tilfælde hvor num_used > 0 og der er færre pop end num_used eller ingen pop
    // BUG:? Fibs hænger her, da destroy ikke opdager, at num_used har nået 0
    pthread_cond_wait (&(job_queue->cond), &(job_queue->mutex)); 
  }
  printf("destroy-done waiting\n");
  printf("destroy num_used: %d\n", job_queue->num_used);
  
  job_queue->dead = 1;
  free(job_queue->buffer);
  assert(pthread_cond_broadcast(&(job_queue->cond)) == 0); // Signal for pop-threads to die

  assert(pthread_mutex_unlock(&(job_queue->mutex)) == 0);

  return 0;
}

  // Bruge mutex og cv, hvis der ikke er plads
  // Skal sleepe, når condition siger, at der ikke kan pushes
  // (kø er fuld)
int job_queue_push(struct job_queue *job_queue, void *data) {
  printf("push-enter\n");
  assert(pthread_mutex_lock(&(job_queue->mutex)) == 0);
  if (job_queue->dead){
    assert(pthread_mutex_unlock(&(job_queue->mutex)) == 0);
  	return 1;
  }

  printf("push num_used pre: %d\n", job_queue->num_used);
  while((job_queue->capacity == job_queue->num_used) && !(job_queue->dead)){
  	pthread_cond_wait (&(job_queue->cond), &(job_queue->mutex)); 
  }
  printf("push-done waiting\n");


  int i = (job_queue->first + job_queue->num_used) % job_queue->capacity;
  job_queue->buffer[i] = data;
  job_queue->num_used++;
  printf("push num_used post: %d\n", job_queue->num_used);
  assert(pthread_cond_broadcast(&(job_queue->cond)) == 0);

  assert(pthread_mutex_unlock(&(job_queue->mutex)) == 0);

  return 0;
}

// Bruge mutex og cv
// Skal sleepe, når condition siger, at der ikke kan poppes 
// (intet i kø)
int job_queue_pop(struct job_queue *job_queue, void **data) {
  printf("pop-enter\n");
  assert(pthread_mutex_lock(&(job_queue->mutex)) == 0);
  if (job_queue->dead) {
    assert(pthread_mutex_unlock(&(job_queue->mutex)) == 0);
    return 1;
  }
  printf("pop num_used: %d\n", job_queue->num_used);
  while((job_queue->num_used == 0) && !(job_queue->dead)){
    pthread_cond_wait (&(job_queue->cond), &(job_queue->mutex));
  }
  printf("pop-done waiting\n");

  if (job_queue->dead == 1) {
    assert(pthread_mutex_unlock(&(job_queue->mutex)) == 0);
    return -1;
  }

  *data = job_queue->buffer[job_queue->first];
  if (job_queue->first == (job_queue->capacity - 1)) {
    job_queue->first = 0;
  }
  else {
    job_queue->first++;
  }
  
  job_queue->num_used--;

  assert(pthread_cond_broadcast(&(job_queue->cond)) == 0);
  assert(pthread_mutex_unlock(&(job_queue->mutex)) == 0);
  return 0;
}