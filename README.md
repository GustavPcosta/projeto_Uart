Projeto: Controle de LEDs e Display com Raspberry Pi Pico

Descrição

Este projeto utiliza a placa Raspberry Pi Pico para controlar uma matriz de LEDs WS2812, um display OLED SSD1306 e LEDs RGB individuais. A interação é feita através de botões e comandos recebidos via UART ou USB.

Funcionalidades

Controle de LEDs RGB através de comandos seriais.

Exibição de números no display OLED SSD1306.

Exibição de números na matriz de LEDs WS2812.

Controle de LEDs e exibição no display através de botões físicos.

Interrupções para detectar pressionamento de botões.

Hardware Utilizado

Raspberry Pi Pico

Display OLED SSD1306 (I2C)

Matriz de LEDs WS2812 (5x5)

LEDs RGB individuais

2 Botões de entradaCompilação e Execução

Instale o SDK do Raspberry Pi Pico e configure seu ambiente de desenvolvimento.

Compile o código usando CMake e GCC ARM.

Carregue o binário para o Raspberry Pi Pico via UF2.

Dependências

pico-sdk (Biblioteca oficial da Raspberry Pi para programar a Pico)

ssd1306.h (Biblioteca para controle do display OLED)

ws2812.pio.h (Biblioteca para controle da matriz de LEDs WS2812)


Observações

A comunicação com serial monitor está sendo feita com COM4-dispositivo serial usb(COM4)

Link para o vídeo 
https://youtube.com/shorts/08XkkJ5pxZ4?feature=share

Autor

Desenvolvido por Gustavo
