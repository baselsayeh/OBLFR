/*
 * Copyright (c) 2016-2023 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Bouffalo Lab nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/****************************************************************************
 * Defines
 */
/*#define pdTRUE 1*/
#include <stdbool.h>

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <bl_os_hal.h>
#include <bl_os_adapter/bl_os_adapter.h>
#include <bl_os_adapter/bl_os_log.h>

#include <bflb_irq.h>

//#ifdef OS_USING_FREERTOS
/*#include <stdio.h>
#include <stdarg.h>*/

#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <event_groups.h>
#include <message_buffer.h>
#include <timers.h>
//#include <blog.h>
//#include <aos/yloop.h>

//#include <bl_irq.h>
//#endif

//Logging
#define DBG_TAG "WIFI_HAL"
#include "log.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct mq_adpt
{
};

struct irq_adpt
{
    void (*func)(void *arg); /* Interrupt callback function */
    void *arg;               /* Interrupt private data */
};

enum bl_os_timer_mode
{
    BL_OS_TIEMR_ONCE = 0,
    BL_OS_TIEMR_CYCLE
};

typedef enum bl_os_timer_mode bl_os_timer_mode_t;

struct timer_adpt
{
    TimerHandle_t handle;
    int32_t delay;
    bl_os_timer_mode_t mode;
    void (*func)(void *arg);
    void *arg;
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

extern void *__attribute__((weak)) _wifi_log_flag;

bl_ops_funcs_t g_bl_ops_funcs =
{
    ._version = BL_OS_ADAPTER_VERSION,
    ._printf = bl_os_printf,
    ._puts = bl_os_puts,
    ._assert = bl_os_assert_func,
    ._init = bl_os_api_init,
    ._enter_critical = bl_os_enter_critical,
    ._exit_critical = bl_os_exit_critical,
    ._msleep = bl_os_msleep,
    ._sleep = bl_os_sleep,
    ._event_group_create = bl_os_event_create,
    ._event_group_delete = bl_os_event_delete,
    ._event_group_send = bl_os_event_send,
    ._event_group_wait = bl_os_event_wait,
    ._event_register = bl_os_event_register,
    ._event_notify = bl_os_event_notify,
    ._task_create = bl_os_task_create,
    ._task_delete = bl_os_task_delete,
    ._task_get_current_task = bl_os_task_get_current_task,
    ._task_notify_create = bl_os_task_notify_create,
    ._task_notify = bl_os_task_notify,
    ._task_wait = bl_os_task_wait,
    ._irq_attach = bl_os_irq_attach,
    ._irq_enable = bl_os_irq_enable,
    ._irq_disable = bl_os_irq_disable,
    ._workqueue_create = bl_os_workqueue_create,
    ._workqueue_submit_hp = bl_os_workqueue_submit_hpwork,
    ._workqueue_submit_lp = bl_os_workqueue_submit_lpwork,
    ._timer_create = bl_os_timer_create,
    ._timer_delete = bl_os_timer_delete,
    ._timer_start_once = bl_os_timer_start_once,
    ._timer_start_periodic = bl_os_timer_start_periodic,
    ._sem_create = bl_os_sem_create,
    ._sem_delete = bl_os_sem_delete,
    ._sem_take = bl_os_sem_take,
    ._sem_give = bl_os_sem_give,
    ._mutex_create = bl_os_mutex_create,
    ._mutex_delete = bl_os_mutex_delete,
    ._mutex_lock = bl_os_mutex_lock,
    ._mutex_unlock = bl_os_mutex_unlock,
    ._queue_create = bl_os_mq_creat,
    ._queue_delete = bl_os_mq_delete,
    ._queue_send_wait = bl_os_mq_send_wait,
    ._queue_send = bl_os_mq_send,
    ._queue_recv = bl_os_mq_recv,
    ._malloc = bl_os_malloc,
    ._free = bl_os_free,
    ._zalloc = bl_os_zalloc,
    ._get_time_ms = bl_os_clock_gettime_ms,
    ._get_tick = bl_os_get_tick,
    ._log_write = bl_os_log_write,
    ._task_notify_isr = bl_os_task_notify_isr,
    ._yield_from_isr = bl_os_yield_from_isr,
    ._ms_to_tick = bl_os_ms_to_tick,
    ._set_timeout = bl_os_set_timeout,
    ._check_timeout = bl_os_check_timeout
};

extern void vprint(const char *fmt, va_list argp);
//extern volatile bool sys_log_all_enable;
volatile bool sys_log_all_enable;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bl_os_assert_func
 *
 * Description:
 *   Validation program developer's expected result
 *
 * Input Parameters:
 *   file  - configASSERT file
 *   line  - configASSERT line
 *   func  - configASSERT function
 *   expr  - configASSERT condition
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void bl_os_assert_func(const char *file, int line, const char *func, const char *expr)
{
    //printf("Assert failed in %s, %s:%d (%s)", func, file, line, expr);
    LOG_F("Assert failed in %s, %s:%d (%s)", func, file, line, expr);

    configASSERT(0);
}

/****************************************************************************
 * Name: bl_os_event_create
 *
 * Description:
 *   Create event group
 *
 * Input Parameters:
 *
 * Returned Value:
 *   Event group data pointer
 *
 ****************************************************************************/

BL_EventGroup_t bl_os_event_create(void)
{
    LOG_D("enter %s\r\n", __func__);
    struct EventGroupDef_t *event;

    event = xEventGroupCreate();

    if (event == NULL) {
        configASSERT(0);
    }

    LOG_D("exit %s\r\n", __func__);
    return event;
}

/****************************************************************************
 * Name: bl_os_event_delete
 *
 * Description:
 *   Delete event and free resource
 *
 * Input Parameters:
 *   event  - event data point
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void bl_os_event_delete(BL_EventGroup_t event)
{
    LOG_D("enter %s\r\n", __func__);
    vEventGroupDelete((EventGroupHandle_t)event);
    LOG_D("exit %s\r\n", __func__);
}

/****************************************************************************
 * Name: bl_os_event_send
 *
 * Description:
 *   Set event bits
 *
 * Input Parameters:
 *
 * Returned Value:
 *   Event value after setting
 *
 ****************************************************************************/

uint32_t bl_os_event_send(BL_EventGroup_t event, uint32_t bits)
{
    LOG_D("enter %s\r\n", __func__);
    EventGroupHandle_t p_event = (EventGroupHandle_t)event;
    BaseType_t xHigherPriorityTaskWoken;
    BaseType_t xResult;

    /* Being interrupted */

    if (xPortIsInsideInterrupt()) {
        xResult = xEventGroupSetBitsFromISR(p_event, bits, &xHigherPriorityTaskWoken);
    } else {
        xResult = xEventGroupSetBits(p_event, bits);
    }

    LOG_D("exit %s\r\n", __func__);
    return xResult;
}

/****************************************************************************
 * Name: bl_os_event_wait
 *
 * Description:
 *   Delete timer and free resource
 *
 * Input Parameters:
 *   event
 *   bits_to_wait_for
 *   clear_on_exit
 *   wait_for_all_bits
 *   block_time_tick
 *
 * Returned Value:
 *   Current event value
 *
 ****************************************************************************/
uint32_t bl_os_event_wait(BL_EventGroup_t event, uint32_t bits_to_wait_for, int clear_on_exit,
                          int wait_for_all_bits, uint32_t block_time_tick)
{
    LOG_D("enter %s\r\n", __func__);
    EventGroupHandle_t p_event = (EventGroupHandle_t)event;

    return xEventGroupWaitBits((EventGroupHandle_t)p_event,
                               bits_to_wait_for,
                               (clear_on_exit != BL_OS_FALSE ? pdTRUE : pdFALSE),
                               (wait_for_all_bits != BL_OS_FALSE ? pdTRUE : pdFALSE),
                               (block_time_tick == BL_OS_WAITING_FOREVER ? portMAX_DELAY : block_time_tick));
}

/****************************************************************************
 * Name: bl_os_event_register
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_event_register(int type, void *cb, void *arg)
{
    return 0;
}

/****************************************************************************
 * Name: bl_os_event_notify
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_event_notify(int evt, int val)
{
    /*return aos_post_event(EV_WIFI, evt, val);*/

