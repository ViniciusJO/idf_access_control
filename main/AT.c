#include "AT.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

AT_Command parseAT_Command(char *str) {

  char *param_sep = strchr(str, '=');

  str += 3;

  AT_Command at = {.command = strtok(str, param_sep ? "=" : ""),
                   .param_count = 0};

  if (param_sep) {
    str = str + strlen(str) + 1;
    char *next_param_sep = strchr(str, ',');
    while (next_param_sep != NULL) {
      at.param_count++;
      at.param = realloc(at.param, at.param_count * sizeof(char *));
      at.param[at.param_count - 1] = strtok(NULL, ",");
      next_param_sep = strchr(next_param_sep + 1, ',');
    }
    at.param_count++;
    at.param = realloc(at.param, at.param_count);
    at.param[at.param_count - 1] = strtok(NULL, "");
  }

  return at;
}

char *at2string(AT_Command at) {
  char *str = (char *)malloc(256);
  size_t len = 0;
  len += sprintf(str, "{\n    command: %s", at.command);
  if (at.param_count) {
    len += sprintf(str + len, "\n    parameters: [\n");
    for (int i = 0; i < at.param_count; i++) {
      len += sprintf(str + len, "        [%d]: %s%c\n", i, at.param[i],
                     i < at.param_count - 1 ? ',' : ' ');
    }
    sprintf(str + len, "    ]\n}n");
  } else {
    sprintf(str + len, "\n    parameters: []\n}\n");
  }
  return str;
}
