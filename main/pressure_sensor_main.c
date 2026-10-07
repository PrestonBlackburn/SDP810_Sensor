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
#define I2C_MASTER_FREQ_HZ          100000 /*!< I2C master clock frequency 400kHz-1000kHz */
// #define I2C_MASTER_TIMEOUT_MS       1000
#define SENSOR_ADDR         0x25        /*!< Address of the Sensiron sensor */

// Sensor properties
#define PRESSURE_SCALE_FACTOR 240 /* Pascals */
#define TEMP_SCALE_FACTOR 200 /* C */

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing I2C Master Bus...");
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

    esp_err_t p = i2c_master_probe(bus_handle, SENSOR_ADDR, 100);
    ESP_LOGI(TAG, "Probe 0x%02X: %s", SENSOR_ADDR, esp_err_to_name(p));

    // Reset any existing continuous measurement /soft reset
    uint8_t stop_cmd[] = {0x3F, 0xF9};
    esp_err_t stop_ret = i2c_master_transmit(dev_handle, stop_cmd, sizeof(stop_cmd), 100);
    vTaskDelay(pdMS_TO_TICKS(20));
    if (stop_ret == ESP_OK) {
        ESP_LOGI(TAG, "Stopped any existing measurements");
    } else {
        ESP_LOGE(TAG, "Stop Config Failed: %s", esp_err_to_name(stop_ret));
        return;
    }

    // Set to Continuous Measurement Mode 
    // 0x361E: Diff Pressure, no averaging 0.5ms update
    uint8_t config_cmd[] = {0x36, 0x1E};    

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
        return;
    }

    ESP_LOGI(TAG, "Continuous Measurement Mode Started");
    // First measurement after 8ms
    vTaskDelay(pdMS_TO_TICKS(15));

    // Read Loop
    // 3 bytes DP (MSB, LSB, CRC), 3 bytes Temp (MSB, LSB, CRC)
    uint8_t raw_data[9] = {0};
    while (1) {
        ret = i2c_master_receive(
            dev_handle, 
            raw_data, 
            sizeof(raw_data), 
            1000
        );

        int16_t raw_dp = 0;
        int16_t raw_temp = 0;
        int16_t scale_factor = 0;
        if (ret == ESP_OK) {
            raw_dp = (int16_t)((raw_data[0] << 8) | raw_data[1]);
            raw_temp = (int16_t)((raw_data[3] << 8) | raw_data[4]);
            scale_factor = (int16_t)((raw_data[6] << 8) | raw_data[7]);

            // ESP_LOGI(TAG, "Raw DP: %d | Raw Temp: %d", raw_dp, raw_temp);
        } else {
            ESP_LOGE(TAG, "Read failed: %s", esp_err_to_name(ret));
        }

        if (ret == ESP_OK) {
            // calc actual diff pressure values (Pa)
            float pressure_pa = 0.0f;
            if (scale_factor != 0) {
                pressure_pa = (float)raw_dp / (float)scale_factor;
            }
            float temp_c = (float)raw_temp / 200.0f;
            ESP_LOGI(TAG, "Pressure: %.4f Pa | Temp: %.2f C | (Scale Factor: %d)", 
                     pressure_pa, temp_c, scale_factor);
        } else {
            ESP_LOGE(TAG, "Calc failed: %s", esp_err_to_name(ret));
        }
        vTaskDelay(pdMS_TO_TICKS(5));
        
    }
}
