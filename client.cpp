#include <iostream>
#include <stdio.h>
#include <memory>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "util.h"
#include <stdlib.h>
using namespace std;
char *socket_path = (char*)
"\0hidden";

int main(int argc, char **argv)
{
	struct sockaddr_un addr;
	int fd;

	int opt;

	// Procesar opciones de linea de comando
	while ((opt = getopt(argc, argv, "s:")) != -1)
	{
		switch (opt)
		{ /*Procesar el flag s si el usuario lo ingresa */
			case 's':
				socket_path = argv[2];
				break;
			default:
				cout << "input invalido" << endl;
				return 0;
		}
	}
	string cmd = "";

	while (cmd != "quit")
	{

		cout << ">";
		cin >> cmd;
		if (cmd == "quit")
		{
			write(fd, cmd.c_str(), sizeof(cmd));
			cout << "quiting" << endl;
			break;
		}
		else if (cmd == "connect")
		{
			if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
			{
				cout << "socket error" << endl;
				continue;
			}

			memset(&addr, 0, sizeof(addr));
			addr.sun_family = AF_UNIX;
			if (*socket_path == '\0')
			{ 	*addr.sun_path = '\0';
				socket_path = (char*)
				"/tmp/db.tuples.sock";
				strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
			}
			else
			{
				strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
			}

			if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
			{
				cout << "connect error" << endl;
				exit(-1);
			}
			else
			{
				cout << "connection success" << endl;
			}
			continue;
		}
		else if (cmd == "disconnect")
		{
			if (fd > 0)
			{
				close(fd);
				cout << "disconnection success" << endl;
			}
			continue;
		}
		else if ((strncmp(cmd.c_str(), "insert", 6) == 0 && strlen(cmd.c_str()) > 6) || (strncmp(cmd.c_str(), "get", 3) == 0 && strlen(cmd.c_str()) > 3) || (strncmp(cmd.c_str(), "peek", 4) == 0 && strlen(cmd.c_str()) > 4) || (strncmp(cmd.c_str(), "update", 6) == 0 && strlen(cmd.c_str()) > 6) || (strncmp(cmd.c_str(), "delete", 6) == 0 && strlen(cmd.c_str()) > 6) || cmd == "list")
		{
			write(fd, cmd.c_str(), sizeof(cmd));
			char tempbuff[1024] = { 0 };
			read(fd, tempbuff, 1024);
			cout << tempbuff << endl;
			continue;
		}
		else
		{
			cout << "invalid input" << endl;
		}
		continue;
	}
	return 0;
}
