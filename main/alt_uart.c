#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <hal/gpio_types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AT.h"
#include "colors.h"

#define PASS_COUNT 6

int is_valid_pass(char *pass) {
  size_t len = strlen(pass);
  if(len < PASS_COUNT) return 0;
  for(int i=0; i<len; i++) {
    if (pass[i] < '1' || pass[i] > '4') return 0;    
  }
  return 1;
}

void uart_receive_alt(void *arg) {
  const uart_port_t uart_num = UART_NUM_2;
  uint8_t data[256] = {0};
  uint8_t *pos;
  for (;;) {
    pos = data;
    do {
      uart_read_bytes(uart_num, pos, 1, portMAX_DELAY);
    } while (*pos++ != '\r' && pos != data + 256);

    *pos = '\0';

    if (strncmp((char *)data, "AT+", 3)) {
      char ret[512];
      sprintf(ret, "Invalid AT Command: %s", data);
      printf(COLOR_RED "%s\n" COLOR_RESET, ret);
      uart_write_bytes(uart_num, ret, strlen(ret));
      continue;
    }

    AT_Command at = parseAT_Command((char *)data);

    printf(COLOR_YELLOW "%s\n\n" COLOR_RESET, at2string(at));
    /*printf("%d changepass, %d count %d, %d size\t%s",strncmp(at.command, "CHANGEPASS", 10), at.param_count > 0, (int)at.param_count, strlen(at.param[0]) >= 6, at.param[0]);*/
    /*if(strcmp(at.command, "CHANGEPASS") && at.param_count > 0 && strlen(at.param[0]) >= 6) {*/
    /*  if(!is_valid_pass(at.param[0])) {*/
    /*    char ret[128];*/
    /*    sprintf(ret, "Invalid password: %s", at.param[0]);*/
    /*    printf("%s\n", ret);*/
    /*    uart_write_bytes(uart_num, (const char *)ret, strlen(ret));*/
    /*  }*/
    /**/
    /*  vTaskSuspend(verify_handle);*/
    /*  for (int i = 0; i < PASS_COUNT; i++)*/
    /*    true_pass[i] = data[i] - '0';*/
    /*  vTaskResume(verify_handle);*/
    /**/
    /*  char ret[128];*/
    /*  sprintf(ret, "Password changed to: %s", at.param[0]);*/
    /*  printf("%s\n", ret);*/
    /*  uart_write_bytes(uart_num, (const char *)ret, strlen(ret));*/
    /*}*/
  }
}

