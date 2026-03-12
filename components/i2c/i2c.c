#include "i2c.h"

static i2c_master_bus_handle_t bus_handle;


esp_err_t i2c_init(void)
{
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = SCL_IO_NUM,
        .sda_io_num = SDA_IO_NUM,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
   
    
    return ESP_OK;
}

esp_err_t i2c_add_device(uint16_t device_addr, i2c_master_dev_handle_t *dev_handle){
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = device_addr,
        .scl_speed_hz = I2C_CLOCK_SPEED_HZ,
    };

    return i2c_master_bus_add_device(bus_handle, &dev_cfg, dev_handle);
}

esp_err_t i2c_delete_handle(){
    return i2c_del_master_bus(bus_handle);
}