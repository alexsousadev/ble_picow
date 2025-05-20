/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include "btstack.h"
#include "server_common.h"
#include "temp_sensor.h"
#include "hardware/adc.h"

#define CONVERSION_FACTOR 3.3f / (1 << 12) // Conversão de ADC para volts
#define TEMP_CHANNEL 4 // Sensor de temperatura interno

// Dados de anúncio BLE
const uint8_t adv_data[] = {
    // Flags
    0x02,
    BLUETOOTH_DATA_TYPE_FLAGS,
    APP_AD_FLAGS,
    // Nome completo: "Pico W"
    0x07,
    BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME,
    'P',
    'i',
    'c',
    'o',
    ' ',
    'W',
    // Nome curto: "Pico"
    0x05,
    BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME,
    'P',
    'i',
    'c',
    'o',
    // UUID do serviço (0x181A - Environmental Sensing)
    0x03,
    BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS,
    0x1A,
    0x18,
};

const uint8_t adv_data_len = sizeof(adv_data);

// Variáveis globais
int le_notification_enabled = 0;
hci_con_handle_t con_handle = 0;
uint16_t current_value = 0;

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

        break;

    default:
        break;
    }
}

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

float read_temperature(void)
{
    static bool adc_initialized = false;
    if (!adc_initialized)
    {
        adc_init();
        adc_set_temp_sensor_enabled(true);
        adc_select_input(4);
        adc_initialized = true;
    }

    uint16_t raw = adc_read();
    float voltage = raw * CONVERSION_FACTOR;
    float temperature = 27.0f - (voltage - 0.706f) / 0.001721f;
    return temperature;
}

void poll_value(void)
{
    float temp = read_temperature();
    current_value = (uint16_t)(temp * 100);
    printf("Temperatura processada: %.2f °C (enviada: %d)\n", temp, current_value);
}
