#include <kernel/OS.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

#include "net/ppp_ctrl.h"

/* Globals */
static thread_id ctrlthd = -1;

#define PROGNAME "PPP"
#define SHOW_DEBUG 1

/* XXX - This needs to be set for the correct modem... */
#define RESET "ATZ\r"
#define ECHO_ON "ATE0\r"
#define INIT_STRING "ATQ0V1F1B0\r"
#define DIAL_STRING "ATDT 08450798879\r"
#define GOOD_CONNECT "CONNECT"

struct speedconv {
	uint32 speed;
	uint8  constant;
} speed_table[] = {
	{      0, B0 },
	{     50, B50 },
	{     75, B75 },
	{    110, B110 },
	{    134, B134 },
	{    150, B150 },
	{    200, B200 },
	{    300, B300 },
	{    600, B600 },
	{   1200, B1200 },
	{   1800, B1800 },
	{   2400, B2400 },
	{   4800, B4800 },
	{   9600, B9600 },
	{  19200, B19200 },
	{  31250, B31250 },
	{  38400, B38400 },
	{  57600, B57600 },
	{ 115200, B115200 },
	{ 230400, B230400 }
};

void usage(void)
{
	printf("ppp - ppp control app for OpenBeOS\n"
	       "usage: ppp <device> <speed> <options>\n"
	       "e.g. ppp /dev/ports/serial1 115200\n"
	       "options:\n"
	       "        nodial - don't try to use a modem, direct connection\n");

	exit(-1);
}

int open_port(char *port_name, uint32 port_speed)
{
	int fd = 0, rv = 0;
	struct termios options;
	uint8 speed = B19200;
	
	if (port_speed > 0) {
		int i;
		for (i=0; i < sizeof(speed_table) / sizeof(speed_table[0]); i++ ) {
			if (speed_table[i].speed == port_speed)
				speed = speed_table[i].constant;
		}
	}
	
	fd = open(port_name, O_RDWR | O_NONBLOCK | O_NOCTTY);
	if (fd < 0)
		return fd;

	fcntl(fd, F_SETFL, 0);
		
	tcgetattr(fd, &options);
	options.c_cflag &= ~CBAUD;
	options.c_cflag |= speed;
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;
	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 10;
	
	rv = tcsetattr(fd, TCSANOW, &options);

#if SHOW_DEBUG
	printf(PROGNAME ": port %s has been setup.\n", port_name);
#endif

	return fd;
}

int check_read(int fd, const char *good, const char *alt)
{
	char buff[80];
	int rv, i;
	int len = 80;
	
	memset(buff, 0, 80);
	while (strstr(buff, "\r") == NULL && len > 0) {
		rv = read(fd, &buff[80 - len], len);
		len -= rv;
	}
	len = 80 - len;

	for (i=0;i<len - 1;i++) {
		if (buff[i] < 0x20)
			buff[i] = 0x20;
	}
printf("%s\n", buff);	
	if (rv < 0)
		return 0;
	if (strstr(buff, good) != NULL || strncmp(buff, good, strlen(good)) == 0)
		return 1;
	else if (alt && strstr(buff, alt) != NULL)
		return 2;
	return 0;
}

int check_modem(int fd)
{
	int rv, echo_on = 0;
	
#if SHOW_DEBUG
	printf(PROGNAME ": resetting modem (fd = %d) using %s\n", fd, RESET);
#endif

	rv = write(fd, RESET, strlen(RESET));
	if (rv < 0) {
		printf(PROGNAME ": write failed for reset [%d:%s]\n", errno, strerror(errno));
		return -1;
	}
	rv = check_read(fd, "ATZ", "OK");
	if (rv == 0)
		return -1;

	if (rv == 2) {
		echo_on = 1;
		check_read(fd, "OK", NULL);
	}
	
#if SHOW_DEBUG
	printf(PROGNAME ": turning off echo using %s\n", ECHO_ON);
#endif

	rv = write(fd, ECHO_ON, strlen(ECHO_ON));
	if (rv < 0)
		return -1;
	if (echo_on) {
		rv = check_read(fd, ECHO_ON, NULL);
		if (rv != 1)
			return -1;
	}
	rv = check_read(fd, "OK", NULL);
	if (rv == 0)
		return -1;
	
#if SHOW_DEBUG
	printf(PROGNAME ": init'ing modem using %s\n", INIT_STRING);
#endif

	rv = write(fd, INIT_STRING, strlen(INIT_STRING));
	if (rv < 0)
		return -1;
	
	if ((rv = check_read(fd, INIT_STRING, NULL)) != 1) {
		return -1;
	}
	rv = check_read(fd, "OK", NULL);
	if (rv == 1)
		return 0;

	return -1;
}

