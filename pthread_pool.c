#include "pthread_pool.h"
#include "task_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
pthread_pool* pthread_pool_create(pthread_pool_attr *attr){
	pthread_pool *tp=NULL;
	pthread_t *thread = NULL;
	int i;
	pthread_attr_t *create_attr=NULL;
	tp = malloc(sizeof(pthread_pool));
	if (tp == NULL){
		return NULL;
	}

	if (attr != NULL){

		if (attr->min > attr->max || attr->max < 0)
		{
			return NULL;
		}
		create_attr=attr->create_attr;
		tp->min = attr->min;
		tp->max = attr->max;

	}
	else
	{
		tp->min = 10;
		tp->max = 20;
	}
	tp->current =0;
	tp->list=malloc(sizeof(task_list_t));
	if (tp->list == NULL){
		return NULL;
	}

	thread = malloc(sizeof(pthread_t));
	if (thread == NULL){
		return NULL;
	}
	tp->list->priority_first_node = NULL;
	tp->list->priority_last_node = NULL;
	tp->list->return_first_node = NULL;
	tp->list->return_last_node = NULL;
	sem_init(&(tp->list->sem), 0, 0);
	tp->list->work=0;
	if (pthread_mutex_init(&(tp->list->return_list_mutex), NULL) != 0)
	{
		return NULL;
	}
	if (pthread_mutex_init(&(tp->list->priority_list_mutex), NULL) != 0)
	{
		return NULL;
	}

	for (i=0; i<tp->max; i++){
		int rv;
		rv = pthread_create(thread, create_attr, function_loop, tp->list);
		if (rv != 0){
			return NULL;
		}
		tp->current++;
		/*tp->list->(pthreads+i)->status=BETWEEN_FUNCTIONS;*/
	}

	return tp;
}


void* function_loop (void* tlist){
	task_list_t * list = (task_list_t *) tlist;
	task_node_t *current_task = NULL;
	int work = 1;
	void *return_value;
	while (work != 0){
		if (list->work != 0){
			work = 0;
			list->work-=1;

			pthread_barrier_wait(&(list->barrier));

			pthread_detach(pthread_self());
			break;
		}
		current_task = get_next_task(&list);
		return_value = current_task->task(current_task->argument);
		change_return_value(&(list->return_first_node), &(list->return_last_node), return_value, current_task->task_id, TASK_FINISHED, &list->return_list_mutex);
	}

	return NULL;
}

void pthread_pool_destroy(pthread_pool *tp) {
	pthread_barrier_init(&(tp->list->barrier), NULL, tp->current + 1);
	tp->list->work = -1;
	pthread_barrier_wait(&(tp->list->barrier));
	return;
}

