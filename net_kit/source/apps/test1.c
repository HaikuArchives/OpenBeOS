#include <stdio.h>
#include <kernel/OS.h>
#include <string.h>
#include <sys/time.h>

#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "sys/select.h"

#define THREADS	2
#define TIME	10

int32 test_thread(void *data)
{
	int tnum = *(int*)data;
	int sock = 0;
	uint32 num = 0;
	bigtime_t tn;

	printf("Thread %d, starting test...\n", tnum + 1);

	tn = real_time_clock();

	while (real_time_clock() - tn <= TIME) {
		sock = socket(AF_INET, SOCK_DGRAM , 0);
		if (sock < 0) {
			printf("Failed! Socket could not be created.\n");
			printf("Error was %d [%s]\n", sock, strerror(sock));
			printf("This was after I had created %ld socket%s\n",
				num, num == 1 ? "" : "s");
			return -1;
		}
		closesocket(sock);
		num++;
	}

	printf( "Thread %d:\n"
		"       sockets created : %5ld\n"
		"       test time       : %5d seconds\n"
		"       average         : %5ld sockets/sec\n",
		tnum + 1, num, TIME, num / TIME);
}

#define TEST_PHRASE "Hello loopback!"
	
int main(int argc, char **argv)
{
	thread_id t[THREADS];
	int s, ls;
	int i, rv;
	status_t retval;
	struct sockaddr_in sa;
	struct sockaddr_in sb;
	char buffer[100];
	struct fd_set fds;
	struct timeval tv;
		
	for (i=0;i<THREADS;i++) {
		t[i] = spawn_thread(test_thread, "socket test thread", 
			B_NORMAL_PRIORITY, &i);
		if (t[i] >= 0)
			resume_thread(t[i]);
	}

	for (i=0;i<THREADS;i++) {
		wait_for_thread(t[i], &retval);
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_LOOPBACK;
	sa.sin_port = htons(7777);
	sa.sin_len = sizeof(sa);

	printf("opening loopback socket...\n");
	if ((ls = socket(AF_INET, SOCK_DGRAM, 0)) > 0) {
		printf("loopback socket is %d\n", ls);
		i = bind(ls, (struct sockaddr*)&sa, sizeof(sa));
		if (i < 0) {
			printf("bind failed! %d [%s]\n", i, strerror(i));
			exit(-1);
		}
		printf("successfully bound to port 7777\n");
	} else {
		printf("Failed to get a socket.\n");
		printf("%d [%s]\n", i, strerror(i));
		exit(-1);
	}		
	printf("opened the loopback and bound it OK.\n");
	
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(7772);

	s = socket(AF_INET, SOCK_DGRAM, 0);
	i = bind(s, (struct sockaddr *)&sa, sizeof(sa));
	printf("bind = %d\n", i);

	sa.sin_port = htons(7777);
	sa.sin_addr.s_addr = INADDR_LOOPBACK;
	i = sendto(s, TEST_PHRASE, strlen(TEST_PHRASE), 0,
		(struct sockaddr*)&sa, sizeof(sa));
	printf("sendto gave %d\n", i);
	
	i = recvfrom(ls, buffer, 100, 0, (struct sockaddr*)&sb, sizeof(sb));
	printf("recvfrom gave %d\n", i);
	printf("%s\n", i > 0 ? buffer : strerror(i));
	printf("data came from %s:%d\n", inet_ntoa(sb.sin_addr),
		ntohs(sb.sin_port));
		
	FD_ZERO(&fds);
	FD_SET(s, &fds);
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	
	rv = net_select(s +1, &fds, NULL, NULL, &tv);
	printf("select gave %d\n", rv);
	rv = net_select(s +1, NULL, &fds, NULL, &tv);
	printf("select gave %d\n", rv);
	rv = net_select(s +1, NULL, NULL, &fds, &tv);
	printf("select gave %d\n", rv);
	
	closesocket(s);
	closesocket(ls);
	
	printf("Test complete.\n");

	return (0);
}

	
