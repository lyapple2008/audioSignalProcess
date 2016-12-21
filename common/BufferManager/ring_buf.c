/*
 * ring_buf.c
 *
 *  Created on: Jul 14, 2016
 *      Author: marshall
 */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "ring_buf.h"

//#define _DEBUG_RINGBUF

#ifdef _DEBUG_RINGBUF
#include <stdio.h>
#endif

#define RINGBUF_START(pbuf) ((unsigned char *)(pbuf->buf_start))
#define RINGBUF_END(pbuf) (pbuf->buf_start+pbuf->buf_size-1)
#define RINGBUF_HEAD(pbuf) (pbuf->buf_head)
#define RINGBUF_TAIL(pbuf) (pbuf->buf_tail)
#define RINGBUF_SIZE(pbuf) (pbuf->buf_size)
#define RINGBUF_USED(pbuf)  (RINGBUF_HEAD(pbuf) <= RINGBUF_TAIL(pbuf) ? \
							RINGBUF_TAIL(pbuf) - RINGBUF_HEAD(pbuf) : \
							RINGBUF_SIZE(pbuf)+RINGBUF_TAIL(pbuf)-RINGBUF_HEAD(pbuf))

#define RINGBUF_FREE(pbuf) (RINGBUF_HEAD(pbuf) <= RINGBUF_TAIL(pbuf) ? \
							RINGBUF_SIZE(pbuf)+RINGBUF_HEAD(pbuf) - RINGBUF_TAIL(pbuf) : \
							RINGBUF_HEAD(pbuf) - RINGBUF_TAIL(pbuf))


struct ringbuf_t{
	pthread_mutex_t mutex;
	unsigned int	buf_size;
	unsigned char	*buf_head; // consumer
	unsigned char	*buf_tail; // producer
	unsigned char	buf_start[1];
};

#ifdef _DEBUG_RINGBUF
static void print_ringbuf_info(ringbuf *buf)
{
	printf("ringbuf info: buf_size = %d\n", buf->buf_size);
	printf("			  buf_head = 0x%x\n", buf->buf_head);
	printf("			  buf_tail = 0x%x\n", buf->buf_tail);
	printf("			  buf_uesed = %d\n", RINGBUF_USED(buf));
	printf("			  buf_free = %d\n", RINGBUF_FREE(buf));
}
#endif

int ringbuf_create(ringbuf **pbuf, unsigned int size)
{
	if(size > 0){
		*pbuf = malloc(sizeof(ringbuf) + size);
		if(!(*pbuf)){
			return -1;
		}
		(*pbuf)->buf_size = size;
		(*pbuf)->buf_head = (*pbuf)->buf_tail = (*pbuf)->buf_start;
		pthread_mutex_init(&((*pbuf)->mutex), NULL);
		return 0;
	}

	return -1;
}

int ringbuf_push(ringbuf *pbuf, const void *data, int size)
{
	pthread_mutex_lock(&(pbuf->mutex));
	if(pbuf && data && (size>0) && (size<(int)RINGBUF_FREE(pbuf))){
		unsigned int tail_free = RINGBUF_END(pbuf) - RINGBUF_TAIL(pbuf) + 1;
		if(tail_free >= size){
			memcpy(RINGBUF_TAIL(pbuf), (unsigned char *)data, size);
			RINGBUF_TAIL(pbuf) += size;
			if(RINGBUF_TAIL(pbuf) > RINGBUF_END(pbuf)){
				RINGBUF_TAIL(pbuf) = RINGBUF_START(pbuf);
			}
		}else{
			memcpy(RINGBUF_TAIL(pbuf), (unsigned char *)data, tail_free);
			unsigned int lev = size - tail_free;
			memcpy(RINGBUF_START(pbuf), (unsigned char *)data + tail_free, lev);
			RINGBUF_TAIL(pbuf) = RINGBUF_START(pbuf) + lev;
		}
#ifdef _DEBUG_RINGBUF
		print_ringbuf_info(pbuf);
#endif
		pthread_mutex_unlock(&(pbuf->mutex));
		return size;
	}
#ifdef _DEBUG_RINGBUF
		print_ringbuf_info(pbuf);
#endif
	pthread_mutex_unlock(&(pbuf->mutex));
	return 0;
}

int ringbuf_pop(ringbuf *pbuf, void *data, int size)
{
	pthread_mutex_lock(&(pbuf->mutex));
	if(pbuf && data && (size>0) && (size<(int)RINGBUF_USED(pbuf))){
		unsigned int head_used = RINGBUF_END(pbuf) - RINGBUF_HEAD(pbuf) + 1;
		if(head_used >= size){
			memcpy((unsigned char *)data, RINGBUF_HEAD(pbuf), size);
			RINGBUF_HEAD(pbuf) += size;
			if(RINGBUF_HEAD(pbuf) > RINGBUF_END(pbuf)){
				RINGBUF_HEAD(pbuf) = RINGBUF_END(pbuf);
			}
		}else{
			memcpy((unsigned char *)data, RINGBUF_HEAD(pbuf), head_used);
			unsigned int lev = size - head_used;
			memcpy((unsigned char *)data+head_used, RINGBUF_START(pbuf), lev);
			RINGBUF_HEAD(pbuf) = RINGBUF_START(pbuf) + lev;
		}
#ifdef _DEBUG_RINGBUF
		print_ringbuf_info(pbuf);
#endif
		pthread_mutex_unlock(&(pbuf->mutex));
		return size;
	}
#ifdef _DEBUG_RINGBUF
		print_ringbuf_info(pbuf);
#endif
	pthread_mutex_unlock(&(pbuf->mutex));
	return 0;
}

int ringbuf_get_size(ringbuf *pbuf)
{
	return pbuf->buf_size;
}

int ringbuf_get_used_size(ringbuf *pbuf)
{
	int used_size;

	pthread_mutex_lock(&(pbuf->mutex));
	if(pbuf->buf_head <= pbuf->buf_tail){
		used_size = pbuf->buf_tail - pbuf->buf_head;
	}else{
		used_size = pbuf->buf_size + pbuf->buf_tail - pbuf->buf_head;
	}
	pthread_mutex_unlock(&(pbuf->mutex));

	return used_size;
}

int ringbuf_get_free_size(ringbuf *pbuf)
{
	int free_size;

	pthread_mutex_lock(&(pbuf->mutex));
	if(pbuf->buf_head <= pbuf->buf_tail){
		free_size = pbuf->buf_size + pbuf->buf_head - pbuf->buf_tail;
	}else{
		free_size = pbuf->buf_head - pbuf->buf_tail;
	}
	pthread_mutex_unlock(&(pbuf->mutex));

	return free_size;
}

void ringbuf_destroy(ringbuf *pbuf)
{
	free(pbuf);
}


