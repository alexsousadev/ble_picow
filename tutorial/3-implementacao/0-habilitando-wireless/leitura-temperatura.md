# Captando os dados de temperatura

Como estamos realizando a leitura da temperatura, primeiro vamos configurar o ADC para capturar esses dados. Essa parte não tem a ver com o Bluetooth, mas é essencial.

1. Importamos a biblioteca ADC (não esqueça de colocar no CMake) e definimos o canal do sensor de temperatura interno (que é 4) e um fator de conversão para converter os valores lidos pelo ADC em Volts.

    ```c
    #include "hardware/adc.h"
    #include "stdio.h" // Apenas para poder usar o printf

    #define CONVERSION_FACTOR 3.3f / (1 << 12) // Conversão de ADC para volts
    #define TEMP_CHANNEL 4 // Sensor de temperatura interno
    ```

2. Vamos definir a variável que vai armazenar o valor lido do sensor de temperatura e uma flag que vai sinalizar que o cliente está pronto para receber os dados.

    ```c
    float current_value = 0;
    hci_con_handle_t con_handle = 0;
    int le_notification_enabled = 0;
    ```
    > `hci_con_handle_t` é um tipo de dado que representa um identificador único para um cliente conectado. Isso será útil para identificar o cliente que está enviando os dados (nossa placa)

3. Agora vamos fazer a leitura da temperatura de fato. primeiro inicializamos o sensor:

    ```c
    // Inicialização do sensor de temperatura
    void init_temperature_sensor()
    {
        adc_init();
        adc_set_temp_sensor_enabled(true);
        adc_select_input(TEMP_CHANNEL);
    }
    ```

    Depois, criamos uma função que irá ler o valor da temperatura e retornar o valor:

    ```c
    // Leitura da temperatura
    float read_temperature(void)
    {
        uint16_t raw = adc_read();
        float voltage = raw * CONVERSION_FACTOR;
        float temperature = 27.0f - (voltage - 0.706f) / 0.001721f;
        return temperature;
    }
    ```
    > Como essa parte não tem a ver com o tutorial, não irei explicar em detalhes para não alongar ainda mais.

4. Por fim, vamos fazer uma função que ir ler o valor da temperatura e atualizar a variável `current_value`.

    ```c
    void poll_value(void)
    {
        float temp = read_temperature();
        current_value = (uint16_t)(temp * 100);
        printf("Temperatura processada: %.2f °C (enviada: %d)\n", temp, current_value);
    }
    ``` 
    > Essa função será responsável por ler o valor da temperatura e atualizar a variável que armazena o valor atual, permitindo que o cliente possar ler posteriormente.

---
## 🔗 Trecho Completo

```c
#define CONVERSION_FACTOR 3.3f / (1 << 12) // Conversão de ADC para volts
#define TEMP_CHANNEL 4 // Sensor de temperatura interno

// Variáveis globais
int le_notification_enabled = 0;
hci_con_handle_t con_handle = 0;
uint16_t current_value = 0;

// Inicialização do sensor de temperatura
float read_temperature(void)
    {
        uint16_t raw = adc_read();
        float voltage = raw * CONVERSION_FACTOR;
        float temperature = 27.0f - (voltage - 0.706f) / 0.001721f;
        return temperature;
    }

// Leitura da temperatura
float read_temperature(void)
    {
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
```