/*
 * frames_buf.h
 *
 *  Created on: Jul 15, 2016
 *      Author: marshall
 */

// frames buffer is used to keep the pcm data will be played

#ifndef BUF_MAG_FRAMES_QUEUE_H_
#define BUF_MAG_FRAMES_QUEUE_H_

typedef struct aframe aframe_t;
typedef struct framesQueue framesQueue_t;

/**
 * @params num_frames the size of framesQueue
 */
int frames_queue_create(framesQueue_t **queue, int num_frames);

/**
 * @params buf the data that need be pushed in framesQueue
 * @params size the size of data
 */
int frames_queue_push(framesQueue_t *queue, unsigned char *buf, unsigned int size);

/**
 * @params buf output buffer
 * @params size will be set to the number of samples in current frame
 * @return
 */
int frames_queue_pop(framesQueue_t *queue, unsigned char **buf, unsigned int *size);

/**
 * @return the number of valid frames in queue
 */
int frames_queue_get_num_valid_frames(framesQueue_t *queue);

/**
 *
 */
int frames_queue_destroy(framesQueue_t* queue);

#endif /* BUF_MAG_FRAMES_QUEUE_H_ */
