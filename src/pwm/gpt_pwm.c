#include "gpt_pwm.h"
#include "rl78i1c_Thread.h"

#define GTIO_INITIAL_OUTPUT_LOW     (0x0U << 4)
#define GTIO_INITIAL_OUTPUT_HIGH    (0x1U << 4)

#define GTIO_LOW_CYCLE_END          (0x1U << 2)
#define GTIO_HIGH_CYCLE_END         (0x2U << 2)

#define GTIO_TOGGLE_COMPARE_MATCH   (0x3U << 0)

/** @briefConfigures both PWM  (1, 2), sets duty cycle, and starts.
 * @param p_ctrl - pointer to gpt control structure
 * @param p_cfg - pointer to gpt configuration structure
 */
void pwm_start(gpt_instance_ctrl_t * const p_ctrl, timer_cfg_t const * const p_cfg)
{
    fsp_err_t status = FSP_SUCCESS;

    /* open gpt pwm timer */
    status = R_GPT_Open(p_ctrl, p_cfg);
    assert(FSP_SUCCESS == status);

    /* Set a starting value for the GPT */
    status = R_GPT_CounterSet(p_ctrl,  (83 * p_ctrl->p_reg->GTPR)/100); //83%
    assert(FSP_SUCCESS == status);

    /* Set a starting value of the  GPT GTCCRn registers */
    status = R_GPT_DutyCycleSet(p_ctrl, (25 * p_ctrl->p_reg->GTPR)/100, GPT_IO_PIN_GTIOCA);//25%
    assert(FSP_SUCCESS == status);

    status = R_GPT_DutyCycleSet(p_ctrl, (75 * p_ctrl->p_reg->GTPR)/100, GPT_IO_PIN_GTIOCB);//75%
    assert(FSP_SUCCESS == status);

    rl78i1c_psu_driver_ctrl.p_reg->GTIOR_b.GTIOA = GTIO_INITIAL_OUTPUT_LOW  | GTIO_LOW_CYCLE_END  | GTIO_TOGGLE_COMPARE_MATCH;     /* Set the GTIOR settings for A pin */
    rl78i1c_psu_driver_ctrl.p_reg->GTIOR_b.GTIOB = GTIO_INITIAL_OUTPUT_HIGH | GTIO_HIGH_CYCLE_END | GTIO_TOGGLE_COMPARE_MATCH;     /* Set the TIOR settings for B pin */

    /* start the pwm and run forever */
    status = R_GPT_Start(p_ctrl);
    assert(FSP_SUCCESS == status);
}
/* END OF FUNCTION*/
