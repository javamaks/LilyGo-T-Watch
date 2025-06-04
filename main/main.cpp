/**
 * @file      main.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2024-01-07
 *
 */

#include <stdio.h>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "amoled_driver.h"
#include "touch_driver.h"
#include "i2c_driver.h"
#include "power_driver.h"
#include "demos/lv_demos.h"
#include "lv_conf_kconfig.h"
#include "tft_driver.h"
#include "rtc_driver.h"
#include "product_pins.h"
#include "pcf8563.h"
#include "touch.h"

extern TimerHandle_t timer_activity_display;
extern i2c_master_bus_handle_t i2c_bus;

static const char *TAG = "main";

const char *months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November" ,"December"};

extern uint8_t buffer[8];
//extern void rtc_set_time(const rtc_time_t *time);
extern rtc_time_t *time;
#define EXAMPLE_LVGL_TICK_PERIOD_MS 2
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 500
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1
#define EXAMPLE_LVGL_TASK_STACK_SIZE (4 * 1024)
#define EXAMPLE_LVGL_TASK_PRIORITY 2
#define CONFIG_LV_FONT_DEFAULT &lv_font_montserrat_42

//extern bool touch_init();
//extern uint8_t touch_get_data(int16_t *x, int16_t *y, uint8_t point_num);

extern "C" {
    #include "pcf8563.h"
}



static SemaphoreHandle_t lvgl_mux = NULL;

static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)

extern "C" {
    lv_disp_drv_t disp_drv;      // contains callback functions
}


/*void lv_print_hello(){
	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x003a30), LV_PART_MAIN);
	lv_obj_t *label = lv_label_create(lv_scr_act());
	lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
	lv_obj_set_style_transform_angle(label, 1800, LV_PART_MAIN);
	lv_label_set_text(label, "Hello, Java");
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}
*/

void timer_activity_callback(TimerHandle_t timer_activity_display){
	
}
  void create_watchface_screen()
{
	lv_obj_clean(lv_scr_act());
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x948979), 0); // фон

    // --- Иконка батареи ---
    lv_obj_t *bat_icon = lv_img_create(scr);
    LV_IMG_DECLARE(full_battery); 
    lv_img_set_src(bat_icon, &full_battery);
    lv_obj_align(bat_icon, LV_ALIGN_TOP_LEFT, 10, 10);
	lv_obj_set_size(bat_icon, -5, 70);
	
	
    lv_obj_t *label_date = lv_label_create(scr);
	
	
	rtc_time_t currenttime;
	
	rtc_time_t *time_ptr = &currenttime;
	
	bool arc = rtc_get_time(time_ptr);

	rtc_time_t t;
	rtc_get_time(&t);
	char buffer1[40];
	sprintf(buffer1, "%d:%d",t.hours, t.minutes);
	
	char buffer_date[64];
	char buffer_month[16];
	char buffer_weekday[16];
	
	const char *months[] = {
		"January", "February", "March", "April",
		"May", "June", "July", "August",
		"September", "October", "November", "December"
	};


	const char *weekdays[] = {
		"Sunday", "Monday", "Tuesday", "Wednesday",
		"Thursday", "Friday", "Saturday"
	};
	
	if (t.month >= 1 && t.month <= 12) {
		strcpy(buffer_month, months[t.month - 1]);
	} else {
		strcpy(buffer_month, "Unknown");
	}

	if (t.weekday <= 6) {
		strcpy(buffer_weekday, weekdays[t.weekday]);
	} else {
		strcpy(buffer_weekday, "Unknown");
	}
	sprintf(buffer_date, "%d %s 20%02d",t.day, buffer_month, t.year);
    lv_label_set_text(label_date, buffer_date);
    lv_obj_set_style_text_font(label_date, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(label_date, lv_color_white(), 0);
    lv_obj_align(label_date, LV_ALIGN_TOP_RIGHT, -10, 80);

    // --- Время (по центру) ---
    lv_obj_t *label_time = lv_label_create(scr);
    lv_label_set_text(label_time, buffer1);
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_48, 0); // шрифт покрупнее
    lv_obj_set_style_text_color(label_time, lv_color_hex(0x393E46), 0);
    lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 30);

    // --- Steps label ---
    lv_obj_t *label_steps = lv_label_create(scr);
    lv_label_set_text(label_steps, "Steps:");
    lv_obj_set_style_text_font(label_steps, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_steps, lv_color_white(), 0);
    lv_obj_align(label_steps, LV_ALIGN_BOTTOM_LEFT, 10, -40);
	
	

    // --- Steps value ---
    lv_obj_t *label_stepcount = lv_label_create(scr);
    lv_label_set_text(label_stepcount, "5467");
    lv_obj_set_style_text_font(label_stepcount, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(label_stepcount, lv_color_hex(0x222831), 0);
    lv_obj_align_to(label_stepcount, label_steps, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    // --- День недели (справа снизу) ---
    lv_obj_t *label_day = lv_label_create(scr);
    lv_label_set_text(label_day, buffer_weekday);
    lv_obj_set_style_text_font(label_day, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(label_day, lv_color_white(), 0);
    lv_obj_align(label_day, LV_ALIGN_BOTTOM_RIGHT, -10, -20);
}


void lv_print_hello(){
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x003a57), LV_PART_MAIN);

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello world");
    lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}
static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
#if DISPLAY_FULLRESH
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );
    display_push_colors(area->x1, area->y1, w, h, (uint16_t *)color_map);
    lv_disp_flush_ready( drv );
#else
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    display_push_colors(offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, (uint16_t *)color_map);
#endif
}


