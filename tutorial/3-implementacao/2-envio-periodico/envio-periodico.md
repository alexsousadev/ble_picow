# Realizando o envio periódico de dados

Para finalizar nossa implementação, vamos fazer o envio periódico da temperatura para o cliente. Para isso, vamos criar um timer que irá chamar a função `poll_value()` a cada certo tempo e enviar!

```c
static bool timer_callback(struct repeating_timer *t)
{
    poll_value(); // Lê o valor
    if (le_notification_enabled)
    {
        att_server_request_can_send_now_event(con_handle); // Solicita envio
    }
    return true; // Mantém o temporizador ativo
}
```
A ideia é verificar se as notificações estão ativas e, se sim, solicitar que o cliente envie os dados. 

## Inicializando o setup

```c
static void setup_system(void)
{
    stdio_init_all(); // Inicializa comunicação serial

    // Inicializa o driver CYW43 (Bluetooth)
    if (cyw43_arch_init())
    {
        printf("Erro: Falha ao inicializar cyw43_arch\n");
        return;
    }

    init_bluetooth(); // Configura o Bluetooth

    // Configura o temporizador repetitivo
    static struct repeating_timer timer;
    if (!add_repeating_timer_ms(POLL_INTERVAL_MS, timer_callback, NULL, &timer))
    {
        printf("Erro: Falha ao iniciar o temporizador\n");
    }
}
```
Essa função configura o sistema no início da execução do programa. Ela:

- Inicializa o chip wireless (`cyw43_arch_init()`)
- Configura o módulo Bluetooth (`init_bluetooth()`)
- Inicia um temporizador repetitivo que chama a função timer_callback periodicamente (`add_repeating_timer_ms()`)
  - `POLL_INTERVAL_MS`: Intervalo de tempo (em milissegundos) entre chamadas do temporizador. Por exemplo, se for 1000, a função timer_callback será chamada a cada 1 segundo.
  - `timer_callback`: A função que será chamada a cada intervalo (definida anteriormente).
  - `NULL`: Um argumento opcional que pode ser passado para a função de callback (não usamos aqui).
  - `&timer`: Endereço da estrutura timer, que será preenchida pelo sistema

---
## 🔗 Trecho Completo
```c
static bool timer_callback(struct repeating_timer *t)
{
    poll_value(); // Lê o valor
    if (le_notification_enabled)
    {
        att_server_request_can_send_now_event(con_handle); // Solicita envio
    }
    return true; // Mantém o temporizador ativo
}

static void setup_system(void)
{
    stdio_init_all(); // Inicializa comunicação serial

    // Inicializa o driver CYW43 (Bluetooth)
    if (cyw43_arch_init())
    {
        printf("Erro: Falha ao inicializar cyw43_arch\n");
        return;
    }

    init_bluetooth(); // Configura o Bluetooth

    // Configura o temporizador repetitivo
    static struct repeating_timer timer;
    if (!add_repeating_timer_ms(POLL_INTERVAL_MS, timer_callback, NULL, &timer))
    {
        printf("Erro: Falha ao iniciar o temporizador\n");
    }
}
```