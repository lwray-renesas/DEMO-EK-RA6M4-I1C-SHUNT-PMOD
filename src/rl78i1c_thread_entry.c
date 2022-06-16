#include "rl78i1c_thread.h"
#include "gpt_pwm.h"
#include "stdlib.h"
#include "stdio.h"

/********************************************************************************
 *                                  MACROS                                      *
 ********************************************************************************/
/** @brief GPERF Minimum word length of any tokens to parse - for length validation*/
#define MIN_WORD_LENGTH 11
/** @brief GPERF Maximum word length of any tokens to parse - for length validation*/
#define MAX_WORD_LENGTH 21
/** @brief GPERF Minimum hash value of any tokens to parse - for quick value validation*/
#define MIN_HASH_VALUE 11
/** @brief GPERF Maximum hash value of any tokens to parse - for quick value validation*/
#define MAX_HASH_VALUE 47

/** @brief maximum raw buffer size - 3k*/
#define MAX_RAW_BUF_SIZE  3072U

/** @brief Macro to determine whether a token string start condition is found (true if it has)*/
#define TOKEN_START(p)  (*(p) == '|')
/** @brief Macro to determine whether a token string end condition is found (true if it has)*/
#define TOKEN_END(p)    (*(p) == '\r')
/** @brief Macro to determine whether a parameter token start condition is found (true if it has)*/
#define PARAM_START(p)  ((*(p) >= 'A') && (*(p) <= 'Z'))
/** @brief Macro to determine whether a parameter token end condition is found (true if it has)*/
#define PARAM_END(p)    ((*(p) == ' ') && (*((p)+1) == ' '))
/** @brief Macro to determine whether a value token start condition is found (true if it has)*/
#define VALUE_START(p)  (((*(p) >= 'A') && (*(p) <= 'Z')) ||  \
		                ((*(p) >= '0') && (*(p) <= '9')) || \
		                (*(p) == '-'))
/** @brief Macro to determine whether a value token end condition is found (true if it has)*/
#define VALUE_END(p)    ((*(p) == ' ') && (*((p)+1) == '|'))
/** @brief Macro to determine whether a buffer overrun has occurred (true if it has)*/
#define BUFFER_OVERRUN(p, q)    ((p) >= (q))

/********************************************************************************
 *                         TYPES, ENUMS & STRUCTS                               *
 ********************************************************************************/
/** @enum enumerated values for populating rl78i1c message*/
typedef enum
{
    V_RMS = 0,
    I_RMS_SHUNT,
    I_RMS_CT,
    ACT_POWER,
    FUND_POWER,
    REACT_POWER,
    APPAR_POWER,
    P_FACT,
    LINE_FREQ,
    TOTAL_ACT_ENERGY,
    TOTAL_REACT_ENERGY,
    TOTAL_APPAR_ENERGY,
    ACT_MAX_DEMAND,
    REACT_MAX_DEMAND,
    APPAR_MAX_DEMAND,
    AMB_TEMP,
    P_FACT_SIGN,
    UNSUPPORTED
}rl78_i1c_message_key;

/** @brief struct containing rl78i1c message data*/
typedef struct
{
    float v_rms;            /**< RMS Voltage*/
    float i_rms_shunt;      /**< RMS Current (Shunt)*/
    float i_rms_ct;         /**< RMS Current (Current Transformer)*/
    float act_pwr;          /**< Active Power*/
    float fund_pwr;         /**< Fundamental Power*/
    float react_pwr;        /**< Reactive Power*/
    float appar_pwr;        /**< Apparent Power*/
    float p_fact;           /**< Power Factor*/
    float line_frq;         /**< Line Frequency*/
    float tot_act_ergy;     /**< Total Active Energy*/
    float tot_react_ergy;   /**< Total Reactive Energy*/
    float tot_appar_ergy;   /**< Total Apparent Energy*/
    float act_max_dmnd;     /**< Active Max Demand*/
    float react_max_dmnd;   /**< Reactive Max Demand*/
    float appar_max_dmnd;   /**< Apparent Max Demand*/
    float amb_temp;         /**< Ambient Temperature*/
    char p_fact_sign[16];   /**< Power Factor Sign*/
}rl78_i1c_message_t;

