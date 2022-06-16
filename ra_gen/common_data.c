/* generated common source file - do not edit */
#include "common_data.h"
ioport_instance_ctrl_t g_ioport_ctrl;
const ioport_instance_t g_ioport =
{ .p_api = &g_ioport_on_ioport, .p_ctrl = &g_ioport_ctrl, .p_cfg = &g_bsp_pin_cfg, };
SemaphoreHandle_t rl78i1c_tx_complete_semaphore;
#if 1
StaticSemaphore_t rl78i1c_tx_complete_semaphore_memory;
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
StreamBufferHandle_t rl78i1c_uart_sb;
#if 1
StaticStreamBuffer_t rl78i1c_uart_sb_memory;
uint8_t rl78i1c_uart_sb_buffer_memory[256 + 1];
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
SemaphoreHandle_t g_usb_write_complete_binary_semaphore;
#if 1
StaticSemaphore_t g_usb_write_complete_binary_semaphore_memory;
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
QueueHandle_t g_usb_read_queue;
#if 1
StaticQueue_t g_usb_read_queue_memory;
uint8_t g_usb_read_queue_queue_memory[4 * 20];
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
QueueHandle_t g_usb_event_queue;
#if 1
StaticQueue_t g_usb_event_queue_memory;
uint8_t g_usb_event_queue_queue_memory[8 * 20];
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
void g_common_init(void)
{
    rl78i1c_tx_complete_semaphore =
#if 1
            xSemaphoreCreateBinaryStatic (&rl78i1c_tx_complete_semaphore_memory);
#else
                xSemaphoreCreateBinary();
                #endif
    if (NULL == rl78i1c_tx_complete_semaphore)
    {
        rtos_startup_err_callback (rl78i1c_tx_complete_semaphore, 0);
    }
    rl78i1c_uart_sb =
#if 1
            xStreamBufferCreateStatic (
#else
                xStreamBufferCreate(
                #endif
                                       256,
                                       1
#if 1
                                       ,
                                       &rl78i1c_uart_sb_buffer_memory[0], &rl78i1c_uart_sb_memory
#endif
                                       );
    if (NULL == rl78i1c_uart_sb)
    {
        rtos_startup_err_callback (rl78i1c_uart_sb, 0);
    }
    g_usb_write_complete_binary_semaphore =
#if 1
            xSemaphoreCreateBinaryStatic (&g_usb_write_complete_binary_semaphore_memory);
#else
                xSemaphoreCreateBinary();
                #endif
    if (NULL == g_usb_write_complete_binary_semaphore)
    {
        rtos_startup_err_callback (g_usb_write_complete_binary_semaphore, 0);
    }
    g_usb_read_queue =
#if 1
            xQueueCreateStatic (
#else
                xQueueCreate(
                #endif
                                20,
                                4
#if 1
                                ,
                                &g_usb_read_queue_queue_memory[0], &g_usb_read_queue_memory
#endif
                                );
    if (NULL == g_usb_read_queue)
    {
        rtos_startup_err_callback (g_usb_read_queue, 0);
    }
    g_usb_event_queue =
#if 1
            xQueueCreateStatic (
#else
                xQueueCreate(
                #endif
                                20,
                                8
#if 1
                                ,
                                &g_usb_event_queue_queue_memory[0], &g_usb_event_queue_memory
#endif
                                );
    if (NULL == g_usb_event_queue)
    {
        rtos_startup_err_callback (g_usb_event_queue, 0);
    }
}