    LOG_I("WIFI EVENT: %d %d\r\n", evt, val);
    return 0;
}

/****************************************************************************
 * Name: bl_os_task_create
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_task_create(const char *name,
                      void *entry,
                      uint32_t stack_depth,
                      void *param,
                      uint32_t prio,
                      BL_TaskHandle_t task_handle)
{
    LOG_D("enter %s\r\n", __func__);
    char modified_task_name[64] = {0};
    snprintf(modified_task_name, sizeof(modified_task_name), "wifi_%s", name);

    return xTaskCreate((TaskFunction_t)entry,
                       modified_task_name,
                       (stack_depth >> 2),
                       param,
                       prio,
                       (TaskHandle_t *)&task_handle);
}

/****************************************************************************
 * Name: bl_os_task_delete
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_task_delete(BL_TaskHandle_t task_handle)
{
    LOG_D("enter %s\r\n", __func__);
    vTaskDelete((TaskHandle_t)task_handle);
    LOG_D("exit %s\r\n", __func__);
}

/****************************************************************************
 * Name: bl_os_task_get_current_task
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

BL_TaskHandle_t bl_os_task_get_current_task(void)
{
    LOG_D("enter %s\r\n", __func__);
    return xTaskGetCurrentTaskHandle();
}

/****************************************************************************
 * Name: bl_os_task_notify_create
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

BL_TaskHandle_t bl_os_task_notify_create(void)
{
    LOG_D("enter %s\r\n", __func__);
    return xTaskGetCurrentTaskHandle();
}

/****************************************************************************
 * Name: bl_os_task_notify
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_task_notify(BL_TaskHandle_t task_handle)
{
    LOG_D("enter %s\r\n", __func__);
    BaseType_t xHigherPriorityTaskWoken;

    configASSERT(task_handle);

    if (xPortIsInsideInterrupt()) {
        vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
        xTaskNotifyGive(task_handle);
    }
    LOG_D("exit %s\r\n", __func__);
}

/****************************************************************************
 * Name: bl_os_task_wait
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_task_wait(BL_TaskHandle_t task_handle, uint32_t tick)
{
    LOG_D("enter %s\r\n", __func__);
    ulTaskNotifyTake(pdTRUE, tick);
    LOG_D("exit %s\r\n", __func__);
}

/****************************************************************************
 * Name: bl_os_api_init
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_api_init(void)
{
    return pdTRUE;
}

/****************************************************************************
 * Name: bl_os_lock_gaint
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_lock_gaint(void)
{
}

/****************************************************************************
 * Name: bl_os_unlock_gaint
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_unlock_gaint(void)
{
}

/****************************************************************************
 * Name: bl_os_enter_critical
 *
 * Description:
 *   Enter critical state
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   CPU PS value
 *
 ****************************************************************************/