int dial_modem(int fd)
{
	int rv;

#if SHOW_DEBUG
	printf(PROGNAME ": dialling %s\n", DIAL_STRING);
#endif

	rv = write(fd, DIAL_STRING, strlen(DIAL_STRING));
	if (rv < 0) {
		return -1;
	}
	rv = check_read(fd, DIAL_STRING, "CONNEC");
	if (rv == 0)
		return -1;

	if (rv == 1) {
		rv = check_read(fd, "CONNEC", NULL);
		if (rv != 1)
			return -1;
	}
	
	return 0;
}

/* XXX - This is just a hack to get us working, needs
 * to be ripped out and replaced by something that works
 * with scripts - chat for instance.
 */
int modem_login(int fd, char *username, char *pw)
{
	char buffer[80];
	int rv;

#if SHOW_DEBUG
	printf(PROGNAME ": trying to login\n");
#endif
	
	while (strstr(buffer, "gin:") == NULL && rv >= 0) {
		memset(buffer, 0, 80);
		rv = read(fd, buffer, 80);
		if (rv > 0)
			printf ("%s", buffer);
	}
	printf("\nsending username\n");
	write(fd, username, strlen(username));
	write(fd, "\r", 1);
	rv = check_read(fd, "ord:", username);
	if (rv == 0) {
		printf("didn't get password prompt\n");
		return -1;
	}
	
	printf("sending password\n");
	write(fd, pw, strlen(pw));
	write(fd, "\r", 1);
	check_read(fd, pw, NULL);

printf("done!\n");

	while (strstr(buffer, "PPP") == NULL && rv >= 0) {
		memset(buffer, 0, 80);
		rv = read(fd, buffer, 80);
		printf ("%s", buffer);
	}

	return 0;
}

int main(int argc, char **argv)
{
	char data[PPPCTRL_MAXSIZE];
	int32 code = PPPCTRL_CREATE;
	int rv, fd = 0;
	char thread_name[B_OS_NAME_LENGTH];
	thread_id sender, conn;
	char *port;
	uint32 speed = 0;
	int modem = 1;
	
	/* process command line args... 
	 * XXX - This needs to be done correctly :)
	 */
	if (argc < 2)
		usage();

	ctrlthd = find_thread(PPPCTRL_THREADNAME);
	if (ctrlthd < 0) {
		printf(PROGNAME ": could not connect to PPP controller. Is it running?\n");
		exit(-1);
	}

#if SHOW_DEBUG
	printf(PROGNAME ": found the PPP Control thread.\n");		
#endif

	port = strdup(argv[1]);
	printf(PROGNAME ": port to use is %s\n", port);
	if (argc > 2)
		speed = atol(argv[2]);

	if (argc >= 3) {
		printf("argv[3] = %s\n", argv[3]);
		if (strcmp(argv[3], "nodial") == 0) 
			modem = 0;
	}
	
#if SHOW_DEBUG
	printf(PROGNAME ": port speed %ld\n", speed);
#endif

	fd = open_port(port, speed);
	if (fd < 0) {
		printf(PROGNAME ": failed to open %s [%d:%s]\n", port, 
		       errno, strerror(errno));
		exit (-1);
	}
	
	if (modem) {
		rv = check_modem(fd);
		if (rv < 0) {
			printf(PROGNAME ": modem failed check\n");
			close(fd);
			exit(-1);
		}
	
		if (dial_modem(fd) < 0) {
			close(fd);
			exit(-1);
		}
		rv = modem_login(fd, "xxxxxxxx", "yyyyyyyy");
	}
		
	rv = send_data(ctrlthd, code, port, strlen(port));
	if (rv < 0) {
		printf("write_port failed!\n");
		exit(-1);
	}
	printf("written request, awaiting response\n");
	code = receive_data(&sender, &data, PPPCTRL_MAXSIZE);
	printf("thread [%ld] replied with %ld [%s]\n", sender, code, data);


	sprintf(thread_name, "%s_control_thread", data);
	conn = find_thread(thread_name);
	printf("conn = %ld\n", conn);
	if (conn > 0) {
		printf("sending data to thread %ld\n", conn);
		rv = send_data(conn, PPPCTRL_UP, NULL, 0);
		printf("rv = %d\n", rv);
	}
	code = receive_data(&sender, &data, PPPCTRL_MAXSIZE);
	
	printf("thread [%ld] replied with %ld [%s]\n", sender, code, data);

	
	return 0;
}

