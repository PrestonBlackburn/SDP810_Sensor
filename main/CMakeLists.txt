idf_component_register(SRCS "pressure_sensor_main.c"
                       PRIV_REQUIRES esp_driver_i2c spi_flash
                       INCLUDE_DIRS "")
