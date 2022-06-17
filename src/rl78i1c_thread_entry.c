#include "gpt_pwm.h"
#include "rl78i1c_parser.h"
#include "rl78i1c_thread.h"
#include "stdio.h"

/** @brief maximum raw buffer size - 3k*/
#define MAX_RAW_BUF_SIZE  3072U

/** @brief CMD_PROMPT string*/
static char const CMD_PROMPT[] = "CMD>";
/** @brief DISPLAY_COMMAND string*/
static char const DISPLAY_COMMAND[] = "display\r";
/** @brief raw character buffer from the rl78i1c UART*/
static char rl78i1c_raw_msg_buffer[MAX_RAW_BUF_SIZE] = {0};
/** @brief raw character buffer fill level*/
static uint32_t bytes_in_raw_buffer = 0U;


/** @brief Wait indefinitely for the CMD_PROMPT message to be received.*/
static void Wait_for_cmd(void);
/** @brief Sends display command.*/
static void Send_display(void);
/** @brief Writes rl78/i1c data to terminal (uses clear screen ansi symbol so must use terminal with ansi escape sequence support)*/
static void Write_rl78i1c_msg_to_usb(rl78_i1c_message_t const * msg);


/** @brief RL78/I1C Thread entry function */
void rl78i1c_thread_entry(void *pvParameters)
{
    fsp_err_t  err; /* return variable used to check for errors*/
    FSP_PARAMETER_NOT_USED (pvParameters);

    /* Open the UART driver to communicate with the RL78/I1C*/
    err = R_SCI_UART_Open(&rl78i1c_uart_ctrl, &rl78i1c_uart_cfg);
    assert(FSP_SUCCESS == err);

    /* Start the pwm to power the RL78/I1C*/
    pwm_start(&rl78i1c_psu_driver_ctrl, &rl78i1c_psu_driver_cfg);

    /* Startup USB for streaming out rl78i1c data*/
    err = R_USB_Open (&g_basic0_ctrl, &g_basic0_cfg);
    assert(FSP_SUCCESS == err);

    /* Wait indefinitely for the CMD_PROMPT string - first time is usless*/
    Wait_for_cmd();

    while (1)
    {
        rl78_i1c_message_t const * parsed_msg;

        /* Send the display command again*/
        Send_display();

        /* Wait indefinitely for the CMD_PROMPT string - this indicates data is ready for parsing*/
        Wait_for_cmd();

        /* Try parse the i1c data*/
        parsed_msg = Parser(rl78i1c_raw_msg_buffer, bytes_in_raw_buffer);

        /* Send out the data to USB*/
        Write_rl78i1c_msg_to_usb(parsed_msg);
    }
}
/* END OF FUNCTION*/


static void Wait_for_cmd(void)
{
    char * p_buf = rl78i1c_raw_msg_buffer;

    /* Reset the byte counter before waiting for the cmd prompt*/
    bytes_in_raw_buffer = 0U;

    /* Read until there are AT LEAST the correct number of bytes in the buffer to contain the CMD_PROMPT*/
    while(bytes_in_raw_buffer < (sizeof(CMD_PROMPT)-1U))
    {
        bytes_in_raw_buffer += xStreamBufferReceive(rl78i1c_uart_sb, &rl78i1c_raw_msg_buffer[bytes_in_raw_buffer], sizeof(rl78i1c_raw_msg_buffer-bytes_in_raw_buffer), ( TickType_t ) portMAX_DELAY);
    }

    /* Scan the buffer until CMD_PROMPT is received*/
    while(0 != memcmp(CMD_PROMPT, p_buf, (sizeof(CMD_PROMPT)-1U)))
    {
        /* If the scan pointer still has room to search, then increment it*/
        if((p_buf+(sizeof(CMD_PROMPT)-1U)) <= &rl78i1c_raw_msg_buffer[bytes_in_raw_buffer])
        {
            ++p_buf;
        }
        else
        {
            /* Try read more data from the UART*/
            bytes_in_raw_buffer += xStreamBufferReceive(rl78i1c_uart_sb, &rl78i1c_raw_msg_buffer[bytes_in_raw_buffer], sizeof(rl78i1c_raw_msg_buffer-bytes_in_raw_buffer), 1);
        }
    }

    /* Remove the CMD_PROMPT from the buffer index*/
    bytes_in_raw_buffer = (uint32_t)(p_buf - rl78i1c_raw_msg_buffer);
}
/*END OF FUNCTION*/

