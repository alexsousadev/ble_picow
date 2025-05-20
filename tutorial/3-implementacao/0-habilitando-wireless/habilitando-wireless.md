# Habilitando o módulo wireless do Pico W

O Pico W tem um chip de Bluetooth chamado CYW43. Antes de ele enviar sinais, precisamos “acordá-lo”, como ligar o Bluetooth do seu celular antes de conectar um fone de ouvido.

Para isso, o Pico W tem uma biblioteca chamada `cyw43_drivers`. Essa biblioteca é específica para o chip CYW43439 que suporta Wi-Fi e Bluetooth. Vamos importar:

```c
#include "pico/cyw43_arch.h"
```

E depois podemos ligar o chip, vamos definir as variáveis globais:

```c
// Variáveis globais
int le_notification_enabled = 0;
hci_con_handle_t con_handle = 0;
uint16_t current_value = 0;
```
- `le_notification_enabled`: Indica se o cliente está habilitado para receber notificações
- `con_handle`: Identificador único do cliente
- `current_value`: O valor atual da temperatura




---
## Próximo: [Anunciando para o mundo (GAP)](./1-anuncio/anuncio.md)