#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "zh_network.h"

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]

void zh_network_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

constexpr auto kIsServer = true;

uint8_t target[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

QueueHandle_t queue1;

typedef struct {
    int64_t base_micros; 
} time_sync_message_t;

void send_time(void *params) {
    time_sync_message_t message = {0};
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    for (int i = 0; i < 10; i++) {
        message.base_micros = esp_timer_get_time();
        zh_network_send(target, (uint8_t *)&message, sizeof(message));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
} 

void blink(void *params) {

    int64_t inter_esp_micros = 0;

    const gpio_num_t GP = GPIO_NUM_14;
    gpio_reset_pin(GP);
    gpio_set_direction(GP, GPIO_MODE_OUTPUT);
    while (true) {
        int64_t master_micros = 0;
        if (xQueueReceive(queue1, &master_micros, 0)) {
            inter_esp_micros = esp_timer_get_time() - master_micros - 1000; // The 1000 is to compensate for the delay in the send_time task
            printf("Master micros %lld\n", master_micros);
        }

        int64_t cycle_time = (esp_timer_get_time() - inter_esp_micros) % 250000;
        if (cycle_time < 25000) {
            gpio_set_level(GP, 1);
        } else {
            gpio_set_level(GP, 0);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

}

extern "C" {
    void app_main(void);
}

void app_main(void) {
    esp_log_level_set("zh_vector", ESP_LOG_NONE);
    esp_log_level_set("zh_network", ESP_LOG_NONE);
//    if (queue1 == NULL)
        queue1 = xQueueCreate(5, sizeof(int64_t));

    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_init_config);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    zh_network_init_config_t network_init_config = ZH_NETWORK_INIT_CONFIG_DEFAULT();
    zh_network_init(&network_init_config);
    esp_event_handler_instance_register(ZH_NETWORK, ESP_EVENT_ANY_ID, &zh_network_event_handler, NULL, NULL);

    if (kIsServer) {
        xTaskCreate(send_time, "send_time", 4096, NULL, 1, NULL);
    }
    xTaskCreate(blink, "blink", 4096, NULL, 1, NULL);
}

void zh_network_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{


    switch (event_id)
    {
    case ZH_NETWORK_ON_RECV_EVENT:;
        {
            zh_network_event_on_recv_t *recv_data = (zh_network_event_on_recv_t*) event_data;
            printf("Message from MAC %02X:%02X:%02X:%02X:%02X:%02X is received. Data lenght %d bytes.\n", MAC2STR(recv_data->mac_addr), recv_data->data_len);
            time_sync_message_t *recv_message = (time_sync_message_t *)recv_data->data;
            printf("Hop count %d\n", recv_data->hop_count);
            xQueueSend(queue1, &recv_message->base_micros, 0);
            heap_caps_free(recv_data->data);
            break;
        }
    case ZH_NETWORK_ON_SEND_EVENT:;
        {
            zh_network_event_on_send_t *send_data = (zh_network_event_on_send_t*) event_data;
            if (send_data->status == ZH_NETWORK_SEND_SUCCESS)
            {
                printf("Message to MAC %02X:%02X:%02X:%02X:%02X:%02X sent success.\n", MAC2STR(send_data->mac_addr));
            }
            else
            {
                printf("Message to MAC %02X:%02X:%02X:%02X:%02X:%02X sent fail.\n", MAC2STR(send_data->mac_addr));
            }
            break;
        }
    default:
        break;
    }
}

