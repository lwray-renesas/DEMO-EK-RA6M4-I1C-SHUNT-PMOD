
#include "rl78i1c_parser.h"
#include "stdlib.h"
#include "string.h"

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

rl78_i1c_message_t const * Parse_display(char const * p_raw_data_buf, uint32_t num_bytes_in_raw_buf)
{
    static rl78_i1c_message_t rl78i1c_msg;
    parsing_states_t parsing_state = look_for_start;
    uint32_t parsing_byte_index = 0U;
    parameter_list_t const * l_param = NULL;

    while(buffer_processed != parsing_state)
    {
        switch(parsing_state)
        {
            case look_for_start:
            {
                while((!TOKEN_START((p_raw_data_buf + parsing_byte_index))) &&
                        (!BUFFER_OVERRUN(parsing_byte_index, num_bytes_in_raw_buf)))
                {
                    parsing_byte_index += 1U;
                }

                parsing_state = BUFFER_OVERRUN(parsing_byte_index, num_bytes_in_raw_buf) ? buffer_processed : look_for_parameter;
            }
            break;

            case look_for_parameter:
            {
                static char param_buf[32] = {0};
                uint32_t param_len = 0U;

                /* Scan forward until we hit start of the parameter token sequence*/
                while((!PARAM_START((p_raw_data_buf + parsing_byte_index))) &&
                        (!BUFFER_OVERRUN(parsing_byte_index, num_bytes_in_raw_buf)))
                {
                    parsing_byte_index += 1U;
                }

                /* Keep counting until we hit the end of parameter token sequence*/
                while((!PARAM_END((p_raw_data_buf + parsing_byte_index))) &&
                        (!BUFFER_OVERRUN(parsing_byte_index, num_bytes_in_raw_buf)))
                {
                    /* Increment the local byte index to track token length*/
                    param_buf[param_len] = *(p_raw_data_buf + parsing_byte_index);
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
                    parsing_state = BUFFER_OVERRUN(parsing_byte_index, num_bytes_in_raw_buf) ? buffer_processed : look_for_value;
                }
            }
            break;

            case look_for_value:
            {
                static char value_buf[32] = {0};
                uint32_t value_len = 0U;

                /* Scan forward until we hit a numerical character*/
                while((!VALUE_START((p_raw_data_buf + parsing_byte_index))) &&
                        (!BUFFER_OVERRUN(parsing_byte_index, num_bytes_in_raw_buf)))
                {
                    parsing_byte_index += 1U;
                }

                /* Keep counting until we hit end of value token sequence*/
                while((!VALUE_END((p_raw_data_buf + parsing_byte_index))) &&
                        (!BUFFER_OVERRUN(parsing_byte_index, num_bytes_in_raw_buf)))
                {
                    /* Increment the local byte index to track token length*/
                    value_buf[value_len] = *(p_raw_data_buf + parsing_byte_index);
                    value_len += 1U;
                    parsing_byte_index += 1U;
                }

                value_buf[value_len] = '\0'; /* NULL Terminate*/

                /* If power factor sign just copy the string across*/
                if(P_FACT_SIGN == l_param->key_value)
                {
                    (void)strcpy(rl78i1c_msg.p_fact_sign, value_buf);
                }
                else
                {
                    /* Access the struct as an array because the enum values line up correctly*/
                    *((float *) (&rl78i1c_msg) + l_param->key_value) = strtof(value_buf, NULL);
                }

                parsing_state = BUFFER_OVERRUN(parsing_byte_index, num_bytes_in_raw_buf) ? buffer_processed : look_for_end;
            }
            break;

            case look_for_end:
            {
                while((!TOKEN_END((p_raw_data_buf + parsing_byte_index))) &&
                        (!BUFFER_OVERRUN(parsing_byte_index, num_bytes_in_raw_buf)))
                {
                    parsing_byte_index += 1U;
                }

                parsing_state = BUFFER_OVERRUN(parsing_byte_index, num_bytes_in_raw_buf) ? buffer_processed : look_for_start;
            }
            break;

            default:
                /* Buffer processed, nothing left to do*/
                break;
        }
    }

    return &rl78i1c_msg;
}
/*END OF FUNCTION*/