uint32_t bl_os_enter_critical(void)
{
    LOG_D("enter %s\r\n", __func__);
    portENTER_CRITICAL();
    LOG_D("exit %s\r\n", __func__);

    return 0;
}

/****************************************************************************
 * Name: bl_os_exit_critical
 *
 * Description:
 *   Exit from critical state
 *
 * Input Parameters:
 *   level - CPU PS value
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void bl_os_exit_critical(uint32_t level)
{
    LOG_D("enter %s\r\n", __func__);
    portEXIT_CRITICAL();
    LOG_D("exit %s\r\n", __func__);
}

/****************************************************************************
 * Name: bl_os_msleep
 *
 * Description:
 *   Sleep in milliseconds
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_msleep(long msec)
{
    LOG_D("enter %s\r\n", __func__);
    const TickType_t ms = msec / portTICK_RATE_MS;

    vTaskDelay(ms);

    LOG_D("exit %s\r\n", __func__);

    return pdTRUE;
}

/****************************************************************************
 * Name: bl_os_sleep
 *
 * Description:
 *   Sleep in seconds
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_sleep(unsigned int seconds)
{
    LOG_D("enter %s\r\n", __func__);
    const TickType_t ms = seconds * 1000 / portTICK_RATE_MS;

    vTaskDelay(ms);

    LOG_D("exit %s\r\n", __func__);
    return pdTRUE;
}

/****************************************************************************
 * Name: bl_os_printf
 *
 * Description:
 *   Output format string and its arguments
 *
 * Input Parameters:
 *   format - format string
 *
 * Returned Value:
 *   0
 *
 ****************************************************************************/

void bl_os_printf(const char *__fmt, ...)
{
    LOG_D("enter %s\r\n", __func__);

    char formattedstring[256] = {0};
    va_list list;
    va_start(list, __fmt);
    vsnprintf(formattedstring, sizeof(formattedstring), __fmt, list);
    va_end(list);

    LOG_I("printf: %s", formattedstring);
}

/****************************************************************************
 * Name: bl_os_puts
 *
 * Description:
 *   Output format string
 *
 * Input Parameters:
 *   s - string
 *
 * Returned Value:
 *   0
 *
 ****************************************************************************/

void bl_os_puts(const char *s)
{
    LOG_D("enter %s\r\n", __func__);
    if (s != NULL) {
        LOG_I("%s", s);
    }
}

/****************************************************************************
 * Name: bl_os_malloc
 *
 * Description:
 *   Allocate a block of memory
 *
 * Input Parameters:
 *   size - memory size
 *
 * Returned Value:
 *   Memory pointer
 *
 ****************************************************************************/

