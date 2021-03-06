#ifndef _PTHREAD_POOL_H_
	#define _PTHREAD_POOL_H_

#include "task_list.h"

typedef struct {
	int pthread_admin;
	int min;
	int max;
	int current;
	task_list_t* list;
}pthread_pool;

typedef struct{
	int min;
	int max;
	pthread_attr_t *create_attr;
}pthread_pool_attr;

	pthread_pool* pthread_pool_create(pthread_pool_attr*);
	void* function_loop (void*); /*recibe el tasklist */
	void pthread_pool_destroy(pthread_pool *tp);



#endif
