#include "touch.h"
#include "driver/i2c.h"
#include "freertos/timers.h"
#include "freertos/FreeRTOS.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

#define TAG "FT6236"
#define TOUCH_ADDR 0x38  
#define I2C_SDA_PIN 39
#define I2C_SCL_PIN 40
#define I2C_PORT I2C_NUM_1
#define FT_REG_CHIP_ID 0xA8

#define INACTIVITY_TIMEOUT_MS 15000

i2c_master_bus_handle_t i2c_touch_bus = NULL;
i2c_master_dev_handle_t i2c_touch_dev = NULL;

TimerHandle_t timer_activity_display;

void i2c_touch_bus_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_touch_bus));
}

void i2c_touch_init(void)
{
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TOUCH_ADDR,
        .scl_speed_hz = 400000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_touch_bus, &dev_config, &i2c_touch_dev));
    ESP_LOGI(TAG, "FT6236U touch added to I2C_NUM_1");

    uint8_t chip_id = 0;
    uint8_t reg = FT_REG_CHIP_ID;
    if (i2c_master_transmit_receive(i2c_touch_dev, &reg, 1, &chip_id, 1, -1) == ESP_OK) {
        ESP_LOGI(TAG, "FT6x36 Chip ID: 0x%02X", chip_id);  // 0x64 - FT6236U
    } else {
        ESP_LOGW(TAG, "FT6x36 chip ID read failed");
    }
}

static esp_err_t ft_read_bytes(uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(i2c_touch_dev, &reg, 1, data, len, -1);
}

void read_touch()
{
    uint8_t gesture = 0;
    uint8_t reg_gest = 0x01;

    if (i2c_master_transmit_receive(i2c_touch_dev, &reg_gest, 1, &gesture, 1, -1) == ESP_OK) {
        switch (gesture) {
            case 0x10: ESP_LOGI(TAG, "Gesture: Swipe Up"); break;
            case 0x14: ESP_LOGI(TAG, "Gesture: Swipe Down"); break;
            case 0x18: ESP_LOGI(TAG, "Gesture: Swipe Left"); break;
            case 0x1C: ESP_LOGI(TAG, "Gesture: Swipe Right"); break;
            case 0x48: ESP_LOGI(TAG, "Gesture: Zoom In"); break;
            case 0x49: ESP_LOGI(TAG, "Gesture: Zoom Out"); break;
            default:
                if (gesture != 0x00) {
                    ESP_LOGI(TAG, "Unknown gesture: 0x%02X", gesture);
                }
                break;
        }
    } else {
        ESP_LOGW(TAG, "Failed to read gesture register");
    }

    uint8_t buf[16];
    if (ft_read_bytes(0x00, buf, sizeof(buf)) != ESP_OK) {
        ESP_LOGE(TAG, "I2C read error");
        return;
    }

    uint8_t points = buf[2] & 0x0F;
    ESP_LOGI(TAG, "Touch Points: %d", points);

    if (points >= 1) {
        uint16_t x1 = ((buf[3] & 0x0F) << 8) | buf[4];
        uint16_t y1 = ((buf[5] & 0x0F) << 8) | buf[6];
        ESP_LOGI(TAG, "P1: X=%d Y=%d", x1, y1);
    }
    if (points >= 2) {
        uint16_t x2 = ((buf[9] & 0x0F) << 8) | buf[10];
        uint16_t y2 = ((buf[11] & 0x0F) << 8) | buf[12];
        ESP_LOGI(TAG, "P2: X=%d Y=%d", x2, y2);
    }
}

