#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void tcp_check(char *hostname, int port) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    printf("cannot open socket\n");
    return;
  }

  struct hostent *server = gethostbyname(hostname);
  if (server == NULL) {
    printf("no such host\n");
    goto tcp_check_flag;
  }

  struct sockaddr_in serv_addr;
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

  serv_addr.sin_port = htons(port);
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("connect failed\n");
  } else {
    printf("connect success\n");
  }
tcp_check_flag:
  close(sockfd);
  return;
}
