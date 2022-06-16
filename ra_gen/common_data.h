/* generated common header file - do not edit */
#ifndef COMMON_DATA_H_
#define COMMON_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "r_ioport.h"
#include "bsp_pin_cfg.h"
FSP_HEADER
/* IOPORT Instance */
extern const ioport_instance_t g_ioport;

/* IOPORT control structure. */
extern ioport_instance_ctrl_t g_ioport_ctrl;
extern SemaphoreHandle_t rl78i1c_tx_complete_semaphore;
extern StreamBufferHandle_t rl78i1c_uart_sb;
extern SemaphoreHandle_t g_usb_write_complete_binary_semaphore;
extern QueueHandle_t g_usb_read_queue;
extern QueueHandle_t g_usb_event_queue;
void g_common_init(void);
FSP_FOOTER
#endif /* COMMON_DATA_H_ */