void *bl_os_malloc(unsigned int size)
{
    void *ret;
    LOG_D("enter %s, %u\r\n", __func__, size);
    ret = pvPortMalloc(size);
    if (size == 14) {
        uint8_t *aa = (uint8_t *)ret;
        aa[13] = 0xDE;
        aa[0] = 0xAA;
    }
    LOG_D("Exit %s, %p\r\n", __func__, ret);
    return ret;

    //return pvPortMalloc(size);
}

/****************************************************************************
 * Name: bl_os_free
 *
 * Description:
 *   Free a block of memory
 *
 * Input Parameters:
 *   ptr - memory block
 *
 * Returned Value:
 *   No
 *
 ****************************************************************************/

void bl_os_free(void *ptr)
{
    LOG_D("enter %s\r\n", __func__);
    vPortFree(ptr);
}

/****************************************************************************
 * Name: bl_os_zalloc
 *
 * Description:
 *   Allocate a block of memory
 *
 * Input Parameters:
 *   size - memory size
 *
 * Returned Value:
 *   Memory pointer
 *
 ****************************************************************************/

void *bl_os_zalloc(unsigned int size)
{
    LOG_D("enter %s\r\n", __func__);
    uint8_t *ret = bl_os_malloc(size);
    if (ret != NULL) memset(ret, 0x00, size);
    //return pvPortCalloc(1, size);

    return (void *)ret;
}

/****************************************************************************
 * Name: bl_os_update_time
 *
 * Description:
 *   Transform ticks to time and add this time to timespec value
 *
 * Input Parameters:
 *   timespec - Input timespec data pointer
 *   ticks    - System ticks
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

static void bl_os_update_time(uint32_t *ms, uint32_t ticks)
{

}

/****************************************************************************
 * Name: bl_os_errno_trans
 *
 * Description:
 *   Transform from nuttx Os error code to Wi-Fi adapter error code
 *
 * Input Parameters:
 *   ret - NuttX error code
 *
 * Returned Value:
 *   Wi-Fi adapter error code
 *
 ****************************************************************************/

static inline int32_t bl_os_errno_trans(int ret)
{
    if (ret == pdTRUE) {
        return 0;
    } else {
        return 1;
    }
}

/****************************************************************************
 * Name: bl_os_mq_creat
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

BL_MessageQueue_t bl_os_mq_creat(uint32_t queue_len, uint32_t item_size)
{
    LOG_D("enter %s\r\n", __func__);
    return xQueueCreate(queue_len, item_size);
}

/****************************************************************************
 * Name: bl_os_mq_delete
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_mq_delete(BL_MessageQueue_t mq)
{
    LOG_D("enter %s\r\n", __func__);
    vQueueDelete((QueueHandle_t)mq);
}

/****************************************************************************
 * Name: bl_os_mq_send_wait
 *
 * Description:
 *   Generic send message to queue within a certain period of time
 *
 * Input Parameters:
 *   queue - Message queue data pointer
 *   item  - Message data pointer
 *   ticks - Wait ticks
 *   prio  - Message priority
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int bl_os_mq_send_wait(BL_MessageQueue_t queue, void *item, uint32_t len, uint32_t ticks, int prio)
{
    LOG_D("enter %s\r\n", __func__);
    int ret;
    QueueHandle_t mq = (QueueHandle_t)queue;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Wi-Fi interrupt function will call this adapter function to send
       * message to message queue, so here we should call kernel API
       * instead of application API
       */

    if (xPortIsInsideInterrupt()) {
        ret = xQueueSendFromISR(mq, item, &xHigherPriorityTaskWoken);
    } else {
        ret = xQueueSend(mq, item, ticks);
    }

    LOG_D("exit %s\r\n", __func__);

    return bl_os_errno_trans(ret);
}

/****************************************************************************
 * Name: bl_os_mq_send
 *
 * Description:
 *   Send message of low priority to queue within a certain period of time
 *
 * Input Parameters:
 *   queue - Message queue data pointer
 *   item  - Message data pointer
 *   ticks - Wait ticks
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int bl_os_mq_send(BL_MessageQueue_t queue, void *item, uint32_t len)
{
    LOG_D("enter %s\r\n", __func__);
    return bl_os_mq_send_wait(queue, item, len, 0, 0);
}

/****************************************************************************
 * Name: bl_os_mq_recv
 *
 * Description:
 *   Receive message from queue within a certain period of time
 *
 * Input Parameters:
 *   queue - Message queue data pointer
 *   item  - Message data pointer
 *   ticks - Wait ticks
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int bl_os_mq_recv(BL_MessageQueue_t queue, void *item, uint32_t len, uint32_t tick)
{
    LOG_D("enter %s\r\n", __func__);
    int ret;
    QueueHandle_t mq = (QueueHandle_t)queue;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Wi-Fi interrupt function will call this adapter function to send
       * message to message queue, so here we should call kernel API
       * instead of application API
       */

    if (xPortIsInsideInterrupt()) {
        ret = xQueueReceiveFromISR(mq, item, &xHigherPriorityTaskWoken);
    } else {
        ret = xQueueReceive(mq, item, tick);
    }

    return bl_os_errno_trans(ret);
}

