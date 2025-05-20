## ATT Write
Ela é chamada automaticamente quando um cliente BLE envia um comando para escrever ou alterar um atributo no servidor (o Raspberry Pi Pico W).

```c
int att_write_callback(
hci_con_handle_t connection_handle,
uint16_t att_handle,
uint16_t transaction_mode,
uint16_t offset,
uint8_t *buffer,
uint16_t buffer_size)
```
- `connection_handle`: Identificador único do cliente.
- `att_handle`: Identificador único (handle) do atributo GATT que o cliente está pedindo alterar.
- `transaction_mode`: Indica o tipo de operação de escrita (ex.: escrita direta ou preparada).
     > _Marcado como UNUSED(transaction_mode), indicando que o código ignora esse parâmetro_
- `offset`: Indica a partir de qual byte do valor do atributo o cliente quer alterar (usado para dados grandes que podem ser alterados em partes).
- `*buffer`: Um ponteiro para um array onde o servidor deve copiar o valor do atributo solicitado.
- `buffer_size`: O tamanho (em bytes) do buffer disponível para armazenar os dados.

Agora que os parâmetros estão claros, vamos começar a escrever nossa função de fato. Alguns parâmetros não serão utilizados, são eles:

```c
UNUSED(transaction_mode);
UNUSED(offset);
UNUSED(buffer_size);
```
> Ao usar a macro `UNUSED()`, você informa ao compilador que você está ciente de que o parâmetro não está sendo usado, e que isso é intencional. Isso evita que o compilador gere um aviso (warning)

### 1) Verificação do Handle
```c
if (att_handle != ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_CLIENT_CONFIGURATION_HANDLE)
    return 0;
```
Verifica se o atributo solicitado pelo cliente é a característica de temperatura
> O servidor pode ter múltiplos atributos (ex.: temperatura, nome do dispositivo, configurações).


Se tudo okey, podemos habiliar/desabilitar as notificações do cliente:

```c
le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
    con_handle = connection_handle;
```
- `little_endian_read_16(buffer, 0)`: Lê 2 bytes do buffer, começando na posição 0, e os interpreta como um número de 16 bits no formato little-endian.
    >  Isso porque o BLE usa little-endian para valores de 16 bits, então essa função garante que os bytes sejam interpretados corretamente, independentemente da arquitetura do processador.
- `= GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION`: Uma constante definida no BTstack (ou biblioteca BLE) que indica que o cliente quer ativar notificações (ou não).
    > O que faz? Compara o valor lido do buffer (via little_endian_read_16) com 0x0001.Se o valor lido for 0x0001, a expressão resulta em true (1). Se o valor lido for qualquer outro (ex.: 0x0000, 0x0002), a expressão resulta em false (0).

    Ou seja, com isso vamos saber se o cliente quer ou não receber notificações e alterar a variável `le_notification_enabled` com base nisso.


Com isso já temos nosso callback. Podemos também colocar alguns printf para debugar, sinalizando que as notificações estão ativas ou não...

---
### Código Completo
```c
int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    UNUSED(transaction_mode);
    UNUSED(offset);
    UNUSED(buffer_size);

    if (att_handle != ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_CLIENT_CONFIGURATION_HANDLE)
        return 0;

    le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
    con_handle = connection_handle;

    if (le_notification_enabled)
    {
        printf("Notificações ativadas pelo cliente\n");
        att_server_request_can_send_now_event(con_handle);
    }
    else
    {
        printf("Notificações desativadas pelo cliente\n");
    }
    return 0;
}
```

---
## [Voltar](../gerenciador-eventos.md)