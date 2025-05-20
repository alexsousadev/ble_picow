/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "btstack.h"
#include "server_common.h"

// Constantes
#define POLL_INTERVAL_MS 10000 // Intervalo para leitura e envio (10 segundos)

// Protótipos de funções
static void setup_system(void);
static void init_bluetooth(void);
static bool timer_callback(struct repeating_timer *t);

// Configura o sistema (serial, CYW43, Bluetooth, temporizador)
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
    gap_advertisements_set_data(adv_data_len, (uint8_t *)adv_data);
    gap_advertisements_enable(1);

    hci_power_control(HCI_POWER_ON);
}

// Callback do temporizador: executa leitura e envio de dados
static bool timer_callback(struct repeating_timer *t)
{
    poll_value(); // Lê o valor
    if (le_notification_enabled)
    {
        att_server_request_can_send_now_event(con_handle); // Solicita envio
    }
    return true; // Mantém o temporizador ativo
}

int main(void)
{
    setup_system(); // Configura o sistema
    while (true)
    {
        // Loop infinito, sistema gerenciado por interrupções
    }
    return 0;
}