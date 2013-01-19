#include "return_list.h"
#include "task_list.h"
#include <stdlib.h>
#include <stdio.h>

int add_new_task(TaskList **list, Task task, void *argument, int priority) {

	if(pthread_mutex_lock(&((*list)->priority_list_mutex)) != 0) return -1;
	if(list_add_new_task(task, argument, priority, (*list)->next_task_id, &((*list)->priority_first_node), &((*list)->priority_last_node)) != 0){
		/* Set Error */
		return -1;
	}
	if(sem_post(&((*list)->sem)) != 0){
		/* Set Error */
		return -1;
	}
	if(add_return_value(&((*list)->return_first_node), &((*list)->return_last_node), NULL , (*list)->next_task_id, TASK_NOT_STARTED, &(*list)->return_list_mutex) != 0) {
		/* Set Error */
		return -1;
	}
	((*list)->next_task_id)++;
	if(pthread_mutex_unlock(&((*list)->priority_list_mutex)) != 0) return -1;
	if(add_return_value(&(*list)->return_first_node, &(*list)->return_last_node, NULL, (*list)->next_task_id-1, TASK_NOT_STARTED, &(*list)->return_list_mutex) != 0)
	{
		/* Set Error */
		return -1;
	}
	return 0;
}

TaskNode *get_next_task(TaskList **list) {
	TaskNode *node = NULL;	

	if(sem_wait(&((*list)->sem)) != 0){
		/* Set Error */
		return NULL;
	}

	if (pthread_mutex_lock(&((*list)->priority_list_mutex)) != 0) return NULL;

	if((node = list_get_next_task(&((*list)->priority_first_node), &((*list)->priority_last_node))) == NULL) {
		/*Set Error */
		return NULL;
	}
	
	/* Set RUNNING in return list */
	if(pthread_mutex_unlock(&((*list)->priority_list_mutex)) != 0) return NULL;

	change_return_value (&(*list)->return_first_node, &(*list)->return_last_node, NULL, node->task_id, TASK_RUNNING, &(*list)->return_list_mutex);
	return node;
}


int get_return_value_by_id(TaskList** list, int task_id, void** value_returned){
	ReturnNode *node= NULL;
	ReturnNode *aux= NULL;

	pthread_mutex_lock(&(*list)->return_list_mutex);
	if ((*list)->return_first_node == NULL)
	{
		pthread_mutex_unlock(&(*list)->return_list_mutex);
		return -1;
	}
	if ((*list)->return_first_node->task_id == task_id)
	{
		aux=((*list)->return_first_node);
		node=aux->next;
	}
	else
	{
		for(node=(*list)->return_first_node; node->next !=NULL; node=node->next){
			if (node->next->task_id == task_id){
				aux=node->next;
				break;
			}
		}
	}

	if (aux==NULL){
		pthread_mutex_unlock(&(*list)->return_list_mutex);
		return -2;
	}

	if(aux->task_state != TASK_FINISHED)
	{
		pthread_mutex_unlock(&(*list)->return_list_mutex);
		return aux->task_state;
	}
	*value_returned=aux->return_value;
	
	node->next=aux->next;

	if (aux==(*list)->return_first_node){
		(*list)->return_first_node=aux->next;
	}
	if (aux==(*list)->return_last_node){
		(*list)->return_last_node=node;
		(*list)->return_last_node->next=NULL;
	}
	free(aux);

	pthread_mutex_unlock(&(*list)->return_list_mutex);
	return aux->task_state;
}

