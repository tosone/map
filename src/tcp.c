
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

void tcp_check(char *hostname, int port) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    printf("cannot open socket\n");
    return;
  }

  struct sockaddr_in serv_addr;
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;

  if (inet_pton(AF_INET, hostname, &serv_addr.sin_addr) <= 0) {
    printf("please check ip address is valid or not\n");
    goto tcp_check_flag;
  }

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
