## ATT Read

Ela é chamada automaticamente pela stack Bluetooth (neste caso, o BTstack) quando um cliente BLE (como um celular) faz um pedido para ler o valor de um atributo específico no servidor (o Raspberry Pi Pico W).
> No contexto do código, o atributo é a temperatura atual medida pelo sensor do Pico W.

## Estrutura
A função tem a seguinte assinatura:

```c
uint16_t att_read_callback(
hci_con_handle_t connection_handle,
uint16_t att_handle,
uint16_t offset,
uint8_t *buffer,
uint16_t buffer_size)
```
- `connection_handle`: Identificador único do cliente. Permite que a função saiba qual cliente está fazendo o pedido, caso haja múltiplas conexões. 
- `att_handle`: Identificador único (handle) do atributo GATT que o cliente está pedindo ler. Nesse caso, é o atributo:
    ```c
    ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_VALUE_HANDLE
    ```
- `offset`: Indica a partir de qual byte do valor do atributo o cliente quer começar a ler (usado para dados grandes que podem ser lidos em partes).
- `*buffer`: Um ponteiro para um array onde o servidor deve copiar o valor do atributo solicitado.
    > É onde a função coloca os dados (neste caso, o valor da temperatura) para que o stack Bluetooth os envie ao cliente.
- `buffer_size`: O tamanho (em bytes) do buffer disponível para armazenar os dados.

Agora que os parâmetros estão claros, vamos começar a escrever nossa função de fato. 


### 1) Verificação do Handle
```c
if (att_handle == ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_VALUE_HANDLE)
```
Verifica se o atributo solicitado pelo cliente é a característica de temperatura
> O servidor pode ter múltiplos atributos (ex.: temperatura, nome do dispositivo, configurações). Essa verificação garante que estamos lidando com a característica de temperatura.

Se for...
```c
return att_read_callback_handle_blob(
(uint8_t *)¤t_value,
sizeof(current_value),
offset,
buffer,
buffer_size);
```
- Essa função serve para copiar o valor da temperatura para o buffer

    ### O que é um BLOB?
   O protocolo ATT impõe um limite no tamanho dos dados que podem ser incluídos em um único pacote de resposta. Se o valor de um atributo for maior que esse limite, uma única operação de leitura não será suficiente para obter o valor completo.

    É aqui que entra o conceito de "Blob" (Binary Large Object) no contexto do BLE. Quando um atributo tem um valor grande, ele precisa ser lido em partes, ou "pedaços". O protocolo ATT oferece mecanismos para realizar essa leitura segmentada: `att_read_callback_handle_blob`
    ### Parâmetros
    - `(uint8_t *)&current_value`: Um ponteiro para o valor a ser lido.
    - `sizeof(current_value)`: O tamanho do valor a ser lido.
        > Nesse caso, 2 bytes, já que current_value é um uint16_t.
    - `offset`: A posição inicial para leitura (geralmente 0, já que a temperatura é um valor pequeno).
    - `buffer`: O buffer onde o valor será copiado.
    - `buffer_size`: O tamanho disponível no buffer.

    > *Resumindo*: Copia os dados de `current_value` para o `buffer`, respeitando o `offset` e o `buffer_size`.

### Analogia
A att_read_callback é como um vendedor em uma loja de termômetros. Quando um cliente entra e pergunta "Qual é a temperatura agora?", o vendedor verifica se o pedido é válido (checa o att_handle), pega o termômetro (o valor de current_value), e entrega a leitura ao cliente (copia para o buffer). Se o cliente pedir algo que a loja não tem, o vendedor diz "Não temos isso" (retorna 0).




---
### Função Completa
```c
uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    UNUSED(connection_handle);

    if (att_handle == ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_VALUE_HANDLE)
    {
        return att_read_callback_handle_blob((uint8_t *)&current_value,
                                             sizeof(current_value),
                                             offset,
                                             buffer,
                                             buffer_size);
    }
    return 0;
}
```

---
## [Voltar](../gerenciador-eventos.md)