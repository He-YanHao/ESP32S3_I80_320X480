#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp_exio.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    vTaskDelay(100 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(bsp_exio_port_init());
    ESP_LOGI(TAG, "EXIO initialized");

    int count = 0;
    while (1) {
        set_led1_up();
        ESP_LOGI(TAG, "LED ON (%d)", ++count);
        vTaskDelay(pdMS_TO_TICKS(500));

        set_led1_down();
        ESP_LOGI(TAG, "LED OFF");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}