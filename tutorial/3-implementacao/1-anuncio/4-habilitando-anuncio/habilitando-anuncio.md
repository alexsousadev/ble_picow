# Habilitando o Bluetooth
Agora que já temos todos os arquivos de configuração, podemos habilitar o Bluetooth associando todas as configurações criadas.

## 1) Habilitando os protocolos (l2CAP, SMP, ATT)

Vamos começar habilitando os protocolos que o Pico W usa para se comunicar com outros dispositivos:

```c
// Habilitando os protocolos
l2cap_init();
sm_init();
att_server_init(profile_data, att_read_callback, att_write_callback);
```

- `l2cap_init()`: Inicializa o protocolo L2CAP (Logical Link Control and Adaptation Protocol), que é usado para gerenciar a transmissão de dados entre dispositivos Bluetooth.
- `sm_init()`: Inicializa o Security Manager (Gerenciador de Segurança), que cuida da autenticação e criptografia das conexões
- `att_server_init()`: Inicializa o servidor ATT (Attribute Protocol), que é responsável por lidar com os atributos (ex.: temperatura, nome, etc.)
  > Ele usa como parâmetro nossas funções de gerenciamento de eventos!

## 2) Habilitando eventos HCI e ATT
> Lembrando que O HCI é a interface entre a pilha de software do Bluetooth (host) e o hardware do rádio Bluetooth (controlador). Eventos HCI incluem conexões, desconexões, completamento de comandos, etc.

Primeiro, vamos criar uma estrutura que vai guardar as configurações do HCI:

```c
btstack_packet_callback_registration_t hci_event_callback_registration;
```
Agora vamos de fato realizar as configurações para o HCI

```c
// Registra o callback para eventos HCI
hci_event_callback_registration.callback = &packet_handler;
hci_add_event_handler(&hci_event_callback_registration);
hci_power_control(HCI_POWER_ON);
```
- Configuramos o handler (`packet_handler`) para processar os eventos HCI  
- Conectamos a estrutura configurada ao HCI (`hci_add_event_handle`)
- Ligamos o HCI
Também iremos configurar o ATT usando o nosso handler:

```c
// Registra o callback para eventos ATT
att_server_register_packet_handler(packet_handler);
```

## 3) Configurando o GAP
Agora, vamos configurar parâmetros dos anúncios BLE no nível do GAP (Generic Access Profile), que define como o dispositivo se apresenta e se conecta a outros dispositivos.

Vamos criar um endereço para o nosso Bluetooth:

```c
bd_addr_t null_addr = {0};
```
> Na biblioteca BTstack, bd_addr_t é tipicamente definido como um array de 6 bytes (`uint8_t[6]`). Quando fazemos `{0}`, estamos inicializando o array com zeros, o que significa que o endereço é composto por 6 bytes de 0. **Estamos fazendo isso para indicar que o dispositivo não está filtrando ou direcionando anúncios para um endereço Bluetooth específico.**

Agora podemos configurar o GAP:

```c
gap_advertisements_set_params(800, 800, 0, 0, null_addr, 0x07, 0x00);

gap_advertisements_set_data(adv_data_len, (uint8_t *)adv_data);

gap_advertisements_enable(1);
```
- `gap_advertisements_set_params()`: Configurar o comportamento do anúncio.

  - `800, 800`: Define o intervalo mínimo e máximo de anúncio em unidades de 0,625 ms (800 * 0,625 ms = 500 ms). Isso significa que o dispositivo enviará anúncios a cada ~500 ms.
  - `0`: Tipo de canal de anúncio (0 significa usar todos os canais disponíveis: 37, 38 e 39).
  - `0`: Tipo de endereço público (0 indica endereço público padrão).
  - `null_addr`: Endereço Bluetooth do dispositivo (neste caso, um endereço nulo, indicando que não há filtragem por endereço).
  - `0x07`: Modo de descoberta (0x07 indica "conectável e escaneável").
  - `0x00`: Flags adicionais (nenhum flag especial configurado).

- `gap_advertisements_set_data()`: Define os dados que serão anunciados
- `gap_advertisements_enable()`: É o botão liga/desliga do anúncio.
---
## 🔗 Trecho completo
```c
// Inicializa o stack Bluetooth
static void init_bluetooth(void)
{
    l2cap_init();
    sm_init();
    att_server_init(profile_data, att_read_callback, att_write_callback);

    // Registra o callback para eventos HCI
    btstack_packet_callback_registration_t hci_event_callback_registration;
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Registra o callback para eventos ATT
    att_server_register_packet_handler(packet_handler);

    // Define os parâmetros de anúncio BLE
    bd_addr_t null_addr = {0};
    gap_advertisements_set_params(800, 800, 0, 0, null_addr, 0x07, 0x00);

    // Define os dados que serão anunciados
    gap_advertisements_set_data(adv_data_len, (uint8_t *)adv_data);

    // Habilita o anúncio
    gap_advertisements_enable(1);

    hci_power_control(HCI_POWER_ON);
}
```
---
## [Voltar](../anuncio.md#4-ativar-o-anúncio)