/*
 * frames_buf.c
 *
 *  Created on: Jul 15, 2016
 *      Author: marshall
 */

#include <stdlib.h>
#include <pthread.h>
#include "frames_queue.h"

struct aframe{
	int ready;//1:ready for play  0:data is dirty or out of data
	int valid_size; //the valid data size of pcm data
	int size;
	unsigned char *data;
};

struct framesQueue{
	pthread_mutex_t mutex;
	int num_frames;
	int index_head;
	int index_tail;
	aframe_t *frames;
};


int frames_queue_create(framesQueue_t **queue, int num_frames)
{
	*queue = (framesQueue_t *)malloc(sizeof(framesQueue_t));
	if( !(*queue) ){
		printf("Fail to create frames queue!\n");
		return -1;
	}

	(*queue)->frames = (aframe_t *)malloc(num_frames * sizeof(aframe_t));
	if(!((*queue)->frames)){
		printf("Fail to create frames!\n");
		return -1;
	}

	(*queue)->num_frames = num_frames;
	(*queue)->index_head = 0;
	(*queue)->index_tail = 0;

	while(num_frames--){
		(*queue)->frames[num_frames].ready = 0;
		(*queue)->frames[num_frames].valid_size = 0;
		(*queue)->frames[num_frames].size = 0;
		(*queue)->frames[num_frames].data = 0;
	}

	pthread_mutex_init(&((*queue)->mutex), NULL);

	return 0;
}

int frames_queue_push(framesQueue_t *queue, unsigned char *buf, unsigned int size)
{
	pthread_mutex_lock(&(queue->mutex));

	if((!queue) || (!buf) || (size==0) ||
		queue->frames[queue->index_tail].ready){
		pthread_mutex_unlock(&(queue->mutex));
		return -1;
	}

	if(queue->frames[queue->index_tail].size < size){
		if(queue->frames[queue->index_tail].data){
			free(queue->frames[queue->index_tail].data);
		}
		queue->frames[queue->index_tail].data = (unsigned char *)malloc(size);
		if(!(queue->frames[queue->index_tail].data)){
			pthread_mutex_unlock(&(queue->mutex));
			return -1;
		}
		queue->frames[queue->index_tail].size = size;
	}
	memcpy(queue->frames[queue->index_tail].data, buf, size);
	queue->frames[queue->index_tail].valid_size = size;
	queue->frames[queue->index_tail].ready = 1;
	queue->index_tail += 1;
	if(queue->index_tail == queue->num_frames){
		queue->index_tail = 0;
	}

	pthread_mutex_unlock(&(queue->mutex));

	return 0;
}

int frames_queue_pop(framesQueue_t *queue, unsigned char **buf, unsigned int *size)
{
	pthread_mutex_lock(&(queue->mutex));

	if(!queue || !(queue->frames[queue->index_head].ready)){
		pthread_mutex_unlock(&(queue->mutex));
		return -1;
	}

	*buf = queue->frames[queue->index_head].data;
	*size = queue->frames[queue->index_head].valid_size;
	queue->frames[queue->index_head].ready = 0;
	queue->index_head += 1;
	if(queue->index_head == queue->num_frames){
		queue->index_head = 0;
	}

	pthread_mutex_unlock(&(queue->mutex));

	return 0;
}


int frames_queue_get_num_valid_frames(framesQueue_t *queue)
{
	int head, tail, num_frames;

	pthread_mutex_lock(&(queue->mutex));
	head = queue->index_head;
	tail = queue->index_tail;
	num_frames = queue->num_frames;
	pthread_mutex_unlock(&(queue->mutex));

	if(head <= tail){
		return tail - head;
	}else{
		return num_frames + tail - head;
	}
}

int frames_queue_destroy(framesQueue_t* queue)
{
	int num_frames;

	pthread_mutex_lock(&(queue->mutex));
	num_frames = queue->num_frames;
	if(!queue){
		pthread_mutex_unlock(&(queue->mutex));
		return 0;
	}

	while(num_frames--){
		free(queue->frames[num_frames].data);
	}
	free(queue->frames);
	free(queue);

	pthread_mutex_unlock(&(queue->mutex));

	return 0;
}