/** @brief parameter list of rl78 i1c parser*/
typedef struct
{
    char const * const name;                /**< parameter name*/
    rl78_i1c_message_key const  key_value;  /** parameter enumerated type*/
}parameter_list_t;

/** @brief enumerated type whose values represent the state of the parser*/
typedef enum
{
    look_for_start = 0,     /**< State representing looking for start of token string*/
    look_for_parameter,     /**< State representing looking for parameter token*/
    look_for_value,         /**< State representing looking for value token*/
    look_for_end,           /**< State representing looking for end of token string*/
    buffer_processed        /**< State representing buffer processed*/
}parsing_states_t;

/********************************************************************************
 *                                  VARIABLES                                   *
 ********************************************************************************/
/** @brief CMD_PROMPT string*/
static char const CMD_PROMPT[] = "CMD>";
/** @brief DISPLAY_COMMAND string*/
static char const DISPLAY_COMMAND[] = "display\r";
/** raw character buffer from the rl78i1c UART*/
static char rl78i1c_raw_msg_buffer[MAX_RAW_BUF_SIZE] = {0};
/** raw character buffer fill level*/
static uint32_t bytes_in_raw_buffer = 0U;
/** rl78i1c message*/
static rl78_i1c_message_t rl78i1c_msg;

/********************************************************************************
 *                                  FUNCTIONS                                   *
 ********************************************************************************/
/** @brief Wait indefinitely for the CMD_PROMPT message to be received.*/
static void Wait_for_cmd(void);
/** @brief Parse the raw inputs into valid data.*/
static void Parser(void);
/** @brief Sends display command.*/
static void Send_display(void);
/** @brief Writes rl78/i1c data to terminal (uses clear screen ansi symbol so must use terminal with ansi escape sequence support)*/
static void Write_rl78i1c_msg_to_usb(void);


