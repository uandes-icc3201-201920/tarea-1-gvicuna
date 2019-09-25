#include <iostream>
#include <sstream>
#include <stdio.h>
#include <memory>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <string>
#include <pthread.h>
#include <time.h>
using namespace std;

// Almacenamiento KV
KVStore db;

char client_message[2000];
int fd, cl, rc;
int sflag = 0;
int opt;
int ex = 0;
int randk = 0;
pthread_t tid[60];
int i = 0;
string primerNumero;
string segundoNumero;
struct arg_struct
{
	pthread_rwlock_t * arg1;
	int arg2;
};

pthread_rwlock_t lock_rw = PTHREAD_RWLOCK_INITIALIZER;

int GenerateRandomNumber()
{
	int randomKey;
	return randomKey = rand() % 9001 + 1000;
}

void *socketThread(void *args)
{
	char buf[1024];
	struct arg_struct *arg = (struct arg_struct *) args;
	int newSocket = arg->arg2;
	pthread_t thId = pthread_self();
	std::cout << "Thread Id from thread function : " << thId << std::endl;
	pthread_rwlock_t *p = (pthread_rwlock_t*) arg->arg1;
	while ((rc = read(newSocket, buf, sizeof(buf))) > 0)
	{
		if (strncmp(buf, "quit", 4) == 0 && strlen(buf) == 4)
		{
			cout << "quiting..." << endl;
			ex = 1;
			break;
		}
		printf("%.*s\n", rc, buf);
		string copy(buf);
		//Identifica los valores a y b de insert(a,b)
		pthread_t thId = pthread_self();
		std::cout << "Thread Id from thread function : " << thId << std::endl;
		if (strncmp(buf, "insert", 6) == 0)
		{
			string parentesis = copy.substr(6);
			int posComa = parentesis.find(',');
			if (posComa != string::npos)
			{
				int posUltimoParentesis = parentesis.find(')');
				string primerNumero = parentesis.substr(1, posComa - 1);
				string segundoNumero = parentesis.substr(posComa + 1, (posUltimoParentesis - posComa - 1));
				char cstr[primerNumero.size() + 1];
				strcpy(cstr, primerNumero.c_str());
				Value valuee = { segundoNumero.size(), segundoNumero
				};
				unsigned long key = strtoul(cstr, NULL, 0);
				pthread_rwlock_wrlock(p);
				db.insert(std::pair < unsigned long, Value> (key, valuee));
				*buf = '\0';
				pthread_rwlock_unlock(p);
				pthread_rwlock_rdlock(p);
				string msg = "Se guardo el valor " + db[key].data + " en la key " + primerNumero;
				send(newSocket, msg.c_str(), strlen(msg.c_str()), 0);
				pthread_rwlock_unlock(p);
			}
			else
			{
				int ultParentesis = parentesis.find(')');
				string primerNumero = parentesis.substr(1, ultParentesis - 1);
				Value valuee = { primerNumero.size(), primerNumero };
				if (randk == 0)
				{
					randk = GenerateRandomNumber();
				}
				else
				{
					randk++;
				}
				//adentro de ese lock abajo tenique subir el contador
			    	pthread_rwlock_wrlock(p);
				db.insert(std::pair < unsigned long, Value> ((unsigned long) randk, valuee));
				stringstream ss;
				ss << randk;
				string out_string = ss.str();
				string msg = "Se guardo el valor " + db[(unsigned long) randk].data + " en la key " + out_string;
				send(newSocket, msg.c_str(), strlen(msg.c_str()), 0);
				pthread_rwlock_unlock(p);
				*buf = '\0';
			}
		}
		//Identifica el valor a de get(a)
		else if (strncmp(buf, "get", 3) == 0)
		{
			string parentesis = copy.substr(3);
			int ultParentesis = parentesis.find(')');
			string primerNumero = parentesis.substr(1, ultParentesis - 1);
			char cstr2[primerNumero.size() + 1];
			strcpy(cstr2, primerNumero.c_str());
			unsigned long key = strtoul(cstr2, NULL, 0);
			map < unsigned long, Value>::iterator it;
			pthread_rwlock_rdlock(p);
			it = db.find(key);
			if (it == db.end())
			{
				string msg = "No existe un value asociado a esa key";
				send(newSocket, msg.c_str(), strlen(msg.c_str()), 0);
			}
			else
			{
				string msg = "El valor asociado al key " + primerNumero + " es: " + db[key].data;
				send(newSocket, msg.c_str(), strlen(msg.c_str()), 0);
			}
			pthread_rwlock_unlock(p);
			*buf = '\0';
		}
		//Identifica el valor a de peek(a)
		else if (strncmp(buf, "peek", 4) == 0)
		{
			string parentesis = copy.substr(4);
			int ultParentesis = parentesis.find(')');
			string primerNumero = parentesis.substr(1, ultParentesis - 1);
			map < unsigned long, Value>::iterator it;
			char cstr2[primerNumero.size() + 1];
			strcpy(cstr2, primerNumero.c_str());
			unsigned long key = strtoul(cstr2, NULL, 0);
			pthread_rwlock_rdlock(p);
			it = db.find(key);
			if (it == db.end())
			{
				string msg = "false";
				send(newSocket, msg.c_str(), strlen(msg.c_str()), 0);
			}
			else
			{
				string msg = "true";
				send(newSocket, msg.c_str(), strlen(msg.c_str()), 0);
			}
			pthread_rwlock_unlock(p);
			*buf = '\0';
		}
		//Identifica el valor a y b de update(a,b)
		else if (strncmp(buf, "update", 6) == 0)
		{
			string parentesis = copy.substr(6);
			int posComa = parentesis.find(',');
			int posUltimoParentesis = parentesis.find(')');
			string primerNumero = parentesis.substr(1, posComa - 1);
			string segundoNumero = parentesis.substr(posComa + 1, (posUltimoParentesis - posComa - 1));
			char cstr2[primerNumero.size() + 1];
			map < unsigned long, Value>::iterator it;
			strcpy(cstr2, primerNumero.c_str());
			unsigned long key = strtoul(cstr2, NULL, 0);
			pthread_rwlock_wrlock(p);
			it = db.find(key);
			if (it == db.end())
			{
				string msg = "Key no existe en la base de datos";
				send(newSocket, msg.c_str(), strlen(msg.c_str()), 0);
			}
			else
			{
				Value valuee = { segundoNumero.size(), segundoNumero
				};
				db[key] = valuee;
				string msg = "Valor actualizado";
				send(newSocket, msg.c_str(), strlen(msg.c_str()), 0);
			}
			*buf = '\0';
			pthread_rwlock_unlock(p);
		}
		//Identifica el valor a de delete(a)
		else if (strncmp(buf, "delete", 6) == 0)
		{
			string parentesis = copy.substr(6);
			int ultParentesis = parentesis.find(')');
			string primerNumero = parentesis.substr(1, ultParentesis - 1);
			map < unsigned long, Value>::iterator it;
			char cstr2[primerNumero.size() + 1];
			strcpy(cstr2, primerNumero.c_str());
			unsigned long key = strtoul(cstr2, NULL, 0);
			pthread_rwlock_wrlock(p);
			it = db.find(key);
			if (it == db.end())
			{
				string msg = "No existe";
				send(newSocket, msg.c_str(), strlen(msg.c_str()), 0);
			}
			else
			{
				db.erase(key);
				string msg = "Eliminado";
				send(newSocket, msg.c_str(), strlen(msg.c_str()), 0);
			}
			*buf = '\0';
			pthread_rwlock_unlock(p);
		}
		//Retorna list de keys
		else if (strncmp(buf, "list", 4) == 0)
		{
			pthread_rwlock_rdlock(p);
			string msg = "Keys disponibles en la BD:\n";
			for (map < unsigned long, Value>::iterator it = db.begin(); it != db.end(); ++it)
			{
				char buffers [50];

				sprintf (buffers, "%lu", it->first);
				string ss = buffers;
				msg += ss + "\n";
			}
			send(newSocket, msg.c_str(), strlen(msg.c_str()),0);

			*buf = '\0';
			pthread_rwlock_unlock(p);
		}
		
	}
	if (rc == -1)
	{
		cout << "read error" << endl;
		close(newSocket);
		pthread_exit(NULL);
	}
	else if (rc == 0)
	{
		cout << "EOF\n" << endl;
		close(newSocket);
	}
	if (ex == 1)
	{
		pthread_exit(NULL);
	}

	return 0;
}