/****************************************************************************
 * Name: bl_os_timer_callback
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

static void bl_os_timer_callback(int arg)
{
    LOG_D("enter %s\r\n", __func__);
    TimerHandle_t handle;
    struct timer_adpt *timer;

    handle = (TimerHandle_t)arg;
    timer = (struct timer_adpt *)pvTimerGetTimerID(handle);

    if (timer != NULL && timer->func) {
        timer->func(timer->arg);
    }
}

/****************************************************************************
 * Name: bl_os_timer_create
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

BL_Timer_t bl_os_timer_create(void *func, void *argv)
{
    LOG_D("enter %s\r\n", __func__);
    struct timer_adpt *timer;

    //timer = (struct timer_adpt *)pvPortCalloc(1, sizeof(struct timer_adpt));
    timer = (struct timer_adpt *)bl_os_zalloc(sizeof(struct timer_adpt));

    if (!timer) {
        configASSERT(0);
    }

    timer->handle = xTimerCreate(NULL, 1000, pdFALSE, (void *)timer, bl_os_timer_callback);

    if (timer->handle == NULL) {
        configASSERT(0);
    }

    timer->func = (void (*)(void *arg))func;
    timer->arg = argv;

    LOG_D("exit %s\r\n", __func__);
    return timer;
}

/****************************************************************************
 * Name: bl_os_timer_delete
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_timer_delete(BL_Timer_t timerid, uint32_t tick)
{
    LOG_D("enter %s\r\n", __func__);
    struct timer_adpt *timer;

    timer = (struct timer_adpt *)timerid;

    if (!timer) {
        return pdFALSE;
    }

    xTimerStop(timer->handle, tick);

    xTimerDelete(timer->handle, tick);

    vPortFree(timer);

    LOG_D("exit %s\r\n", __func__);
    return pdTRUE;
}

/****************************************************************************
 * Name: bl_os_timer_start_once
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_timer_start_once(BL_Timer_t timerid, long t_sec, long t_nsec)
{
    LOG_D("enter %s\r\n", __func__);
    struct timer_adpt *timer;
    int32_t tick;
    int ret;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    timer = (struct timer_adpt *)timerid;

    if (!timer) {
        return pdFALSE;
    }

    tick = pdMS_TO_TICKS((t_sec * 1000) + ((t_nsec > 1e6) ? (t_nsec / 1e6) : 0));

    timer->mode = BL_OS_TIEMR_ONCE;
    timer->delay = tick;

    vTimerSetReloadMode(timer->handle, pdFALSE);

    if (xPortIsInsideInterrupt()) {
        ret = xTimerChangePeriodFromISR(timer->handle, tick, &xHigherPriorityTaskWoken);
    }
    else {
        ret = xTimerChangePeriod(timer->handle, tick, portMAX_DELAY);

    }

    LOG_D("exit %s\r\n", __func__);
    return bl_os_errno_trans(ret);
}

/****************************************************************************
 * Name: bl_os_timer_start_periodic
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_timer_start_periodic(BL_Timer_t timerid, long t_sec, long t_nsec)
{
    LOG_D("enter %s\r\n", __func__);
    struct timer_adpt *timer;
    int32_t tick;
    int ret;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    timer = (struct timer_adpt *)timerid;

    if (!timer) {
        return pdFALSE;
    }

    tick = pdMS_TO_TICKS((t_sec * 1000) + ((t_nsec > 1e6) ? (t_nsec / 1e6) : 0));

    timer->mode = BL_OS_TIEMR_CYCLE;
    timer->delay = tick;

    vTimerSetReloadMode(timer->handle, pdTRUE);

    if (xPortIsInsideInterrupt()) {
        ret = xTimerChangePeriodFromISR(timer->handle, tick, &xHigherPriorityTaskWoken);
    }
    else {
        ret = xTimerChangePeriod(timer->handle, tick, portMAX_DELAY);

    }

    LOG_D("exit %s\r\n", __func__);
    return bl_os_errno_trans(ret);
}

/****************************************************************************
 * Name: bl_os_workqueue_create
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void *bl_os_workqueue_create(void)
{
    LOG_D("enter %s\r\n", __func__);
    return xTaskGetCurrentTaskHandle();
}

/****************************************************************************
 * Name: bl_os_workqueue_submit_hpwork
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_workqueue_submit_hpwork(void *work, void *worker, void *argv, long tick)
{
    LOG_D("enter %s\r\n", __func__);
    bl_os_task_notify(work);

    LOG_D("exit %s\r\n", __func__);
    return 0;
}

/****************************************************************************
 * Name: bl_os_workqueue_submit_lpwork
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_workqueue_submit_lpwork(void *work, void *worker, void *argv, long tick)
{
    LOG_D("enter %s\r\n", __func__);
    bl_os_task_notify(work);

    LOG_D("exit %s\r\n", __func__);
    return 0;
}

/****************************************************************************
 * Name: bl_os_clock_gettime_ms
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

uint64_t bl_os_clock_gettime_ms(void)
{
    LOG_D("enter %s\r\n", __func__);
    long long ms = 0;
    TickType_t ticks = 0;
    BaseType_t overflow_count = 0;

    //xTaskGetTickCount2(&ticks, &overflow_count);
    //TODO: NO OVERFLOWS
    ticks = xTaskGetTickCount();

    ms = ((long long)ticks) + ((TickType_t)(-1) * ((long long)overflow_count));

    LOG_D("exit %s\r\n", __func__);
    return ms;
}

/****************************************************************************
 * Name: bl_os_get_tick
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

uint32_t bl_os_get_tick()
{
    LOG_D("enter %s\r\n", __func__);
    uint32_t tick;

    if (xPortIsInsideInterrupt()) {
        tick = xTaskGetTickCountFromISR();
    }
    else {
        tick = xTaskGetTickCount();
    }

    LOG_D("exit %s\r\n", __func__);
    return tick;
}

/****************************************************************************
 * Name: bl_os_isr_adpt_cb
 *
 * Description:
 *   Wi-Fi interrupt adapter callback function
 *
 * Input Parameters:
 *   arg - interrupt adapter private data
 *
 * Returned Value:
 *   0 on success
 *
 ****************************************************************************/

