/* ethernet Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_err.h"
#include "esp_event_loop.h"
#include "esp_event.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_eth.h"

#include "rom/ets_sys.h"
#include "rom/gpio.h"

#include "soc/dport_reg.h"
#include "soc/io_mux_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_sig_map.h"

#include "tcpip_adapter.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

static const char *TAG = "eth_demo";

#define PIN_PHY_POWER 17
#define PIN_SMI_MDC   23
#define PIN_SMI_MDIO  18


void phy_device_power_enable(bool enable)
{
    gpio_pad_select_gpio(PIN_PHY_POWER);
    gpio_set_direction(PIN_PHY_POWER,GPIO_MODE_OUTPUT);
    if(enable == true) {
        gpio_set_level(PIN_PHY_POWER, 1);
        ESP_LOGD(TAG, "phy_device_power_enable(TRUE)");
    } else {
        gpio_set_level(PIN_PHY_POWER, 0);
        ESP_LOGD(TAG, "power_enable(FALSE)");
    }
}

void eth_gpio_config_rmii(void)
{
    //crs_dv to gpio27 ,can not change (default so not needed but physical signal must be connected)
    //PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO27_U, FUNC_GPIO27_EMAC_RX_DV);

    //txd0 to gpio19 ,can not change
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO19_U, FUNC_GPIO19_EMAC_TXD0);
    //tx_en to gpio21 ,can not change
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO21_U, FUNC_GPIO21_EMAC_TX_EN);
    //txd1 to gpio22 , can not change
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO22_U, FUNC_GPIO22_EMAC_TXD1);
    //rxd0 to gpio25 , can not change
    gpio_set_direction(25, GPIO_MODE_INPUT);
    //rxd1 to gpio26 ,can not change
    gpio_set_direction(26, GPIO_MODE_INPUT);
    //rmii clk  ,can not change
    gpio_set_direction(0, GPIO_MODE_INPUT);

    //mdc to gpio23
    gpio_matrix_out(PIN_SMI_MDC, EMAC_MDC_O_IDX, 0, 0);
    //mdio to gpio18
    gpio_matrix_out(PIN_SMI_MDIO, EMAC_MDO_O_IDX, 0, 0);
    gpio_matrix_in(PIN_SMI_MDIO, EMAC_MDI_I_IDX, 0);
}

void eth_task(void *pvParameter)
{
    tcpip_adapter_ip_info_t ip;
    memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    while (1) {

        vTaskDelay(2000 / portTICK_PERIOD_MS);

        if (tcpip_adapter_get_ip_info(ESP_IF_ETH, &ip) == 0) {
            ESP_LOGI(TAG, "~~~~~~~~~~~");
            ESP_LOGI(TAG, "ETHIP:"IPSTR, IP2STR(&ip.ip));
            ESP_LOGI(TAG, "ETHPMASK:"IPSTR, IP2STR(&ip.netmask));
            ESP_LOGI(TAG, "ETHPGW:"IPSTR, IP2STR(&ip.gw));
            ESP_LOGI(TAG, "~~~~~~~~~~~");
        }
    }
}

void app_main()
{
    esp_err_t ret = ESP_OK;
    tcpip_adapter_init();
    esp_event_loop_init(NULL, NULL);

    eth_config_t config = default_ethernet_phy_config;
    config.gpio_config = eth_gpio_config_rmii;
    config.tcpip_input = tcpip_adapter_eth_input;
    config.phy_power_enable = phy_device_power_enable;

    ret = esp_eth_init(&config);

    if(ret == ESP_OK) {
        esp_eth_enable();
        xTaskCreate(eth_task, "eth_task", 2048, NULL, (tskIDLE_PRIORITY + 2), NULL);
    }

}
