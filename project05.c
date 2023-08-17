#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "server.h"
#include "network.h"

#define PORT "8100"


int main(int argc, char **argv) { 

	int sock_fd = create_tcp_socket(PORT);

	while (1) {
	  int new_fd;
	  while ((new_fd = accept(sock_fd, NULL, NULL)) == -1) {
	      if (errno == EAGAIN || errno == EWOULDBLOCK) {
	          usleep(1000);
	          continue;
	      } else {
	          perror("Failed to accept incoming connection");
	          exit(1);
	      }
	  }

	  handle_new_connection(new_fd);
	}

}