static void Send_display(void)
{
    static char const * end_of_display_cmd = DISPLAY_COMMAND + (sizeof(DISPLAY_COMMAND)-1U);

    /* Loop through the charactors of the display command with a 50ms pause between them*/
    for (char const * p = DISPLAY_COMMAND; p < end_of_display_cmd;  ++p)
    {
        fsp_err_t err = R_SCI_UART_Write(&rl78i1c_uart_ctrl, (uint8_t *)p, 1);
        assert(FSP_SUCCESS == err);

        BaseType_t status = xSemaphoreTake(rl78i1c_tx_complete_semaphore, ( TickType_t ) portMAX_DELAY );
        assert(pdTRUE == status);

        vTaskDelay (50 / portTICK_PERIOD_MS); // Delay
    }
}
/*END OF FUNCTION*/

static void Write_rl78i1c_msg_to_usb(rl78_i1c_message_t const * msg)
{
    static char l_buf[1024];
    fsp_err_t err;

    int buf_fill = sprintf(l_buf, "\x1b[2J\x1b[1;1HParameter Table\n\r"
                  "\x1b[36mVoltage RMS            %.3f [V]\n\r"
                  "\x1b[33mCurrent RMS Shunt      %.3f [A]\n\r"
                  "\x1b[36mCurrent RMS CT         %.3f [A]\n\r"
                  "\x1b[33mActive Power           %.3f [W]\n\r"
                  "\x1b[36mFundamental Power      %.3f [W]\n\r"
                  "\x1b[33mReactive Power         %.3f [VAr]\n\r"
                  "\x1b[36mApparent Power         %.3f [VA]\n\r"
                  "\x1b[33mPower Factor           %.3f\n\r"
                  "\x1b[36mPower Factor Sign      %s\n\r"
                  "\x1b[33mLine Frequency         %.2f [Hz]\n\r"
                  "\x1b[36mTotal Active Energy    %.3f [kWh]\n\r"
                  "\x1b[33mTotal Reactive Energy  %.3f [kVArh]\n\r"
                  "\x1b[36mTotal Apparent Energy  %.3f [kVA]\n\r"
                  "\x1b[33mActive Max Demand      %.3f [kWh]\n\r"
                  "\x1b[36mReactive Max Demand    %.3f [kVArh]\n\r"
                  "\x1b[33mApparent Max Demand    %.3f [kVAh]\n\r"
                  "\x1b[36mAmbient Temperature    %.3f [degree.C]\n\r"
                  "\x1b[0m",
                  msg->v_rms, msg->i_rms_shunt, msg->i_rms_ct,
                  msg->act_pwr, msg->fund_pwr, msg->react_pwr,
                  msg->appar_pwr, msg->p_fact, msg->p_fact_sign,
                  msg->line_frq, msg->tot_act_ergy, msg->tot_react_ergy,
                  msg->tot_appar_ergy, msg->act_max_dmnd, msg->react_max_dmnd,
                  msg->appar_max_dmnd, msg->amb_temp);

    /* Ensure we are successfully printing formatted data to the buffer*/
    assert((buf_fill > 0) && (buf_fill < (int)sizeof(l_buf)));

    /* Write data to USB*/
    err = R_USB_Write(&g_basic0_ctrl, (uint8_t *)l_buf, (uint32_t)strlen(l_buf),  USB_CLASS_PCDC);

    /* USB Write will fail if we are not connected - so only blocking wait if we are connected*/
    if(FSP_SUCCESS == err)
    {
        /* Wait USB Write to complete */
        BaseType_t status = xSemaphoreTake( g_usb_write_complete_binary_semaphore, portMAX_DELAY );
        assert(pdTRUE == status);
    }
}
/*END OF FUNCTION*/

/** @brief uart call back function.
 * @param p_args - pointer to structure containing callback/uart data.
 */
void rl78i1c_uart_callback(uart_callback_args_t *p_args)
{
    BaseType_t xHigherPriorityTaskWoken  = pdFALSE;

    if (UART_EVENT_RX_CHAR == p_args->event)
    {
        xStreamBufferSendFromISR(rl78i1c_uart_sb, (char *)&p_args->data, 1, &xHigherPriorityTaskWoken);
    }
    else if (UART_EVENT_TX_COMPLETE == p_args->event)
    {
        xSemaphoreGiveFromISR(rl78i1c_tx_complete_semaphore, &xHigherPriorityTaskWoken);
    }
    else
    {
        /* Do Nothing*/
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
/* END OF FUNCTION*/
