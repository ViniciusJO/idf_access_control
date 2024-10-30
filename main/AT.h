#ifndef __AT_H__
#define __AT_H__

#include <stddef.h>

typedef struct {
  char *command;
  char **param;
  size_t param_count;
} AT_Command;


AT_Command parseAT_Command(char *str);
char *at2string(AT_Command at);

#endif //__AT_H__
