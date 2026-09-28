/*
 * SPDX-FileCopyrightText: 2010-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "diff_pressure";

#define I2C_MASTER_SCL_IO           9/*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           8 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0 /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          400000 /*!< I2C master clock frequency */
// #define I2C_MASTER_TIMEOUT_MS       1000
#define SENSOR_ADDR         0x00        /*!< Address of the Sensiron sensor */

// Sensor properties
#define MAX_PRESSURE 500
#define MIN_PRESSURE -500
#define MAX_TEMP 80
#define MIN_TEMP -40

void print_bits(uint8_t *value) {
    for (int i = 7; i>=0; i--) {
        uint8_t bit  = (*value >> i) & 1;
        printf("%d", bit);
    }
    printf("\n");
}

void app_main(void)
{
    printf("Startup -- \n");
    /* Print chip information */
   
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SENSOR_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));

    ESP_LOGI(TAG, "I2C Initialized successfully");
    ESP_LOGI(TAG, "Starting continuous I2C read loop...");

    while (1) {
        // config sequence - (0xAA, 0x00, 0x80)
        uint8_t config_cmd[] = {0xAA, 0x00, 0x80};

        esp_err_t ret = i2c_master_transmit(
            dev_handle,
            config_cmd,
            sizeof(config_cmd),
            1000
        );

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Config success");
        } else {
            ESP_LOGE(TAG, "Config Failed: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    
        uint8_t raw_data[7] = {0};
        ret = i2c_master_receive(
            dev_handle, 
            raw_data, 
            sizeof(raw_data), 
            1000
        );

        if (ret == ESP_OK) {
            
            printf("Got Bytes: \n");
            size_t raw_len = sizeof(raw_data) / sizeof(raw_data[0]);
            for (size_t i = 0; i < raw_len; i++) {
                printf("Bit %d: ", i);
                print_bits(&raw_data[i]);
            }
            uint8_t status = raw_data[0]; 
            // reconstruct 24-bit values
            uint32_t raw_p_24 = ((uint32_t)raw_data[1] << 16) | ((uint32_t)raw_data[2] << 8) | (uint32_t)raw_data[3];
            uint32_t raw_t_24 = ((uint32_t)raw_data[4] << 16) | ((uint32_t)raw_data[5] << 8) | (uint32_t)raw_data[6];
            // shift based on actual data
            uint16_t pressure_bits = (raw_p_24 >> 10) & 0x3FFF; // 14 bit
            uint16_t temp_bits = (raw_t_24 >> 8) & 0xFFFF; // 16 bit
            
            uint32_t timestamp_ms = pdTICKS_TO_MS(xTaskGetTickCount());

            float pressure_pa = (((MAX_PRESSURE - MIN_PRESSURE) * (float)pressure_bits) / pow(2, 14))  + MIN_PRESSURE;
            float temp_c = (((MAX_TEMP - MIN_TEMP) * (float)temp_bits) / pow(2, 16)) + MIN_TEMP;
            printf("DATA,%lu,%.2f,%.2f\n", (unsigned long)timestamp_ms, pressure_pa, temp_c);
           
            
            ESP_LOGI(TAG, "32 bit counts: \nPresure: %u | Temp: %u\n", raw_p_24, raw_t_24);
            ESP_LOGI(TAG, "raw counts: \nPressure: %u | Temp: %u\n", pressure_bits, temp_bits);
        } else {
            ESP_LOGE(TAG, "Read Failed: %s", esp_err_to_name(ret));
        }

    }


}
