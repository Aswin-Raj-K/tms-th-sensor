idf_component_register(SRCS "https_client.c"
                    INCLUDE_DIRS "."
                    REQUIRES json https_client device esp-tls sht3x ssd1306 i2c data_storage esp_time)