char *socket_path = (char*)
"\0hidden";

int main(int argc, char **argv)
{
	srand(time(NULL));
	struct sockaddr_un addr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	cout << socket_path;
	// Procesar opciones de linea de comando
	while ((opt = getopt(argc, argv, "s:")) != -1)
	{
		switch (opt)
		{ /*Procesar el flag s si el usuario lo ingresa */
			case 's':
				socket_path = argv[2];
				sflag = 1;
				break;
			default:
				cout << "input invalido" << endl;
				return 0;
		}
	}
	while (1)
	{
		if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		{
			cout << "socket error" << endl;
			exit(-1);
		}

		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		if (*socket_path == '\0')
		{ *addr.sun_path = '\0';
			socket_path = (char*)
			"/tmp/db.tuples.sock";
			strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
		}
		else
		{
			strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
			unlink(socket_path);
		}

		if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
		{
			cout << "bind error: socket ya existe en esta direccion" << endl;
			exit(-1);
		}
		if (listen(fd, 5) == -1)
		{
			cout << "listen error" << endl;
			i = 0;
		}
		else
		{
			cout << "Escuchando conexion del cliente..." << endl;
		}

		while (1)
		{

			addr_size = sizeof serverStorage;
			int newSocket = accept(fd, (struct sockaddr *) &serverStorage, &addr_size);
			struct arg_struct args;
			args.arg1 = &lock_rw;
			args.arg2 = newSocket;
			if (pthread_create(&tid[i], NULL, socketThread, (void*) &args) != 0)
				printf("Failed to create thread\n");
			else
			{
				cout << "Conexion con cliente aceptada\nEsperando comandos del clientes..." << endl;
			}

			if (i >= 5)
			{
				i = 0;
				while (i < 5)
				{
					pthread_join(tid[i++], NULL);
				}
				i = 0;
			}

		}
	}
}

