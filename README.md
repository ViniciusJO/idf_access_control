# IDF Access Control

An access control system develop with an ESP32 with the ESPRESSIF IDF platform. There is a correct sequence of 6 digits, inputed via a 4 buttons interface, for which the green LED indicates access granted. For wrong sequencies the red LED signals access denied. Send the desired sequence of six characters of the valid alphabet ['1', '2', '3', '4'] for the UART 2 of the board to configure the password.

To build the project, flash the board and see the serial monitor use the commands:

```sh
idf.py build flash monitor -p <serial_port>
```

The project uses the following GPIOS for the interfaces:

- `gpio_2`:   Green LED
- `gpio_15`:  Red LED
- `gpio_16`:  UART 2 TX
- `gpio_17`:  UART 2 TX
- `gpio_4`:   Button 1
- `gpio_5`:   Button 2
- `gpio_18`:  Button 3
- `gpio_19`:  Button 4

## [WIP]

- [ ] Connection diagram
- [ ] AT commands and parser