#if BOARD_HAS_TOUCH
static void example_lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    int16_t touchpad_x[1] = {0};
    int16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    /* Get coordinates */
    touchpad_cnt = touch_get_data(touchpad_x, touchpad_y, 1);

    if (touchpad_cnt > 0) {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
#endif

static void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

bool example_lvgl_lock(int timeout_ms)
{
    // Convert timeout in milliseconds to FreeRTOS ticks
    // If `timeout_ms` is set to -1, the program will block until the condition is met
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_mux, timeout_ticks) == pdTRUE;
}

void example_lvgl_unlock(void)
{
    xSemaphoreGiveRecursive(lvgl_mux);
}

static void example_lvgl_port_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
	int count = 0;
	if(count  <= 0){
		count = count + 1;
		//lv_print_hello();
		//lv_example_get_started_1();
	}
	else if(count >= 1){
		return;
	}
    while (1) {
        // Lock the mutex due to the LVGL APIs are not thread-safe
        if (example_lvgl_lock(-1)) {
            task_delay_ms = lv_timer_handler();
            // Release the mutex
			//lv_print_hello();
			create_watchface_screen();
			rtc_time_t t;
			rtc_get_time(&t);
			ESP_LOGI(TAG, "Time: %02d:%02d:%02d", t.hours, t.minutes, t.seconds);
			read_touch();
			vTaskDelay(pdMS_TO_TICKS(30));
            example_lvgl_unlock();
        }
        if (task_delay_ms > EXAMPLE_LVGL_TASK_MAX_DELAY_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < EXAMPLE_LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}
extern "C" {
    void example_lvgl_demo_ui(lv_disp_t *disp);
}

extern "C" void app_main(void)
{
	timer_activity_display = xTimerCreate("timer_activity", pdMS_TO_TICKS(1000), pdTRUE, NULL, timer_activity_callback);
	
	
	//if(timer_activity_display == NULL){
	//	return false;
	//}
	xTimerStart(timer_activity_display, 0);
	//rtc_set_time(nowPtr);
    ESP_LOGI(TAG, "------ Initialize I2C.");
    //i2c_driver_init();
	
	rtc_user_init();
	rtc_time_t now = {
    .seconds = 10,
    .minutes = 56,
    .hours = 8,
    .day = 19,
    .weekday = 1,
    .month = 5,
    .year = 25,
	};
	rtc_set_time(&now);
	
	

    ESP_LOGI(TAG, "------ Initialize PMU.");
    if (!power_driver_init()) {
        ESP_LOGE(TAG, "ERROR :No find PMU ....");
    }

    ESP_LOGI(TAG, "------ Initialize TOUCH.");
    i2c_touch_bus_init();   // создать I2C-шину
	i2c_touch_init();       // добавить тач

	//touch_init();

	/*int16_t x, y;
	touch_get_data(&x, &y, 1);
	printf("Touch: x = %d, y = %d\n", x, y);
	*/
	
    ESP_LOGI(TAG, "------ Initialize DISPLAY.");
    display_init();


    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
	
	/*rtc_time_t t = {
    .seconds = 0,
    .minutes = 10,
    .hours = 15,
    .day = 7,
    .weekday = 3,
    .month = 5,
    .year = 25
	};
	rtc_user_init();
	rtc_set_time(&t);

	// позже
	rtc_time_t now;
	rtc_get_time(&now);
	printf("%02d:%02d:%02d\n", now.hours, now.minutes, now.seconds);
	*/
	//lv_print_hello();


    // alloc draw buffers used by LVGL
    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
#if CONFIG_SPIRAM

// #if CONFIG_LILYGO_T_RGB
//     // initialize LVGL draw buffers

//     extern void *buf1;
//     extern void *buf2;

//     lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISPLAY_BUFFER_SIZE);
// #else
    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(DISPLAY_BUFFER_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf1);

    lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(DISPLAY_BUFFER_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf2);

    // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISPLAY_BUFFER_SIZE);
// #endif
#else
    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(AMOLED_HEIGHT * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = (lv_color_t *) heap_caps_malloc(AMOLED_HEIGHT * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, AMOLED_HEIGHT * 20);
#endif

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 320;
    disp_drv.flush_cb = example_lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.full_refresh = DISPLAY_FULLRESH;
    lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "lvgl_tick",
        .skip_unhandled_events = false
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

#if BOARD_HAS_TOUCH
    ESP_LOGI(TAG, "Register touch driver to LVGL");
    static lv_indev_drv_t indev_drv; // Input device driver (Touch)
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = example_lvgl_touch_cb;
    lv_indev_drv_register(&indev_drv);
#endif

    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    assert(lvgl_mux);


    ESP_LOGI(TAG, "Display LVGL");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (example_lvgl_lock(-1)) {

// #if   CONFIG_USE_DEMO_WIDGETS
        // lv_demo_widgets();
// #elif CONFIG_USE_DEMO_BENCHMARK
        // lv_demo_benchmark();
// #elif CONFIG_USE_DEMO_STRESS
//         lv_demo_stress();
// #elif CONFIG_USE_DEMO_MUSIC
        //lv_demo_music();
// #endif
        // Release the mutex
        example_lvgl_unlock();
    }
    // tskIDLE_PRIORITY,
    ESP_LOGI(TAG, "Create LVGL task");
    // xTaskCreate(example_lvgl_port_task, "LVGL", EXAMPLE_LVGL_TASK_STACK_SIZE, NULL, EXAMPLE_LVGL_TASK_PRIORITY, NULL);
    xTaskCreate(example_lvgl_port_task, "LVGL", EXAMPLE_LVGL_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

}
