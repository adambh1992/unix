#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>

#include "libcli.h"
#include "telnet.h"
// vim:sw=4 tw=120 et

#define CLITEST_PORT 8000
#define MODE_CONFIG_INT 10

#ifdef __GNUC__
#define UNUSED(d) d __attribute__((unused))
#else
#define UNUSED(d) d
#endif

unsigned int regular_count = 0;
unsigned int debug_regular = 0;

int idle_timeout(struct cli_def *cli) {
  cli_print(cli, "Custom idle timeout");
  return CLI_QUIT;
}


int cmd_backtrace(struct cli_def *cli, const char *command, char *argv[], int argc) {
    FILE * f = fopen("trace.txt", "r");
    if(f < 0){
        puts("error cant open file");
        return CLI_OK;
    }
    char buff[4096];
    fread(buff, 1, 4096, f );
    cli_print(cli, "%s", buff);
    return CLI_OK;
}

void * run_child(void * args) {
  struct cli_command *c;
  struct cli_def *cli;
  struct cli_optarg *o;
  int x = *((int*)args);
  cli = cli_init();
  cli_set_banner(cli, "welcome to Filesystem Monitor!");
  cli_set_hostname(cli, "FSMon");
  cli_telnet_protocol(cli, 1);

  // set 60 second idle timeout
  cli_set_idle_timeout_callback(cli, 60, idle_timeout);
  cli_register_command(cli, NULL, "backtrace", cmd_backtrace, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, NULL);

  cli_loop(cli, x);
  cli_done(cli);
}

int open_telnet_server() {
  int server_socket, client_socket;
  struct sockaddr_in addr;

  signal(SIGCHLD, SIG_IGN);


  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return 1;
  }

  int on = 1;
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
    perror("setsockopt");
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(CLITEST_PORT);
  if (bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    return 1;
  }

  if (listen(server_socket, 50) < 0) {
    perror("listen");
    return 1;
  }

  printf("Listening on port %d\n", CLITEST_PORT);
  while ((client_socket = accept(server_socket, NULL, 0))) {
    socklen_t len = sizeof(addr);
    if (getpeername(client_socket, (struct sockaddr *)&addr, &len) >= 0)
        printf(" * accepted connection from %s\n", inet_ntoa(addr.sin_addr));


    pthread_t thread;
    pthread_create(&thread, NULL, run_child, &client_socket);
  }

  return 0;
}

// int main(){
//     open_telnet_server();
// }