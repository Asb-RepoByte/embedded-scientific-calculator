// buch of utilites
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
// for memory allocation and task creation
#include "expression.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "esp_timer.h"
// for keyboard matrice
#include "driver/gpio.h"
// these are for the spi and lcd confiuration to enable the dispaly and fonts
#include "math_parser.h"
#include "st7789.h"
#include "fontx.h"

#include "keymap.h"

// for keyboard matrix
#define COL_NUM 4
#define ROW_NUM 6

// for lcdst7789 gpios
#define GPIO_SCLK 18
#define GPIO_MOSI 23
#define GPIO_RESET 22
#define GPIO_DC 3
#define GPIO_CS -1
#define GPIO_BL -1
#define GPIO_MISO -1

// screen resolution
#define DIS_WIDTH 240
#define DIS_HEIGHT 240

// utilities for quality of life
#define TAG "ff"
#define WAIT vTaskDelay(200);

#define ADJUST_X(x) (DIS_WIDTH - x)

#define HOLD_GAP 500 //ms half a second


// gpio maps
// gpio reserved for LCD spi 2:DC, 4:RES, 18:SCK, 23:MOSI (2, 4, 18, 23)

// cols -> input  (pulldown)                              (26, 27, 14, 12)
// rows -> output (set to 1{HIGH} to indicate a press)    (15, 2, 0, 4, 16, 17) gpio26 have been skiped due to a loose connection in the breadboard


void handle_key(int row, int col);


void vReadKBD(void *vpParameter) {

    TickType_t updateTime = pdMS_TO_TICKS(20);
    TickType_t lastWake = xTaskGetTickCount();


    const gpio_num_t cols_gpio[COL_NUM] = {26, 27, 14, 12}; 
    const gpio_num_t rows_gpio[ROW_NUM] = {15, 2, 4, 0, 16, 17};

    for (uint8_t r = 0; r < ROW_NUM; r++) {
        gpio_reset_pin(rows_gpio[r]);
        gpio_set_direction(rows_gpio[r], GPIO_MODE_OUTPUT);
        gpio_set_level(rows_gpio[r], 0);
    }
    for (uint8_t c = 0; c < COL_NUM; c++) {
        gpio_reset_pin(cols_gpio[c]);
        gpio_set_direction(cols_gpio[c], GPIO_MODE_INPUT);
        gpio_set_pull_mode(cols_gpio[c], GPIO_PULLDOWN_ENABLE); // gpio34-39 can only be pulldown not up
    }

    char kdb_value_prev[ROW_NUM][COL_NUM] = {0};
    char kdb_value[ROW_NUM][COL_NUM] = {0};
    ESP_LOGI("kbd", "hello there this is me no doubt just to make it long and apparent");

    bool should_restart_timer = false;
    int64_t last_time = esp_timer_get_time();
    int64_t current_time = esp_timer_get_time();

    
    for (;;) {
        should_restart_timer = false;
        for (uint8_t r = 0; r < ROW_NUM; r++) {
            gpio_set_level(rows_gpio[r], 1);
            for (uint8_t c = 0; c < COL_NUM; c++) {
                current_time = esp_timer_get_time();
                kdb_value_prev[r][c] = kdb_value[r][c];
                kdb_value[r][c] = gpio_get_level(cols_gpio[c]);
                if (kdb_value[r][c] == 1 && (kdb_value_prev[r][c] == 0 || (current_time - last_time) / 1000.0f > HOLD_GAP)) {
                    should_restart_timer = true;
                    handle_key(r, c);
                    ESP_LOGI("time", "[%f]", (current_time - last_time) / 1000.0f);
                    ESP_LOGI("kbd", "[%d], [%d]: pressed", r, c);
                    ESP_LOGI("expr", "%s", expr_get_str(expr_get()));
                }
                    
            }
            gpio_set_level(rows_gpio[r], 0);
        }
        if (should_restart_timer)
            last_time = current_time;
        vTaskDelayUntil(&lastWake, updateTime);
    }
    
}

static void listSPIFFS(char * path) {
	DIR* dir = opendir(path);
	assert(dir != NULL);
	while (true) {
		struct dirent*pe = readdir(dir);
		if (!pe) break;
		ESP_LOGI(__FUNCTION__,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
	}
	closedir(dir);
}

esp_err_t mountSPIFFS(char * path, char * label, int max_files) {
	esp_vfs_spiffs_conf_t conf = {
		.base_path = path,
		.partition_label = label,
		.max_files = max_files,
		.format_if_mount_failed = true
	};

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret ==ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret== ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",esp_err_to_name(ret));
		}
		return ret;
	}

