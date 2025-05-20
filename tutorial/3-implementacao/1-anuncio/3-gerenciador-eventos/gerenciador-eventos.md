  # Configurando o Gerenciamento de Eventos
  
  O Pico precisa ouvir o cliente e reagir a coisas como conexão, pedidos, ou desligamento. Isso é feito por três funções especiais (callbacks):
  - `packet_handler`: Escuta eventos gerais do Bluetooth, como ligar o dispositivo, desconectar, ou estar pronto pra mandar dados.
    > Pense nele como um "gerente" que monitora tudo o que acontece no sistema Bluetooth e toma decisões com base nos eventos recebidos. Conexão com cliente deu certo? Faz tal coisa. Cliente se desconectou? Faz outra coisa....
  - `att_read_callback`: é chamado quando o cliente pede para ler um valor do Pico W (ou seja, lendo alguma característica).
    > Pense nisso como um cliente perguntando: "Qual é a temperatura agora?" e o Pico W respondendo com o valor atual.
  - `att_write_callback`: Lida com pedidos para alterar uma configuração no Pico W, como ativar ou desativar notificações. 
    > Pense nisso como o cliente dizendo: "Quero receber atualizações automáticas da temperatura" ou "Pare de me enviar atualizações".
---
- ### [1) Packet Handler](./1-packet-handler/packet-handler.md)
- ### [2) Att Read Callback](./2-att-read/att-read.md)
- ### [3) Att Write Callback](./3-att-write/att-write.md)
Agora que já temos nossos callbacks, vamos habilitar nosso anúncio e fazer a conexão!

---


## 🔗 Trecho Completo

```c
// Packet Handler
void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(size);
    UNUSED(channel);

    if (packet_type != HCI_EVENT_PACKET)
        return;

    bd_addr_t local_addr;
    switch (hci_event_packet_get_type(packet))
    {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING)
            return;
        gap_local_bd_addr(local_addr);
        printf("BTstack inicializado: %s\n", bd_addr_to_str(local_addr));
        break;

    case HCI_EVENT_DISCONNECTION_COMPLETE:
        le_notification_enabled = 0;
        printf("Cliente desconectado\n");
        break;

    case ATT_EVENT_CAN_SEND_NOW:
        att_server_notify(con_handle,
                          ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_VALUE_HANDLE,
                          (uint8_t *)&current_value,
                          sizeof(current_value));

        att_server_notify(con_handle, (uint16_t)"a37867a-1d2f-49d7-9c51-e64e18c3f948",
                          (uint8_t *)&current_value,
                          sizeof(current_value));

        break;

    default:
        break;
    }
}


// Att Read Callback
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

// Att Write Callback
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

## Pŕoximo: [Habilitando o Anúncio](../3-habilitando-anuncio/habilitando-anuncio.md)