static int bl_os_isr_adpt_cb(int irq, void *context, void *arg)
{
    LOG_D("enter %s\r\n", __func__);
    struct irq_adpt *adapter = (struct irq_adpt *)arg;

    adapter->func(adapter->arg);

    LOG_D("exit %s\r\n", __func__);
    return 0;
}

/****************************************************************************
 * Name: bl_os_irq_attach
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_irq_attach(int32_t n, void *f, void *arg)
{
    //TODO: Make sure this works
    LOG_D("enter %s\r\n", __func__);
    LOG_I("bl_os_irq_attach: %u to %p\r\n", n, f);
    bflb_irq_attach(n, f, arg);
    LOG_D("exit %s\r\n", __func__);
}

/****************************************************************************
 * Name: bl_os_irq_enable
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_irq_enable(int32_t n)
{
    //TODO: Make sure this works
    LOG_D("enter %s\r\n", __func__);
    LOG_I("bl_os_irq_enable: %u\r\n", n);
    bflb_irq_enable(n);
    LOG_D("exit %s\r\n", __func__);
}

/****************************************************************************
 * Name: bl_os_irq_disable
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_irq_disable(int32_t n)
{
    //TODO: Make sure this works
    LOG_I("bl_os_irq_disable: %u\r\n", n);
    bflb_irq_disable(n);
}

/****************************************************************************
 * Name: bl_os_mutex_create
 *
 * Description:
 *   Create mutex
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   Mutex data pointer
 *
 ****************************************************************************/

BL_Mutex_t bl_os_mutex_create(void)
{
    LOG_D("enter %s\r\n", __func__);
    SemaphoreHandle_t xSemaphore;

    xSemaphore = xSemaphoreCreateMutex();
    configASSERT(xSemaphore);

    LOG_D("exit %s\r\n", __func__);
    return xSemaphore;
}

