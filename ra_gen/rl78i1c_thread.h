/* generated thread header file - do not edit */
#ifndef RL78I1C_THREAD_H_
#define RL78I1C_THREAD_H_
#include "bsp_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hal_data.h"
#ifdef __cplusplus
                extern "C" void rl78i1c_thread_entry(void * pvParameters);
                #else
extern void rl78i1c_thread_entry(void *pvParameters);
#endif
#include "r_usb_basic.h"
#include "r_usb_basic_api.h"
#include "r_usb_pcdc_api.h"
#include "r_dtc.h"
#include "r_transfer_api.h"
#include "r_sci_uart.h"
#include "r_uart_api.h"
#include "r_gpt.h"
#include "r_timer_api.h"
FSP_HEADER
/* Basic on USB Instance. */
extern const usb_instance_t g_basic0;

/** Access the USB instance using these structures when calling API functions directly (::p_api is not used). */
extern usb_instance_ctrl_t g_basic0_ctrl;
extern const usb_cfg_t g_basic0_cfg;

#ifndef NULL
void NULL(void*);
#endif

#if 2 == BSP_CFG_RTOS
#ifndef usb_cdc_rtos_callback
void usb_cdc_rtos_callback(usb_event_info_t *, usb_hdl_t, usb_onoff_t);
#endif
#endif
/** CDC Driver on USB Instance. */
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer0;

/** Access the DTC instance using these structures when calling API functions directly (::p_api is not used). */
extern dtc_instance_ctrl_t g_transfer0_ctrl;
extern const transfer_cfg_t g_transfer0_cfg;
/** UART on SCI Instance. */
extern const uart_instance_t rl78i1c_uart;

/** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
extern sci_uart_instance_ctrl_t rl78i1c_uart_ctrl;
extern const uart_cfg_t rl78i1c_uart_cfg;
extern const sci_uart_extended_cfg_t rl78i1c_uart_cfg_extend;

#ifndef rl78i1c_uart_callback
void rl78i1c_uart_callback(uart_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t rl78i1c_psu_driver;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t rl78i1c_psu_driver_ctrl;
extern const timer_cfg_t rl78i1c_psu_driver_cfg;

#ifndef NULL
void NULL(timer_callback_args_t *p_args);
#endif
FSP_FOOTER
#endif /* RL78I1C_THREAD_H_ */
