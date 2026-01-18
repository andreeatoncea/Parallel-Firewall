// SPDX-License-Identifier: BSD-3-Clause

#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "consumer.h"
#include "ring_buffer.h"
#include "packet.h"
#include "utils.h"


void *consumer_thread(so_consumer_ctx_t *ctx)
{
	/* TODO: implement consumer thread */
	so_packet_t pkt;
	ssize_t length;
	unsigned long my_ticket;
	char output_buffer[256];
	int output_length;

	while (1) {
		pthread_mutex_lock(&ctx->mutex_fetch);
		length = ring_buffer_dequeue(ctx->producer_rb, (void **)&pkt, sizeof(so_packet_t));
		if (length <= 0) {
			pthread_mutex_unlock(&ctx->mutex_fetch);
			break;
		}

		my_ticket = ctx->next_ticket++;
		pthread_mutex_unlock(&ctx->mutex_fetch);

		int action = process_packet(&pkt);
		unsigned long hash = packet_hash(&pkt);
		unsigned long timestamp = pkt.hdr.timestamp;

		pthread_mutex_lock(&ctx->mutex_write);
		while (my_ticket != ctx->next_to_write)
			pthread_cond_wait(&ctx->cond_write, &ctx->mutex_write);

		output_length = snprintf(output_buffer, sizeof(output_buffer), "%s %016lx %lu\n",
		RES_TO_STR(action), hash, timestamp);
		write(ctx->output_fd, output_buffer, output_length);
		ctx->next_to_write++;
		pthread_cond_broadcast(&ctx->cond_write);
		pthread_mutex_unlock(&ctx->mutex_write);
	}

	//(void) ctx;
	return NULL;
}

int create_consumers(pthread_t *tids,
					 int num_consumers,
					 struct so_ring_buffer_t *rb,
					 const char *out_filename)
{
	(void) tids;
	(void) num_consumers;
	(void) rb;
	(void) out_filename;

	so_consumer_ctx_t *ctx = malloc(sizeof(so_consumer_ctx_t));

	ctx->output_fd = open(out_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	ctx->next_ticket = 0;
	ctx->next_to_write = 0;

	pthread_mutex_init(&ctx->mutex_fetch, NULL);
	pthread_mutex_init(&ctx->mutex_write, NULL);
	pthread_cond_init(&ctx->cond_write, NULL);

	ctx->producer_rb = rb;
	for (int i = 0; i < num_consumers; i++) {
		/*
		 * TODO: Launch consumer threads
		 **/
		pthread_create(&tids[i], NULL, (void *)consumer_thread, (void *)ctx);
	}

	return num_consumers;
}
