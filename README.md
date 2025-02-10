📚 Unidade 4 - Capítulo 6: Introdução às Interfaces de Comunicação Serial com RP2040
📝 Objetivo do Projeto
Este projeto visa consolidar os conceitos de comunicação serial (UART, SPI, I2C) no microcontrolador RP2040, utilizando a placa BitDogLab para controlar LEDs, botões e displays, enquanto exploramos o uso de interrupções, debouncing e envio de dados via UART.


🛠️ Funcionalidades Implementadas
Modificação da Biblioteca font.h ✍️

Adição de caracteres minúsculos para exibição no display.
Criação de novos caracteres de forma criativa.
Entrada de Caracteres via PC 💻

O Serial Monitor do VS Code é utilizado para digitar caracteres.
Cada caractere digitado será exibido no Display SSD1306.
Números de 0 a 9 exibem um símbolo correspondente na Matriz de LEDs WS2812.
Interação com o Botão A 🟩

Pressionar o Botão A alterna o estado do LED RGB Verde.
O estado do LED é mostrado no display e enviado ao Serial Monitor.
Interação com o Botão B 🟦

Pressionar o Botão B alterna o estado do LED RGB Azul.
O estado do LED é mostrado no display e enviado ao Serial Monitor.
🔧 Requisitos
Uso de Interrupções (IRQ) ⚡

As funcionalidades dos botões são implementadas por meio de interrupções.
Debouncing de Botões 🛑

O debounce é implementado via software para garantir uma leitura precisa dos botões.
Controle de LEDs 💡

O projeto utiliza tanto LEDs comuns quanto LEDs WS2812 para demonstrar controle de diferentes tipos de LEDs.
Exibição no Display 128x64 📱

O display é utilizado para exibir tanto caracteres maiúsculos quanto minúsculos.
Comunicação Serial via UART 🔄

O projeto envia informações sobre o estado dos LEDs e botões por UART para o Serial Monitor.
Organização e Comentários 💬

O código está bem estruturado e comentado para facilitar a compreensão.
📦 Instruções de Uso

Após carregar o código, você poderá:
Digitando caracteres no Serial Monitor, eles serão exibidos no display.
Pressionando o Botão A alterna o LED verde, e o estado será mostrado no display.
Pressionando o Botão B alterna o LED azul, e o estado será mostrado no display.
📽️ Demonstração
Vídeo de demonstração disponível no YouTube/Google Drive onde o funcionamento do projeto será mostrado.
Apresentação do projeto com a placa BitDogLab.
Demonstração das funcionalidades implementadas.

https://www.youtube.com/watch?v=jSdBryL22uQ

Autor: Willomar Soares dos Santos
Data: 09/02
Versão: 1.0

