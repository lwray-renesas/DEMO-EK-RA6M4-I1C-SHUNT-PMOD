/* generated thread header file - do not edit */
#ifndef USB_THREAD_H_
#define USB_THREAD_H_
#include "bsp_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hal_data.h"
#ifdef __cplusplus
                extern "C" void usb_thread_entry(void * pvParameters);
                #else
extern void usb_thread_entry(void *pvParameters);
#endif
FSP_HEADER
FSP_FOOTER
#endif /* USB_THREAD_H_ */
