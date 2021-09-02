#include <genpasswd.h>

char *genpasswd(int length) {
  char *pool = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@!$#%^&_-";
  char *password = (char *)malloc(sizeof(*password) * (length + 1));

  for (int i = 0; i < length; i++) {
    password[i] = pool[rand() % strlen(pool)];
  }
  password[length] = '\0';

  return password;
}
