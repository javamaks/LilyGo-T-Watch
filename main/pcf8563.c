#include "pcf8563.h"
#include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#define PCF8563_ADDR 0x51
#define SDA_PIN 10
#define SCL_PIN 11

i2c_master_bus_handle_t i2c_bus = NULL;
static i2c_master_dev_handle_t i2c_dev = NULL;



static uint8_t dec2bcd(uint8_t val) { return ((val / 10) << 4) | (val % 10); }
static uint8_t bcd2dec(uint8_t val) { return ((val >> 4) * 10) + (val & 0x0F); }



void rtc_user_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PCF8563_ADDR,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &dev_config, &i2c_dev));

    uint8_t init_cmd[2] = {0x00, 0x00};
    ESP_ERROR_CHECK(i2c_master_transmit(i2c_dev, init_cmd, sizeof(init_cmd), -1));
}


void rtc_set_time(const rtc_time_t *time)
{
    uint8_t data[8] = {
        0x02,
        dec2bcd(time->seconds) & 0x7F,
        dec2bcd(time->minutes),
        dec2bcd(time->hours),
        dec2bcd(time->day),
        dec2bcd(time->weekday),
        dec2bcd(time->month) & 0x1F,
        dec2bcd(time->year)
    };

    ESP_ERROR_CHECK(i2c_master_transmit(i2c_dev, data, sizeof(data), -1));
}


bool rtc_get_time(rtc_time_t *time)
{
    uint8_t reg = 0x02;
    uint8_t data[7];

    esp_err_t err = i2c_master_transmit_receive(i2c_dev, &reg, 1, data, sizeof(data), -1);
    if (err != ESP_OK) {
        ESP_LOGE("RTC", "Read error: %s", esp_err_to_name(err));
        return false;
    }

    time->seconds = bcd2dec(data[0] & 0x7F);
    time->minutes = bcd2dec(data[1] & 0x7F);
    time->hours   = bcd2dec(data[2] & 0x3F);
    time->day     = bcd2dec(data[3] & 0x3F);
    time->weekday = bcd2dec(data[4] & 0x07);
    time->month   = bcd2dec(data[5] & 0x1F);
    time->year    = bcd2dec(data[6]);

    return true;
}