/****************************************************************************
 * Name: bl_os_mutex_delete
 *
 * Description:
 *   Delete mutex
 *
 * Input Parameters:
 *   mutex_data - mutex data pointer
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void bl_os_mutex_delete(BL_Mutex_t mutex_data)
{
    LOG_D("enter %s\r\n", __func__);
    SemaphoreHandle_t sem = (SemaphoreHandle_t)mutex_data;

    vSemaphoreDelete(sem);
    LOG_D("exit %s\r\n", __func__);
}

/****************************************************************************
 * Name: bl_os_mutex_lock
 *
 * Description:
 *   Lock mutex
 *
 * Input Parameters:
 *   mutex_data - mutex data pointer
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int32_t bl_os_mutex_lock(BL_Mutex_t mutex_data)
{
    LOG_D("enter %s\r\n", __func__);
    SemaphoreHandle_t sem = (SemaphoreHandle_t)mutex_data;
    BaseType_t xHigherPriorityTaskWoken;
    int ret = pdFALSE;

    if (sem != NULL) {
        if (xPortIsInsideInterrupt()) {
            ret = xSemaphoreTakeFromISR(sem, &xHigherPriorityTaskWoken);
        } else {
            ret = xSemaphoreTake(sem, portMAX_DELAY);
        }
    }

    LOG_D("exit %s\r\n", __func__);
    return bl_os_errno_trans(ret);
}

/****************************************************************************
 * Name: bl_os_mutex_unlock
 *
 * Description:
 *   Lock mutex
 *
 * Input Parameters:
 *   mutex_data - mutex data pointer
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int32_t bl_os_mutex_unlock(BL_Mutex_t mutex_data)
{
    LOG_D("enter %s\r\n", __func__);
    SemaphoreHandle_t sem = (SemaphoreHandle_t)mutex_data;
    BaseType_t xHigherPriorityTaskWoken;
    int ret = pdFALSE;

    if (sem != NULL) {
        if (xPortIsInsideInterrupt()) {
            ret = xSemaphoreGiveFromISR(sem, &xHigherPriorityTaskWoken);
        } else {
            ret = xSemaphoreGive(sem);
        }
    }

    LOG_D("exit %s\r\n", __func__);
    return bl_os_errno_trans(ret);
}

/****************************************************************************
 * Name: bl_os_sem_create
 *
 * Description:
 *   Create and initialize semaphore
 *
 * Input Parameters:
 *   max  - No mean
 *   init - semaphore initialization value
 *
 * Returned Value:
 *   Semaphore data pointer
 *
 ****************************************************************************/

BL_Sem_t bl_os_sem_create(uint32_t init)
{
    LOG_D("enter %s\r\n", __func__);
    SemaphoreHandle_t xSemaphore;

    xSemaphore = xSemaphoreCreateBinary();
    configASSERT(xSemaphore);

    LOG_D("exit %s\r\n", __func__);
    return xSemaphore;
}

/****************************************************************************
 * Name: bl_os_sem_delete
 *
 * Description:
 *   Delete semaphore
 *
 * Input Parameters:
 *   semphr - Semaphore data pointer
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void bl_os_sem_delete(BL_Sem_t semphr)
{
    LOG_D("enter %s\r\n", __func__);
    SemaphoreHandle_t sem = (SemaphoreHandle_t)semphr;

    vSemaphoreDelete(sem);
    LOG_D("exit %s\r\n", __func__);
}

/****************************************************************************
 * Name: bl_os_sem_take
 *
 * Description:
 *   Wait semaphore within a certain period of time
 *
 * Input Parameters:
 *   semphr - Semaphore data pointer
 *   ticks  - Wait system ticks
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int32_t bl_os_sem_take(BL_Sem_t semphr, uint32_t ticks)
{
    LOG_D("enter %s\r\n", __func__);
    SemaphoreHandle_t sem = (SemaphoreHandle_t)semphr;
    BaseType_t xHigherPriorityTaskWoken;
    int ret = pdFALSE;

    if (sem != NULL) {
        if (xPortIsInsideInterrupt()) {
            ret = xSemaphoreTakeFromISR(sem, &xHigherPriorityTaskWoken);
        } else {
            ret = xSemaphoreTake(sem, portMAX_DELAY);
        }
    }

    LOG_D("exit %s\r\n", __func__);
    return bl_os_errno_trans(ret);
}

/****************************************************************************
 * Name: bl_os_sem_give
 *
 * Description:
 *   Post semaphore
 *
 * Input Parameters:
 *   semphr - Semaphore data pointer
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int32_t bl_os_sem_give(BL_Sem_t semphr)
{
    LOG_D("enter %s\r\n", __func__);
    SemaphoreHandle_t sem = (SemaphoreHandle_t)semphr;
    BaseType_t xHigherPriorityTaskWoken;
    int ret = pdFALSE;

    if (sem != NULL) {
        if (xPortIsInsideInterrupt()) {
            ret = xSemaphoreGiveFromISR(sem, &xHigherPriorityTaskWoken);
        } else {
            ret = xSemaphoreGive(sem);
        }
    }

    LOG_D("exit %s\r\n", __func__);
    return bl_os_errno_trans(ret);
}

/****************************************************************************
 * Name: bl_os_log_writev
 *
 * Description:
 *   Output log with by format string and its arguments
 *
 * Input Parameters:
 *   level  - log level, no mean here
 *   tag    - log TAG, no mean here
 *   file   - file name
 *   line   - assert line
 *   format - format string
 *   args   - arguments list
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

/*char LogLevelChar[] = {'?', 'D', 'I', 'W', 'E', 'A', 'C'};

static void bl_os_log_writev(uint32_t level,
                             const char *tag,
                             const char *file,
                             int line,
                             const char *format,
                             va_list args)
{
    char newformat[256];
    sprintf(newformat, "[%c][%s][%s: %s:%4d] %s", LogLevelChar[level], DBG_TAG, tag, file, line, format);
    vprintf(newformat, args);
    //LOG_F
}*/

