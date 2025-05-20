#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

#include "btstack.h"

// Dados do perfil GATT
extern const uint8_t profile_data[];

// Dados de anúncio BLE
#define APP_AD_FLAGS 0x06
extern const uint8_t adv_data[];
extern const uint8_t adv_data_len;

// Variáveis globais
extern int le_notification_enabled;
extern hci_con_handle_t con_handle;
extern uint16_t current_value; // Atualizado de uint8_t para uint16_t

// Protótipos de função
void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
void poll_value(void);
void setup_advertisements(void);

#endif // SERVER_COMMON_H
