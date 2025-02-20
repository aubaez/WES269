/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <string.h>
#include <stdio.h>

#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/openthread.h>
#include <openthread/thread.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/posix/arpa/inet.h>
#include <zephyr/posix/unistd.h>
#include <zephyr/posix/sys/socket.h>

LOG_MODULE_REGISTER(child_server, CONFIG_CHILD_SERVER_LOG_LEVEL);

static struct k_work udp_server_work;

#define MAXBUF 65536
int status = 0;
int sock;
struct sockaddr_in6 sin6;
int sin6len;
char buffer[MAXBUF];

static void run_server(struct k_work *item)
{
	ARG_UNUSED(item);
	// TODO: implement server receive loop here
	printf("run_server\n");
	while(1){
		printf(".\n");
		recvfrom(sock, buffer, MAXBUF, 0, 
		     (struct sockaddr *)&sin6, &sin6len);
   		printf("buffer : %s\n", buffer);
	}

}

static void initialize_server(void)
{
	// TODO: implement server initialization here
   printf("initialize_server\n");
   sock = socket(PF_INET6, SOCK_DGRAM,0);

   sin6len = sizeof(struct sockaddr_in6);
   const int port = 4501;
   memset(&sin6, 0, sin6len);

   /* just use the first address returned in the structure */

   sin6.sin6_port = htons(port);
   sin6.sin6_family = AF_INET6;
   sin6.sin6_addr = in6addr_any;

   status = bind(sock, (struct sockaddr *)&sin6, sin6len);
   if(-1 == status){
   	printf("shit!\n");
    perror("bind"), exit(1);
   }
	// status = getsockname(sock, (struct sockaddr *)&sin6, &sin6len);

    printf("%d\n", ntohs(sin6.sin6_port));

	k_work_submit(&udp_server_work);

	/*shutdown(sock, 2);
    close(sock);*/
}

static void on_thread_state_changed(otChangedFlags flags, struct openthread_context *ot_context,
				    void *user_data)
{
	if (flags & OT_CHANGED_THREAD_ROLE) {
		switch (otThreadGetDeviceRole(ot_context->instance)) {
		case OT_DEVICE_ROLE_CHILD:
		case OT_DEVICE_ROLE_ROUTER:
		case OT_DEVICE_ROLE_LEADER:
			LOG_INF("Connected to OpenThread network");
			initialize_server();
			break;

		case OT_DEVICE_ROLE_DISABLED:
		case OT_DEVICE_ROLE_DETACHED:
		default:
			break;
		}
	}
}

static struct openthread_state_changed_cb ot_state_changed_cb = {.state_changed_cb = on_thread_state_changed};

int main(void)
{
	int ret;

	LOG_INF("child-udp-server appplication started");

	k_work_init(&udp_server_work, run_server);

	ret = dk_leds_init();
	if (ret) {
		LOG_ERR("Could not initialize leds, err code: %d", ret);
		goto end;
	}

	openthread_state_changed_cb_register(openthread_get_default_context(), &ot_state_changed_cb);
	openthread_start(openthread_get_default_context());

end:
	return 0;
}
