## Packet Handler
  O BLE é cheio de eventos, tipo mensagens que chegam: "O celular conectou!", "O celular quer o valor!", "O celular desligou!" etc. O `packet_handler` é responsável por lidar com esses eventos grandes de conexão. Seus parâmetros são:
  - `packet_type`: Tipo de evento. ✅
  - `channel`: Canal do Bluetooth. ❌
  - `packet`: O conteúdo da mensagem, tipo um pacote com os detalhes do evento. ✅
  - `size`: Tamanho do pacote. ❌
  ---
  ```c
  void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
  ```
  ---
  > Os tópicos com visto são aqueles que usaremos, e em vermelhor são aqueles que existem, mas não são usados no nosso projeto.
  
  No BLE, tudo funciona por eventos. O Pico não fica "adivinhando" o que tá acontecendo; ele espera o Bluetooth avisar. Então nossa função será baseada em checar os tipo de eventos e executar ações apropriadas. Os principais eventos são:
  - `BTSTACK_EVENT_STATE`: Esse evento acontece quando o sistema Bluetooth do Pico tá mudando de estado, tipo ligando ou desligando
  - `HCI_EVENT_DISCONNECTION_COMPLETE`: Esse evento roda quando o celular desconecta do Pico, tipo quando desliga o Bluetooth ou sai do alcance
  - `ATT_EVENT_CAN_SEND_NOW`: Esse evento acontece quando o Bluetooth diz que tá liberado pra mandar uma notificação pro celular (se está aqui, é porque podemso enviar os dados)
  
  Então vamos criar um caso para cada um desses eventos! Mas antes, precisamos converter esse valor em um evento para o HCI, fazemos isso com a função `hci_event_packet_get_type()`. Essa função recebe um pacote e retorna o tipo de evento.
  ```c
  switch (hci_event_packet_get_type(packet))
      {
  ```
  Agora podemos criar cada case para cada evento.
  1. Vamos começar com o `HCI_EVENT_DISCONNECTION_COMPLETE `, que é o evento que acontece quando o Pico desconecta. Bom, se isso ocorrer, então apenas desligamos as notificações e printamos que o cliente desconectou.
  
      ```c
      case HCI_EVENT_DISCONNECTION_COMPLETE:
              // Se a conexão foi encerrada, desativar notificações
              le_notification_enabled = 0;
              printf("Cliente desconectou\n");
              break;
      ```
  2. Se o cliente estiver pronto (`ATT_EVENT_CAN_SEND_NOW`), vamos enviar os dados. Quem é o nosso carteiro? O ATT! Então aqui usaremos ele...
  A função responsável por isso é a `att_server_notify()`.
      ```c
      uint8_t att_server_notify(hci_con_handle_t con_handle, uint16_t attribute_handle, const uint8_t *value, uint16_t value_len);
      ```
  
      Seus parâmetros são:
      - `con_handle`: **O identificador do cliente que está conectado (um valor único para essa conexão).** 
        > Quando um dispositivo se conecta, um handle (identificador) único é gerado para essa conexão. Este parâmetro permite que você especifique qual conexão receberá a notificação.
      - `attribute_handle`: O identificador da característica GATT que queremos enviar
        > Nesse caso, é nossa característica `ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE`
      - `value`: O valor que queremos enviar ( podem ser de qualquer tipo, mas são tratados como um array de bytes (`uint8_t`))
      - `value_len`: O tamanho do valor em bytes 
  
      ---
        ```c
        att_server_notify(
        con_handle,
        ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_VALUE_HANDLE,
        &current_value,
        sizeof(uint8_t));
        ```
      ---
      Vamos destrinchar um pouco esse argumento maior para entender o que significa:

        <p align="center">
          <img src="../../../../../images/att-caracteristic.png" alt="Partes da caracteristica do ATT" width="600"/>
        </p>
      
      - O nome é aquele que você definiu no seu `.gatt`
      - O campo é o que você vai alterar (lembre que uma característica é formada por partes menores, o [atributos](#atributos-de-uma-característica), então temos que sinalizar que queremos alterar o valor, por isso o `VALUE_HANDLE`)
  3. Por último, vamos escrever o código para lidar com o evento `BTSTACK_EVENT_STATE`, que ocorrer quando o sistema tá mudando de estado, como ligando ou desligando. Nesse caso, o importante é quando está totalmente ligado. Se está, continuamos, se não, saímos:
  
      ```c
      case BTSTACK_EVENT_STATE:
          if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING)
              return;
      ```
      - `btstack_event_state_get_state(packet)`: Olha o pacote do evento e diz em que estado o Bluetooth tá (ex.: inicializando, ligado, desligado).
      - `HCI_STATE_WORKING`: É o estado que o Bluetooth está funcionando (ou seja, só continua se está ligado)
  
        ### Ligando o servidor
        Se conseguimos ligar o bluetooth, então podemos iniciar nosso servidor. Para isso, vamos usar as funções GAP! Para ligarmos um servidor, precisamos de um IP, certo? Entao precisamos pegar o IP do nosso Pico W:
  
        ```c
        bd_addr_t local_addr;
        gap_local_bd_addr(local_addr);
        ```
        - O `bd_addr_t` geralmente é uma estrutura de 6 bytes do endereço Bluetooth, comumente representado no formato hexadecimal, como XX:XX:XX:XX:XX:XX.
  
        Agora vamos configurar o nosso "anúncio" com a `gap_advertisements_set_param`
        ```c
        void gap_advertisements_set_params(
        uint16_t adv_int_min,
        uint16_t adv_int_max,
        uint8_t adv_type,
        uint8_t direct_address_typ,
        bd_addr_t direct_address,
        uint8_t channel_map, uint8_t filter_policy);
        ```
        - `adv_int_min`: Tempo mínimo entre anúncios (em milisegundos)
        - `adv_int_max`: Tempo máximo entre anúncios (em milisegundos)
        - `adv_type`: Tipo de anúncio (0 = ADV_IND, 1 = ADV_DIRECT_IND, 2 = ADV_SCAN_IND, 3 = ADV_NONCONN_IND)
        - `direct_address_typ`: Tipo de endereço (0 = public, 1 = random)
        - `direct_address`: Endereço do dispositivo (podemos sinalizar que o anúncio é para um dispositivo específico. Se não for, então só colocar `0x00`, que será público)
        - `channel_map`: Mapeamento de canais (se for public, não precisa ser definido)
        - `filter_policy`: Política de filtragem (se for public, não precisa ser definido)
  
        Após configurar nosso anúncio, podemos definir os dados que aparecerão no anúncio (que definimos na [Configuração do GAP](#configuração-do-gap)) e habilitar ele:
  
        ```c
        gap_advertisements_set_data(adv_data_len, (uint8_t *)adv_data);
        gap_advertisements_enable(1)
        ```
      
  
  ---
  ## [Voltar](../gerenciador-eventos.md)