/****************************************************************************
 * Name: bl_os_log_write
 *
 * Description:
 *   Output log with by format string and its arguments
 *
 * Input Parameters:
 *   level  - log level, no mean here
 *   file   - file name
 *   line   - assert line
 *   tag    - log TAG, no mean here
 *   format - format string
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void bl_os_log_write(uint32_t level, const char *tag, const char *file, int line, const char *format, ...)
{
    LOG_D("enter %s\r\n", __func__);
    char formattedstring[256] = {0};
    va_list list;
    va_start(list, format);
    vsnprintf(formattedstring, sizeof(formattedstring), format, list);
    va_end(list);
    formattedstring[255] = 0x00;
    formattedstring[252] = '[';
    formattedstring[253] = 'O';
    formattedstring[254] = ']';

    switch (level) {
        case LOG_LEVEL_ERROR:
        {
            //bl_os_log_writev(LOG_LEVEL_ERROR, "\x1b[31mERROR \x1b[0m", file, line, format, list);
            LOG_E("[%s: %s:%4d] %s", "\x1b[31mERROR \x1b[0m", file, line, formattedstring);
            break;
        }
        case LOG_LEVEL_WARN:
        {
            //bl_os_log_writev(LOG_LEVEL_WARN, "\x1b[33mWARN  \x1b[0m", file, line, format, list);
            LOG_W("[%s: %s:%4d] %s", "\x1b[33mWARN  \x1b[0m", file, line, formattedstring);
            break;
        }
        case LOG_LEVEL_INFO:
        {
            //bl_os_log_writev(LOG_LEVEL_INFO, "\x1b[32mINFO  \x1b[0m", file, line, format, list);
            LOG_I("[%s: %s:%4d] %s", "\x1b[32mINFO  \x1b[0m", file, line, formattedstring);
            break;
        }
        case LOG_LEVEL_DEBUG:
        {
            //bl_os_log_writev(LOG_LEVEL_DEBUG, "DEBUG ", file, line, format, list);
            LOG_D("[%s: %s:%4d] %s", "DEBUG ", file, line, formattedstring);
            break;
        }
    }

    //va_end(list);
}

/****************************************************************************
 * Name: bl_os_task_notify_isr
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_task_notify_isr(BL_TaskHandle_t task_handle)
{
    LOG_D("enter %s\r\n", __func__);
    BaseType_t xHigherPriorityTaskWoken;

    configASSERT(task_handle);

    vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);

    LOG_D("exit %s\r\n", __func__);
    return xHigherPriorityTaskWoken;
}

/****************************************************************************
 * Name: bl_os_yield_from_isr
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_yield_from_isr(int xYield)
{
    LOG_D("enter %s\r\n", __func__);
    portYIELD_FROM_ISR(xYield);
    LOG_D("exit %s\r\n", __func__);
}

/****************************************************************************
 * Name: bl_os_ms_to_tick
 *
 * Description:
 *  
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

unsigned int bl_os_ms_to_tick(unsigned int ms)
{
    LOG_D("enter %s\r\n", __func__);
    return pdMS_TO_TICKS(ms);
}

/****************************************************************************
 * Name: bl_os_set_timeout
 *
 * Description:
 *  
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

BL_TimeOut_t bl_os_set_timeout(void)
{
    LOG_D("enter %s\r\n", __func__);
    TimeOut_t *xTimeOut = NULL;

    xTimeOut = (TimeOut_t *)bl_os_malloc(sizeof(TimeOut_t));

    vTaskSetTimeOutState(xTimeOut);

    LOG_D("exit %s\r\n", __func__);
    return (BL_TimeOut_t)xTimeOut;
}

/****************************************************************************
 * Name: bl_os_check_timeout
 *
 * Description:
 *  
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_check_timeout(BL_TimeOut_t xTimeOut, BL_TickType_t *xTicksToWait)
{
    LOG_D("enter %s\r\n", __func__);
    return xTaskCheckForTimeOut((TimeOut_t *)xTimeOut, (TickType_t *)xTicksToWait);
}

