#include "usb_thread.h"
#include "rl78i1c_thread.h"
#include "usb_cdc.h"

/* 115200 8n1 by default */
static usb_pcdc_linecoding_t g_line_coding = {
    .dw_dte_rate    = 115200,
    .b_char_format = STOP_BITS_1,
    .b_parity_type = PARITY_NONE,
    .b_data_bits   = DATA_BITS_8,
};

static volatile usb_pcdc_ctrllinestate_t g_control_line_state = {
    .bdtr = 0,
    .brts = 0,
};

/* USB Thread entry function */
/* pvParameters contains TaskHandle_t */
void usb_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED (pvParameters);
    usb_setup_t      setup;
    BaseType_t       status;
    fsp_err_t        err;

    while (1)
    {
        status = xQueueReceive(g_usb_event_queue, &setup, portMAX_DELAY);
        if (pdPASS == status)
        {
            if (USB_PCDC_SET_LINE_CODING == (setup.request_type & USB_BREQUEST))
            {
                R_USB_PeriControlDataGet(&g_basic0_ctrl, (uint8_t *) &g_line_coding, LINE_CODING_LENGTH);
            }
            else if (USB_PCDC_GET_LINE_CODING == (setup.request_type & USB_BREQUEST))
            {
                R_USB_PeriControlDataSet(&g_basic0_ctrl, (uint8_t *) &g_line_coding, LINE_CODING_LENGTH);
            }
            else if (USB_PCDC_SET_CONTROL_LINE_STATE == (setup.request_type & USB_BREQUEST))
            {
                err = R_USB_PeriControlStatusSet(&g_basic0_ctrl, USB_SETUP_STATUS_ACK);
                if (FSP_SUCCESS == err)
                {
                    g_control_line_state.bdtr = (unsigned char)((setup.request_value >> 0) & 0x01);
                    g_control_line_state.brts = (unsigned char)((setup.request_value >> 1) & 0x01);
                }

            }
            else
            {
                /* ACK all other status requests */
                err = R_USB_PeriControlStatusSet(&g_basic0_ctrl, USB_SETUP_STATUS_ACK);
                if (FSP_SUCCESS != err)
                {
                    __NOP();
                }
            }
        }
    }
}

void usb_cdc_rtos_callback(usb_event_info_t * event, usb_hdl_t handle, usb_onoff_t onoff)
{
    FSP_PARAMETER_NOT_USED(handle);
    FSP_PARAMETER_NOT_USED(onoff);

    switch (event->event)
    {
        case USB_STATUS_CONFIGURED :
            break;
        case USB_STATUS_WRITE_COMPLETE :
            if (pdTRUE == xSemaphoreGiveFromISR(g_usb_write_complete_binary_semaphore, NULL))
            {
                __NOP();
            }
            break;
        case USB_STATUS_READ_COMPLETE :
            if (pdTRUE == xQueueSendFromISR(g_usb_read_queue, &event->data_size, NULL ) )
            {
                __NOP();
            }

            break;
        case USB_STATUS_REQUEST : /* Receive Class Request */
        if (pdTRUE == xQueueSendFromISR(g_usb_event_queue, &event->setup, NULL ) )
        {
            __NOP();
        }
        break;
        case USB_STATUS_REQUEST_COMPLETE :
            __NOP();
            break;
        case USB_STATUS_SUSPEND :
        case USB_STATUS_DETACH :
        case USB_STATUS_DEFAULT :
            __NOP();
            break;
        default :
            __NOP();
            break;
    }
}

fsp_err_t get_control_line_state(usb_pcdc_ctrllinestate_t *ptr)
{
    FSP_PARAMETER_NOT_USED(ptr);
    *ptr = (usb_pcdc_ctrllinestate_t)g_control_line_state;
    return FSP_SUCCESS;
}