#if 0
	ESP_LOGI(TAG, "Performing SPIFFS_check().");
	ret = esp_spiffs_check(conf.partition_label);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
		return ret;
	} else {
			ESP_LOGI(TAG, "SPIFFS_check() successful");
	}
#endif

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(conf.partition_label, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"Failed to get SPIFFS partition information (%s)",esp_err_to_name(ret));
	} else {
		ESP_LOGI(TAG,"Mount %s to %s success", path, label);
		ESP_LOGI(TAG,"Partition size: total: %d, used: %d", total, used);
	}

	return ret;
}

void vUpdateLCD(void *vpParameter) {
    ESP_ERROR_CHECK(mountSPIFFS("/fonts", "storage1", 7));
    listSPIFFS("/fonts/");

    TFT_t dev;

    spi_master_init(&dev, GPIO_MOSI,  GPIO_SCLK, GPIO_CS, GPIO_DC, GPIO_RESET, GPIO_BL);
    lcdInit(&dev, DIS_WIDTH, DIS_HEIGHT, 0, 0);
    lcdSetFontDirection(&dev, DIRECTION270);

    FontxFile fx[2];
    InitFontx(fx, "/fonts/ILGH24XB.FNT", "");

    uint8_t expr_str[MAX_EXP_SIZE];
    lcdFillScreen(&dev, GREEN);
    lcdDrawFinish(&dev);
    WAIT

    char result[1000];
    int n = 0;
    uint8_t fw, fh;
    GetFontx(fx, 0, &fw, &fh);
    ESP_LOGI("font", "WIDTH: %ud", fw);
    ESP_LOGI("font", "WIDTH: %ud", fh);

    int64_t start, end;
    int cursor;
    uint16_t cursorX, cursorY;
    cursorX = cursorY = 0;

    uint16_t offsetX = 0;

    uint16_t count = 0;
    uint16_t exprX = 0;
    uint16_t exprY = 0;
    uint16_t exprW = 0;


    for (;;) {
        // clearing the screen each frame
        start = esp_timer_get_time();
        lcdFillScreen(&dev, BLACK);

        cursor = expr_get()->cursor;
        cursorX = cursor * fw;
        cursorY = 0;

        strcpy((char *)expr_str, expr_get_str(expr_get()));
        n = strlen((char *)expr_str);

        exprW = n * fw;

        offsetX = 0;
        if (cursorX - 227 > fw) {
            offsetX = cursorX - 227;
        }

        //draw the expression
        lcdDrawFillRect(&dev, cursorY, DIS_WIDTH - 1 - ((cursorX - offsetX) + fw) , cursorY + fh, DIS_WIDTH - 1 - (cursorX - offsetX), BLUE);
        lcdDrawString(&dev, fx, fh - 1, DIS_HEIGHT - 1 + offsetX, (uint8_t *)expr_str, WHITE);
        // drawing the cursor

        // draw the line separating the result of the expression
        if (offsetX != 0)
            lcdDrawFillArrow(&dev, 180, 220, 170, 230, 5, RED);
        if (cursor != expr_get()->len)
            lcdDrawFillArrow(&dev, 180, 20, 170, 10, 5, GREEN);

        lcdDrawFillRect(&dev, DIS_WIDTH - 1 - fh * 2, 0, DIS_WIDTH - 1 - fh * 2 + fw, DIS_HEIGHT, PURPLE);

        // drawing the result
        snprintf(result, sizeof(result), "%g", expr_get()->result);
        lcdDrawString(&dev, fx, DIS_WIDTH - fh / 2 , DIS_HEIGHT - 1, (uint8_t *)result, WHITE);

        lcdDrawFinish(&dev);

        end = esp_timer_get_time();

        //ESP_LOGI("time", "drawing string duration: %lld us, (%.2f ms)", end - start, (end - start) / 1000.0);

        //snprintf(result, sizeof(result), "%.4fd", expr_evaluate(expr_get()));
        //lcdDrawString(&dev, fx, 100, 100, (uint8_t *)expr_str, BLUE);
        count += 1;
        vTaskDelay(pdMS_TO_TICKS(100));
    }

}

void app_main(void)
{
    xTaskCreate(vReadKBD, "read kdb", 10000, NULL, 1, NULL);
    xTaskCreate(vUpdateLCD, "update lcd", 10000, NULL, 1, NULL);
    vTaskDelete(NULL);
}

void handle_key(int row, int col) {
    key_action_t action = get_action(row, col);

    switch(action.type) {
        case KEY_TYPE_CHAR:
        case KEY_TYPE_STRING:
            expr_insert(expr_get(), action.output);
            break;
        case KEY_TYPE_FUNC:
            if (action.func) action.func();
            break;

    }

}
