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
char *socket_path = (char*)"\0hidden";

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

	// Entra en comunicacion
	while (true)
	{

		cout << ">";
		cin >> cmd;

		// Revisa si recibe "quit" el cual sale del while.
		if (cmd == "quit")
		{
			write(fd, cmd.c_str(), sizeof(cmd));
			cout << "quiting" << endl;
			break;
		}

		// Si recibe el comando de "connect" intenta conectarse
		else if (cmd == "connect")
		{
			// En caso de error de socket: 
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

			// En caso de error de coneccion.
			if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
			{
				cout << "connect error" << endl;
				exit(-1);
			}
			else
			{
				cout << "connection success" << endl;
			}
		}
		// Si recibe el comando de "disconnect" cierra la comunicacion
		else if (cmd == "disconnect")
		{
			if (fd > 0)
			{
				close(fd);
				cout << "disconnection success" << endl;
			}
		}
		// Si recibe los demás comandos, se escribe y responde al servidor.
		else if ((strncmp(cmd.c_str(), "insert", 6) == 0 && strlen(cmd.c_str()) > 6) || (strncmp(cmd.c_str(), "get", 3) == 0 && strlen(cmd.c_str()) > 3) || (strncmp(cmd.c_str(), "peek", 4) == 0 && strlen(cmd.c_str()) > 4) || (strncmp(cmd.c_str(), "update", 6) == 0 && strlen(cmd.c_str()) > 6) || (strncmp(cmd.c_str(), "delete", 6) == 0 && strlen(cmd.c_str()) > 6) || cmd == "list")
		{
			write(fd, cmd.c_str(), sizeof(cmd));
			char tempbuff[1024] = { 0 };
			read(fd, tempbuff, 1024);
			cout << tempbuff << endl;
		}
	}
	return 0;
}
