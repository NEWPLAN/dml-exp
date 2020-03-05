#include <stdio.h>
#include "client.h"

static void userHelp(char *str)
{
	printf("%s [server_ip] [server_port]", str);
}
int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		userHelp(argv[0]);
		return 1;
	}

	TCPClient client;
	client.connect_to_server(argv[1], atoi(argv[2]));
	client.start_service();
	return 0;
}