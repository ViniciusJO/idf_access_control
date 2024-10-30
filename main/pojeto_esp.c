#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <hal/gpio_types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"

#define MIN(a, b) ((a) < (b)) ? (a) : (b)

#define BUT_1 4
#define BUT_2 5
#define BUT_3 18
#define BUT_4 19

#define PASS_COUNT 6

QueueHandle_t queue = NULL;
QueueHandle_t uart_queue = NULL;
TaskHandle_t verify_handle = NULL;

int true_pass[PASS_COUNT] = {1, 2, 3, 4, 1, 2};

void verify_pass_task(void *args) {
  int button;
  int password[PASS_COUNT] = {0};
  int count_digit = 0;
  for (;;) {
    if (xQueueReceive(queue, &button, 5000 / portTICK_PERIOD_MS) != pdTRUE) {
      if (count_digit > 0)
        ESP_LOGI("Verify Pass", "QUEUE TIMEOUT");
      gpio_set_level(2, 0);

      if (count_digit) {
        gpio_set_level(15, 1);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        gpio_set_level(15, 0);
      }
      count_digit = 0;
      continue;
    }
    password[count_digit] = button;
    count_digit++;

    gpio_set_level(2, 0);
    gpio_set_level(15, 0);
    if (count_digit >= PASS_COUNT) {
      puts("PASSWORD: ");
      for (int i = 0; i < PASS_COUNT; i++)
        printf("%d ", password[i]);
      puts("");
      if (!memcmp(password, true_pass, PASS_COUNT * sizeof(int))) {
        puts("Senha correta");
        gpio_set_level(2, 1);
      } else {
        puts("Senha incorreta");
        gpio_set_level(15, 1);
      }
      count_digit = 0;
    }
  }
}

void button_handler_task(void *arg) {
  int hold = 0;
  for (;; vTaskDelay(10)) {
    int pressed = !gpio_get_level(BUT_1) * 1 + !gpio_get_level(BUT_2) * 2 +
                  !gpio_get_level(BUT_3) * 3 + !gpio_get_level(BUT_4) * 4 +
                  hold * 5;
    /*printf("%d\n", pressed);*/

    switch (pressed) {
    case 5:
      hold = 0;
      break;
    case 1:
    case 2:
    case 3:
    case 4:
      while (xQueueSend(queue, &pressed, portMAX_DELAY) != pdTRUE)
        ;
      ESP_LOGI("button_handler_task", "Button pressed: %d", pressed);
      hold = 1;
    }
  }
}

void uart_receive(void *arg) {
  const uart_port_t uart_num = UART_NUM_2;
  uint8_t data[7];
  int pos_data = 0;
  for (;;) {
    uart_read_bytes(uart_num, data + pos_data, 1, portMAX_DELAY);

    if (*(data + pos_data) >= '1' && *(data + pos_data) <= '4') {
      printf("-> Received from uart: %c\n", *(data + pos_data));
      uart_write_bytes(uart_num, data + pos_data, 1);
      pos_data++;
    }

    if (pos_data >= 6) {
      data[6] = '\0';

      vTaskSuspend(verify_handle);
      for (int i = 0; i < PASS_COUNT; i++)
        true_pass[i] = data[i] - '0';
      vTaskResume(verify_handle);

      char ret[128];
      sprintf(ret, "Password changed to: %s", data);
      printf("%s\n", ret);
      uart_write_bytes(uart_num, (const char *)ret, strlen(ret));
      pos_data -= 6;
    }
  }
}

void app_main(void) {
  queue = xQueueCreate(12, sizeof(int));
  uart_queue = xQueueCreate(12, sizeof(int));

  gpio_config_t input = {
    .mode = GPIO_MODE_INPUT,
    .intr_type = GPIO_INTR_DISABLE,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .pin_bit_mask = (1 << BUT_1) | (1 << BUT_2) | (1 << BUT_3) | (1 << BUT_4)
  };
  gpio_config(&input);

  gpio_config_t output = {
    .mode = GPIO_MODE_OUTPUT,
    .intr_type = GPIO_INTR_DISABLE,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .pin_bit_mask = (1 << 2) | (1 << 15)
  };
  gpio_config(&output);

  const int uart_buffer_size = (1024 * 2);
  uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT
  };
  uart_param_config(UART_NUM_2, &uart_config);
  uart_set_pin(UART_NUM_2, 17, 16, GPIO_INTR_DISABLE, UART_PIN_NO_CHANGE);
  uart_driver_install(UART_NUM_2, uart_buffer_size, 0, 0, NULL, 0);

  xTaskCreate(button_handler_task, "BUTTON HANDLER", 2048, NULL, 0, NULL);
  xTaskCreate(verify_pass_task, "PASSWORD HANDLER", 2048, NULL, 0, &verify_handle);
  xTaskCreate(uart_receive, "UART HANDLER", 8192, NULL, 0, NULL);
}