/** @brief RL78/I1C Thread entry function */
void rl78i1c_thread_entry(void *pvParameters)
{
    fsp_err_t  err; /* return variable used to check for errors*/
    FSP_PARAMETER_NOT_USED (pvParameters);

    /* Open the UART driver to communicate with the RL78/I1C*/
    err = R_SCI_UART_Open(&rl78i1c_uart_ctrl, &rl78i1c_uart_cfg);
    assert(FSP_SUCCESS == err);

    /* Start the PWM to drive the RL78/I1C*/
    err = pwm_start(&rl78i1c_psu_driver_ctrl, &rl78i1c_psu_driver_cfg);
    assert(FSP_SUCCESS == err);

    /* Startup USB for streaming out rl78i1c data*/
    err = R_USB_Open (&g_basic0_ctrl, &g_basic0_cfg);
    assert(FSP_SUCCESS == err);

    /* Wait indefinitely for the CMD_PROMPT string - first time is usless*/
    Wait_for_cmd();

    while (1)
    {
        /* Send the display command again*/
        Send_display();

        /* Wait indefinitely for the CMD_PROMPT string - which marks the end of the display content*/
        Wait_for_cmd();

        /* Try parse the i1c data*/
        Parser();

        /* Send out the data to USB*/
        Write_rl78i1c_msg_to_usb();
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


/** @brief Computes hash of input string according to Gperf spec.
 * @param str - input string for computing hash.
 * @param len - length of input string.
 * @return Returns hashed value of input string.
 */
static uint32_t Hash(const char *str, uint32_t len)
{
    static const unsigned char asso_values[] =
    {
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 30, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 25, 48, 48, 48, 48,
     20, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48,  0, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48,  0, 48, 48, 48, 48, 48, 48, 48, 48,
     20, 48, 48, 48,  8, 48, 10, 48,  0, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
     48, 48, 48, 48, 48, 48
    };

    return len + asso_values[(unsigned char)str[6]];
}
/*END OF FUNCTION*/

/** @brief Finds the appropriate RL78 I1C message parameter.
 * @param str - string to parse for the message key.
 * @param len - length of string to parse for the message key.
 * @return key value.
 */
static parameter_list_t const * Find_rl78i1c_param(const char *str, uint32_t len)
{
    static const parameter_list_t wordlist[] =
    {
     {"", UNSUPPORTED}, {"", UNSUPPORTED}, {"", UNSUPPORTED}, {"", UNSUPPORTED}, {"", UNSUPPORTED}, {"", UNSUPPORTED},
     {"", UNSUPPORTED}, {"", UNSUPPORTED}, {"", UNSUPPORTED}, {"", UNSUPPORTED}, {"", UNSUPPORTED},
     {"Voltage RMS", V_RMS},
     {"", UNSUPPORTED}, {"", UNSUPPORTED},
     {"Reactive Power", REACT_POWER},
     {"", UNSUPPORTED}, {"", UNSUPPORTED},
     {"Fundamental Power", FUND_POWER},
     {"", UNSUPPORTED},
     {"Reactive Max Demand", REACT_MAX_DEMAND},
     {"", UNSUPPORTED},
     {"Total Reactive Energy", TOTAL_REACT_ENERGY},
     {"Line Frequency", LINE_FREQ},
     {"", UNSUPPORTED},
     {"Current RMS CT", I_RMS_CT},
     {"", UNSUPPORTED}, {"", UNSUPPORTED},
     {"Current RMS Shunt", I_RMS_SHUNT},
     {"", UNSUPPORTED},
     {"Ambient Temperature", AMB_TEMP},
     {"", UNSUPPORTED}, {"", UNSUPPORTED},
     {"Power Factor", P_FACT},
     {"", UNSUPPORTED},
     {"Apparent Power", APPAR_POWER},
     {"", UNSUPPORTED}, {"", UNSUPPORTED},
     {"Power Factor Sign", P_FACT_SIGN},
     {"", UNSUPPORTED},
     {"Apparent Max Demand", APPAR_MAX_DEMAND},
     {"", UNSUPPORTED}, {"", UNSUPPORTED},
     {"Active Power", ACT_POWER},
     {"", UNSUPPORTED},
     {"Total Active Energy", TOTAL_ACT_ENERGY},
     {"", UNSUPPORTED},
     {"Total Apparent Energy", TOTAL_APPAR_ENERGY},
     {"Active Max Demand", ACT_MAX_DEMAND}
    };

    /* Manually initialise to an invalid entry*/
    parameter_list_t const * ret_val = &wordlist[0];

    /* Validate length of string*/
    if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
        /* Compute hash*/
        uint32_t key = Hash (str, len);

        /* Quick key range validation*/
        if (key <= MAX_HASH_VALUE)
        {
            /* Validate parameter name against token*/
            if (0 == strcmp(str, wordlist[key].name))
            {
                ret_val = &wordlist[key];
            }
        }
    }
    return ret_val;
}
/*END OF FUNCTION*/

/** @brief Sets the global message structure acording to parameter passed
 * @param key - key of the parameter in the message to set.
 * @param value_str - valuye string to set.
 */
static void Set_rl78i1c_message_param(rl78_i1c_message_key key, char const * value_str)
{
    /* If power factor sign just copy the string across*/
    if(P_FACT_SIGN == key)
    {
        (void)strcpy(rl78i1c_msg.p_fact_sign, value_str);
    }
    else
    {
        /* Access the struct as an array because the enum values line up correctly*/
        *((float *) (&rl78i1c_msg) + key) = strtof(value_str, NULL);
    }
}
/*END OF FUNCTION*/

static void Parser(void)
{
    parsing_states_t parsing_state = look_for_start;
    uint32_t parsing_byte_index = 0U;
    parameter_list_t const * l_param = NULL;

    while(buffer_processed != parsing_state)
    {
        switch(parsing_state)
        {
            case look_for_start:
            {
                while((!TOKEN_START(&rl78i1c_raw_msg_buffer[parsing_byte_index])) &&
                        (!BUFFER_OVERRUN(parsing_byte_index, bytes_in_raw_buffer)))
                {
                    parsing_byte_index += 1U;
                }

                parsing_state = !BUFFER_OVERRUN(parsing_byte_index, bytes_in_raw_buffer) ? look_for_parameter : buffer_processed;
            }
            break;

            case look_for_parameter:
            {
                static char param_buf[32] = {0};
                uint32_t param_len = 0U;

                /* Scan forward until we hit start of the parameter token sequence*/
                while((!PARAM_START(&rl78i1c_raw_msg_buffer[parsing_byte_index])) &&
                        (!BUFFER_OVERRUN(parsing_byte_index, bytes_in_raw_buffer)))
                {
                    parsing_byte_index += 1U;
                }

                /* Keep counting until we hit the end of parameter token sequence*/
                while((!PARAM_END(&rl78i1c_raw_msg_buffer[parsing_byte_index])) &&
                        (!BUFFER_OVERRUN(parsing_byte_index, bytes_in_raw_buffer)))
                {
                    /* Increment the local byte index to track token length*/
                    param_buf[param_len] = rl78i1c_raw_msg_buffer[parsing_byte_index];
                    param_len += 1U;
                    parsing_byte_index += 1U;
                }

                param_buf[param_len] = '\0'; /* NULL Terminate*/

                /* Try find parameter in located string*/
                l_param = Find_rl78i1c_param(param_buf, param_len);

                if(UNSUPPORTED == l_param->key_value)
                {
                    parsing_state = look_for_end;
                }
                else
                {
                    parsing_state = !BUFFER_OVERRUN(parsing_byte_index, bytes_in_raw_buffer) ? look_for_value : buffer_processed;
                }
            }
            break;

            case look_for_value:
            {
                static char value_buf[32] = {0};
                uint32_t value_len = 0U;

                /* Scan forward until we hit a numerical character*/
                while((!VALUE_START(&rl78i1c_raw_msg_buffer[parsing_byte_index])) &&
                        (!BUFFER_OVERRUN(parsing_byte_index, bytes_in_raw_buffer)))
                {
                    parsing_byte_index += 1U;
                }

                /* Keep counting until we hit end of value token sequence*/
                while((!VALUE_END(&rl78i1c_raw_msg_buffer[parsing_byte_index])) &&
                        (!BUFFER_OVERRUN(parsing_byte_index, bytes_in_raw_buffer)))
                {
                    /* Increment the local byte index to track token length*/
                    value_buf[value_len] = rl78i1c_raw_msg_buffer[parsing_byte_index];
                    value_len += 1U;
                    parsing_byte_index += 1U;
                }

                value_buf[value_len] = '\0'; /* NULL Terminate*/

                Set_rl78i1c_message_param(l_param->key_value, value_buf);
                parsing_state = !BUFFER_OVERRUN(parsing_byte_index, bytes_in_raw_buffer) ? look_for_end : buffer_processed;
            }
            break;

            case look_for_end:
            {
                while((!TOKEN_END(&rl78i1c_raw_msg_buffer[parsing_byte_index])) &&
                        (!BUFFER_OVERRUN(parsing_byte_index, bytes_in_raw_buffer)))
                {
                    parsing_byte_index += 1U;
                }

                parsing_state = !BUFFER_OVERRUN(parsing_byte_index, bytes_in_raw_buffer) ? look_for_start : buffer_processed;
            }
            break;

            default:
                /* Buffer processed, nothing left to do*/
                break;
        }
    }
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

static void Write_rl78i1c_msg_to_usb(void)
{
    static char l_buf[4096];

    (void)sprintf(l_buf, "\x1b[2J\x1b[1;1HParameter Table\n\r"
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
                  rl78i1c_msg.v_rms, rl78i1c_msg.i_rms_shunt, rl78i1c_msg.i_rms_ct,
                  rl78i1c_msg.act_pwr, rl78i1c_msg.fund_pwr, rl78i1c_msg.react_pwr,
                  rl78i1c_msg.appar_pwr, rl78i1c_msg.p_fact, rl78i1c_msg.p_fact_sign,
                  rl78i1c_msg.line_frq, rl78i1c_msg.tot_act_ergy, rl78i1c_msg.tot_react_ergy,
                  rl78i1c_msg.tot_appar_ergy, rl78i1c_msg.act_max_dmnd, rl78i1c_msg.react_max_dmnd,
                  rl78i1c_msg.appar_max_dmnd, rl78i1c_msg.amb_temp);

    /* Write data to USB*/
    fsp_err_t err = R_USB_Write(&g_basic0_ctrl, (uint8_t *)l_buf, (uint32_t)strlen(l_buf),  USB_CLASS_PCDC);
    assert(FSP_SUCCESS == err);

    /* Wait USB Write to complete */
    BaseType_t status = xSemaphoreTake( g_usb_write_complete_binary_semaphore, portMAX_DELAY );
    assert(pdTRUE == status);
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
