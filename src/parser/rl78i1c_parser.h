#ifndef PARSER_RL78I1C_PARSER_H_
#define PARSER_RL78I1C_PARSER_H_

#include "stdint.h"

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

/** @brief Parse the raw input from the display command into valid data.
 * This function is NOT thread safe & NOT re-entrant.
 */
rl78_i1c_message_t const * Parse_display(char const * p_raw_data_buf, uint32_t num_bytes_in_raw_buf);

#endif /* PARSER_RL78I1C_PARSER_H_ */
