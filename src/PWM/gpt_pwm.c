#include "rl78i1c_Thread.h"
#include "gpt_pwm.h"

#define GTIO_INITIAL_OUTPUT_LOW     (0x0U << 4)
#define GTIO_INITIAL_OUTPUT_HIGH    (0x1U << 4)

#define GTIO_LOW_CYCLE_END          (0x1U << 2)
#define GTIO_HIGH_CYCLE_END         (0x2U << 2)

#define GTIO_TOGGLE_COMPARE_MATCH   (0x3U << 0)


/*******************************************************************************
 * Outline         : pwm_start(gpt_instance_ctrl_t * const p_ctrl, timer_cfg_t const * const p_cfg)
 * Description     : Configures both PWM  (1, 2), sets duty cycle, and starts.
 * Argument        :
 *                  p_ctrl - pointer to gpt control structure
 *                  p_cfg - pointer to gpt configuration structure
 * Return value    : fsp_err status of GPT api calls.
 *******************************************************************************/
fsp_err_t pwm_start(gpt_instance_ctrl_t * const p_ctrl, timer_cfg_t const * const p_cfg)
{
    fsp_err_t status = FSP_SUCCESS;

    /* open g_pwm1_timer */
    status = R_GPT_Open(p_ctrl, p_cfg);
    if(status != FSP_SUCCESS)
    {
        return status;
    }

    /* Set a starting value for the GPT */
    status = R_GPT_CounterSet(p_ctrl,  (83 * p_ctrl->p_reg->GTPR)/100); //83%
    if(status != FSP_SUCCESS)
    {
        return status;
    }

    /* Set a starting value of the  GPT GTCCRn registers */
    status = R_GPT_DutyCycleSet(p_ctrl, (25 * p_ctrl->p_reg->GTPR)/100, GPT_IO_PIN_GTIOCA);//25%
    if(status != FSP_SUCCESS)
    {
        return status;
    }

    status = R_GPT_DutyCycleSet(p_ctrl, (75 * p_ctrl->p_reg->GTPR)/100, GPT_IO_PIN_GTIOCB);//75%
    if(status != FSP_SUCCESS)
    {
        return status;
    }

    rl78i1c_psu_driver_ctrl.p_reg->GTIOR_b.GTIOA = GTIO_INITIAL_OUTPUT_LOW  | GTIO_LOW_CYCLE_END  | GTIO_TOGGLE_COMPARE_MATCH;     /* Set the GTIOR settings for A pin */
    rl78i1c_psu_driver_ctrl.p_reg->GTIOR_b.GTIOB = GTIO_INITIAL_OUTPUT_HIGH | GTIO_HIGH_CYCLE_END | GTIO_TOGGLE_COMPARE_MATCH;     /* Set the TIOR settings for B pin */

    /* start the pwm and run forever */
    status = R_GPT_Start(p_ctrl);
    if(status != FSP_SUCCESS)
    {
        return status;
    }

    return status;
}
/*******************************************************************************
 * End of function pwm_start
 *******************************************************************************/

/*******************************************************************************
 * End of file gpt_pwm.c
 *******************************************************************************/
