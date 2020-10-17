/*------------------------------------------------------------*-
  Indicating LED - header file
  (c) Minh-An Dao 2020
  version 1.00 - 13/10/2020
---------------------------------------------------------------
 * Setup LED for indicating network status.
 * Based on ledc lib.
 * 
 --------------------------------------------------------------*/
#ifndef __LED_C
#define __LED_C
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_err.h"


// ------ Private constants -----------------------------------
#define BLINK_GPIO       (CONFIG_LED_PIN)
#define BLINK_INTERVAL   (CONFIG_LED_INTERVAL)
typedef enum {
    LED_BLINK,
    LED_BLINK_FAST,
    LED_LIT,
    LED_OFF
} led_state_t;
// ------ Private function prototypes -------------------------
// ------ Private variables -----------------------------------
xQueueHandle _blink_queue;
led_state_t _led_status = LED_BLINK;

// ------ PUBLIC variable definitions -------------------------
//--------------------------------------------------------------
// FUNCTION DEFINITIONS
//--------------------------------------------------------------
static void blink_task(void* arg)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    led_state_t status_now;
    while(1) {
        if(xQueueReceive(_blink_queue, &status_now, 0)) //no waiting, just checking for any news
        {
            _led_status = status_now; 
        }
        switch (_led_status)
        {
        case LED_BLINK:
            /* LED off (output low) */
            gpio_set_level(BLINK_GPIO, 0);
            vTaskDelay(BLINK_INTERVAL / portTICK_PERIOD_MS);
            /* LED on (output high) */
            gpio_set_level(BLINK_GPIO, 1);
            vTaskDelay(BLINK_INTERVAL / portTICK_PERIOD_MS);
            break;
        case LED_BLINK_FAST:
            /* LED off (output low) */
            gpio_set_level(BLINK_GPIO, 0);
            vTaskDelay((uint16_t)(BLINK_INTERVAL/5)/portTICK_PERIOD_MS);
            /* LED on (output high) */
            gpio_set_level(BLINK_GPIO, 1);
            vTaskDelay((uint16_t)(BLINK_INTERVAL/5)/portTICK_PERIOD_MS);
            break;
        case LED_LIT:
            /* LED off (output low) --> turn on LED */
            gpio_set_level(BLINK_GPIO, 0);
            break;
        default:
            /* LED on (output high) --> turn off LED */
            gpio_set_level(BLINK_GPIO, 1);
            break;
        }
    }
}

void led_init(void)
{
    _blink_queue = xQueueCreate(3, sizeof(led_state_t));
    //------------ blink task -----------------
    xTaskCreate(
        &blink_task,    /* Task Function */
        "blink task",   /* Name of Task */
        1024,           /* Stack size of Task */
        NULL,           /* Parameter of the task */
        0,              /* Priority of the task, vary from 0 to N, bigger means higher piority, need to be 0 to be lower than the watchdog*/
        NULL);          /* Task handle to keep track of created task */

}

void led_lit(void)
{
    led_state_t led_now = LED_LIT;
    xQueueSend(_blink_queue, &led_now,  portMAX_DELAY);     
}

void led_off(void)
{
    led_state_t led_now = LED_OFF;
    xQueueSend(_blink_queue, &led_now,  portMAX_DELAY);   
}

void led_blink(void)
{
    led_state_t led_now = LED_BLINK;
    xQueueSend(_blink_queue, &led_now,  portMAX_DELAY);    
}

void led_fastblink(void)
{
    led_state_t led_now = LED_BLINK_FAST;
    xQueueSend(_blink_queue, &led_now,  portMAX_DELAY);    
}
#endif