/*
 * ring_buf.h
 *
 *  Created on: Jul 14, 2016
 *      Author: marshall
 */

#ifndef BUF_MAG_RING_BUF_H_
#define BUF_MAG_RING_BUF_H_

#ifdef _cplusplus
extern "C"{
#endif


typedef struct ringbuf_t ringbuf;

/**
 *
 */
int ringbuf_create(ringbuf **pbuf, unsigned int size);

/**
 * @return 1:success 0:fail or not enough space
 */
int ringbuf_push(ringbuf *pbuf, const void *data, int size);

/**
 *
 */
int ringbuf_pop(ringbuf *pbuf, void *data, int size);

/**
 *
 */
int ringbuf_get_size(ringbuf *pbuf);

/**
 *
 */
int ringbuf_get_used_size(ringbuf *pbuf);

/**
 *
 */
int ringbuf_get_free_size(ringbuf *pbuf);

/**
 *
 */
void ringbuf_destroy(ringbuf *pbuf);

#ifdef _cplusplus
}
#endif

#endif /* BUF_MAG_RING_BUF_H_ */
