// SPDX-License-Identifier: BSD-3-Clause
#include <stdlib.h>
#include <pthread.h>
#include "ring_buffer.h"

int ring_buffer_init(so_ring_buffer_t *ring, size_t cap)
{
	/* TODO: implement ring_buffer_init */
	(void) ring;
	(void) cap;

	ring->data = malloc(cap);
	if (!ring->data)
		return -1;

	ring->cap = cap;
	ring->read_pos = 0;
	ring->write_pos = 0;
	ring->len = 0;
	ring->stopped = 0;

	if (pthread_mutex_init(&ring->mutex, NULL) != 0) {
		free(ring->data);
		return -1;
	}

	if (pthread_cond_init(&ring->not_empty, NULL) != 0) {
		pthread_mutex_destroy(&ring->mutex);
		free(ring->data);
		return -1;
	}

	if (pthread_cond_init(&ring->not_full, NULL) != 0) {
		pthread_cond_destroy(&ring->not_empty);
		pthread_mutex_destroy(&ring->mutex);
		free(ring->data);
		return -1;
	}

	return 0;
}

ssize_t ring_buffer_enqueue(so_ring_buffer_t *ring, void *data, size_t size)
{
	/* TODO: implement ring_buffer_enqueue */
	(void) ring;
	(void) data;
	(void) size;

	pthread_mutex_lock(&ring->mutex);

	while (ring->len + size > ring->cap)
		pthread_cond_wait(&ring->not_full, &ring->mutex);
	memcpy(ring->data + ring->write_pos, data, size);
	ring->write_pos = (ring->write_pos + size) % ring->cap;
	ring->len += size;

	pthread_cond_signal(&ring->not_empty);
	pthread_mutex_unlock(&ring->mutex);

	return size;
}

ssize_t ring_buffer_dequeue(so_ring_buffer_t *ring, void *data, size_t size)
{
	/* TODO: Implement ring_buffer_dequeue */
	(void) ring;
	(void) data;
	(void) size;

	pthread_mutex_lock(&ring->mutex);
	while (ring->len < size && !ring->stopped)
		pthread_cond_wait(&ring->not_empty, &ring->mutex);

	if (ring->len == 0 && ring->stopped) {
		pthread_mutex_unlock(&ring->mutex);
		return 0;
	}

	memcpy(data, ring->data + ring->read_pos, size);
	ring->read_pos = (ring->read_pos + size) % ring->cap;
	ring->len -= size;
	pthread_cond_signal(&ring->not_full);
	pthread_mutex_unlock(&ring->mutex);
	return size;
}

void ring_buffer_destroy(so_ring_buffer_t *ring)
{
	/* TODO: Implement ring_buffer_destroy */
	(void) ring;
	pthread_mutex_destroy(&ring->mutex);
	pthread_cond_destroy(&ring->not_empty);
	pthread_cond_destroy(&ring->not_full);
	free(ring->data);
}

void ring_buffer_stop(so_ring_buffer_t *ring)
{
	/* TODO: Implement ring_buffer_stop */
	(void) ring;
	pthread_mutex_lock(&ring->mutex);
	ring->stopped = 1;
	pthread_cond_broadcast(&ring->not_empty);
	pthread_mutex_unlock(&ring->mutex);
}
