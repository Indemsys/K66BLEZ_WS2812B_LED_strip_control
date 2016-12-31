/*!
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* All rights reserved.
*
* \file PWR_Platform.c
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of Freescale Semiconductor, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*****************************************************************************
 *                               INCLUDED HEADERS                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the headers that this module needs to include.    *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
#include "EmbeddedTypes.h"
#include "PhyTypes.h"
#include "Phy.h"
#include "PWR_Configuration.h"
#include "PWRLib.h"
#include "PWR_Interface.h"

#include "TimersManager.h"
//#include "Keyboard.h"
#include "SerialManager.h"

#include "fsl_smc_hal.h"
#include "fsl_clock_manager.h"
#include "fsl_interrupt_manager.h"
#include "fsl_llwu_hal.h"

#include "fsl_lptmr_driver.h"
#include "TMR_Adapter.h"

#include "board.h"
#include "fsl_osa_ext.h"
#include "PWR_Platform.h"

#ifdef gDCDC_Enabled_d
#include "DCDC.h"
#endif

/*****************************************************************************
 *                        PRIVATE TYPE DEFINITIONS                           *
 *---------------------------------------------------------------------------*
 * Add to this section all the data types definitions: stuctures, unions,    *
 * enumerations, typedefs ...                                                *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
typedef void (*xcvr_ptr)(void);

/*****************************************************************************
 *                               PUBLIC VARIABLES                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the variables and constants that have global      *
 * (project) scope.                                                          *
 * These variables / constants can be accessed outside this module.          *
 * These variables / constants shall be preceded by the 'extern' keyword in  *
 * the interface header.                                                     *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
extern const clock_manager_user_config_t g_defaultClockConfigRun;
extern const clock_manager_user_config_t g_defaultClockConfigVlpr;

/*****************************************************************************
 *                               PRIVATE VARIABLES                           *
 *---------------------------------------------------------------------------*
 * Add to this section all the variables and constants that have local       *
 * (file) scope.                                                             *
 * Each of this declarations shall be preceded by the 'static' keyword.      *
 * These variables / constants cannot be accessed outside this module.       *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
static uint8_t gWakeUpSource = 0;
static smc_power_mode_config_t mPWRP_Config;
static uint16_t gu16WakeUpTime = 0;
static bool_t bRadioIRQWakeUp = FALSE;
/*****************************************************************************
 *                           PRIVATE FUNCTIONS PROTOTYPES                    *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions prototypes that have local (file)   *
 * scope.                                                                    *
 * These functions cannot be accessed outside this module.                   *
 * These declarations shall be preceded by the 'static' keyword.             *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
static xcvr_ptr pSendRadioToMode = NULL;
static void Disable_WakeUp(bool_t);
static void Enable_WakeUp(bool_t);
static void Enter_LPMode(power_modes_t mode, smc_stop_submode_t submode);

/*****************************************************************************
 *                           PUBLIC FUNCTIONS PROTOTYPES                     *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions prototypes that have local (file)   *
 * scope.                                                                    *
 * These functions cannot be accessed outside this module.                   *
 * These declarations shall be preceded by the 'static' keyword.             *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
extern void PWRLib_LLWU_Isr(void);

/*---------------------------------------------------------------------------
 * Name: PWRP_Init
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PWRP_Init(void)
{
  /*Init LPTMR*/
  lptmr_prescaler_user_config_t prescaler_config;

  LPTMR_Init(NULL);

  prescaler_config.prescalerBypass = 1;

  prescaler_config.prescalerClockSelect = kLptmrPrescalerClock1;

  LPTMR_HAL_SetPrescalerMode(g_lptmrBase[gLptmrInstance_c], prescaler_config);

  LPTMR_HAL_Disable(g_lptmrBase[gLptmrInstance_c]);

  /*Un-register modules from LLWU*/
  LLWU_HAL_SetExternalInputPinMode(LLWU, kLlwuExternalPinDisabled, PWRP_LLWU_WAKE_UP_PIN);
  //LPTMR module connected to LLWU
  LLWU_HAL_SetInternalModuleCmd(LLWU, (llwu_wakeup_module_t)kLlwuWakeupModule0, false);

  /* Clear external pins wakeup interrupts */
  LLWU_F1 = LLWU_F1;
  LLWU_F2 = LLWU_F2;

  /* LPTMR is wakeup source */
  if( LLWU_HAL_GetInternalModuleWakeupFlag(LLWU, (llwu_wakeup_module_t)kLlwuWakeupModule0) )
  {
    /* Clear LPTMR interrupt */
    LPTMR_HAL_ClearIntFlag(g_lptmrBase[gLptmrInstance_c]);
  }
  OSA_EXT_InstallIntHandler(LLWU_IRQn, PWRLib_LLWU_Isr);
  NVIC_SetPriority(LLWU_IRQn, 0x80 >> (8 - __NVIC_PRIO_BITS));
  NVIC_EnableIRQ(LLWU_IRQn);
}

/*---------------------------------------------------------------------------
 * Name: PWRP_UpdateLPTMRCount
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PWRP_UpdateLPTMRCount(uint16_t u16TimeInMs)
{
  gu16WakeUpTime = u16TimeInMs;
}

/*---------------------------------------------------------------------------
 * Name: PWRP_SetWakeUpSource
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
pwrp_status_t PWRP_SetWakeUpSource(uint8_t u8WUSource)
{
  if ( u8WUSource != LPTMR_WAKE_UP && u8WUSource != UART_WAKE_UP && u8WUSource != GPIO_WAKE_UP
       && u8WUSource != XCVR_IRQ_WAKE_UP )
  {
    return PWRP_ERR;
  }
  gWakeUpSource = u8WUSource;
  return PWRP_OK;
}

/*---------------------------------------------------------------------------
 * Name: PWRP_GetWakeUpReason
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PWRP_GetWakeUpReason(void)
{
  if(PWRLib_MCU_WakeupReason.Bits.FromKeyBoard && gWakeUpSource == GPIO_WAKE_UP)
  {
    return GPIO_WAKE_UP;
  }
  if(PWRLib_MCU_WakeupReason.Bits.FromLPTMR && gWakeUpSource == LPTMR_WAKE_UP)
  {
    return LPTMR_WAKE_UP;
  }
  if(PWRLib_MCU_WakeupReason.Bits.FromPSwitch_UART && gWakeUpSource == UART_WAKE_UP)
  {
    return UART_WAKE_UP;
  }
  if(bRadioIRQWakeUp && gWakeUpSource == XCVR_IRQ_WAKE_UP)
  {
    return XCVR_IRQ_WAKE_UP;
  }

  return UNKNOWN_WAKE_UP;
}
/*---------------------------------------------------------------------------
 * Name: Enter_LPMode
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
static void Enter_LPMode(power_modes_t mode, smc_stop_submode_t submode)
{
  bool_t bLLWUWakeUp = FALSE;
  uint32_t temp = 0;

  OSA_EXT_InterruptDisable();

  /*disable systick*/
  temp = SysTick->CTRL & (SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
  SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);

  mPWRP_Config.powerModeName = mode;
  if(mode == kPowerModeLls || mode == kPowerModeVlls)
  {
    mPWRP_Config.stopSubMode = submode;
    bLLWUWakeUp = TRUE;
  }
  else
  {
    (void)submode;
    bLLWUWakeUp = FALSE;
  }
  bRadioIRQWakeUp = FALSE;
  PWRLib_MCU_WakeupReason.AllBits = 0;
  /*enables wake up source based on previously applied configuration*/
  Enable_WakeUp(bLLWUWakeUp);
  /* configure MCG in BLPI mode */
  CLOCK_SYS_SetConfiguration(&g_defaultClockConfigVlpr);

#if gDCDC_Enabled_d
  /*prepare DCDC module for low power*/
  if(mode != kPowerModeWait && mode != kPowerModeStop)
  {
    DCDC_PrepareForPulsedMode();
  }
#endif

  /*enter configure low power mode*/
  SMC_HAL_SetMode(SMC, &mPWRP_Config);

#if gDCDC_Enabled_d
  /*prepare DCDC module for run mode*/
  if(mode != kPowerModeWait && mode != kPowerModeStop)
  {
    DCDC_PrepareForContinuousMode();
  }
#endif

  /*if previous mode was vlpw then MCU is running in vlpr
  must switch back to run*/
  if(mode == kPowerModeVlpw)
  {
    mPWRP_Config.powerModeName = kPowerModeRun;
    SMC_HAL_SetMode(SMC, &mPWRP_Config);
  }
  /* checks source of wakeup and disables it */
  Disable_WakeUp(bLLWUWakeUp);

  /* configure MCG in PEE mode*/
  CLOCK_SYS_SetConfiguration(&g_defaultClockConfigRun);

  /* restore the state of SysTick */
  SysTick->CTRL |= temp;

  OSA_EXT_InterruptEnable();
}

/*---------------------------------------------------------------------------
 * Name: Enable_WakeUp
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
static void Enable_WakeUp(bool_t bWakeFromLLWU)
{
  if(bWakeFromLLWU)
  {
    if(gWakeUpSource == LPTMR_WAKE_UP)
    {
      /* enable LPTMR as wakeup source for LLWU module */
      LLWU_HAL_SetInternalModuleCmd(LLWU, (llwu_wakeup_module_t)kLlwuWakeupModule0, true);
      LPTMR_HAL_SetCompareValue(g_lptmrBase[gLptmrInstance_c], gu16WakeUpTime);
      LPTMR_HAL_Enable(g_lptmrBase[gLptmrInstance_c]);
    }
    else if(gWakeUpSource == GPIO_WAKE_UP)
    {
      LLWU_HAL_SetExternalInputPinMode(LLWU, kLlwuExternalPinChangeDetect, PWRP_LLWU_WAKE_UP_PIN);
    }
  }
  else
  {
    if(gWakeUpSource == UART_WAKE_UP)
    {
      Serial_EnableLowPowerWakeup(gSerialMgrLpuart_c);
    }
    else if(gWakeUpSource == LPTMR_WAKE_UP)
    {
      LPTMR_HAL_SetCompareValue(g_lptmrBase[gLptmrInstance_c], gu16WakeUpTime);
      LPTMR_HAL_Enable(g_lptmrBase[gLptmrInstance_c]);
    }
    else if(gWakeUpSource == GPIO_WAKE_UP)
    {

    }
    else if(gWakeUpSource == XCVR_IRQ_WAKE_UP)
    {
    }
  }
}

/*---------------------------------------------------------------------------
 * Name: Disable_WakeUp
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
static void Disable_WakeUp(bool_t bWakeFromLLWU)
{
  if(bWakeFromLLWU)
  {
    PWRLib_LLWU_UpdateWakeupReason();

    if(PWRLib_MCU_WakeupReason.Bits.FromKeyBoard)
    {
      LLWU_HAL_SetExternalInputPinMode(LLWU, kLlwuExternalPinDisabled, PWRP_LLWU_WAKE_UP_PIN);
    }
    else if(PWRLib_MCU_WakeupReason.Bits.FromLPTMR)
    {
      LPTMR_HAL_Disable(g_lptmrBase[gLptmrInstance_c]);
      LLWU_HAL_SetInternalModuleCmd(LLWU, (llwu_wakeup_module_t)kLlwuWakeupModule0, false);
    }
  }
  else
  {
    if(Serial_IsWakeUpSource(gSerialMgrLpuart_c))
    {
      //UART
      PWRLib_MCU_WakeupReason.Bits.FromPSwitch_UART = TRUE;
      Serial_DisableLowPowerWakeup(gSerialMgrLpuart_c);
    }
    else if (gWakeUpSource == UART_WAKE_UP)
    {
      Serial_DisableLowPowerWakeup(gSerialMgrLpuart_c);
    }
    if(LPTMR_HAL_IsIntPending(g_lptmrBase[gLptmrInstance_c]))
    {
      LPTMR_HAL_ClearIntFlag(g_lptmrBase[gLptmrInstance_c]);
      LPTMR_HAL_Disable(g_lptmrBase[gLptmrInstance_c]);
      PWRLib_MCU_WakeupReason.Bits.FromLPTMR = TRUE;
    }
    else if(gWakeUpSource == LPTMR_WAKE_UP)
    {
      LPTMR_HAL_ClearIntFlag(g_lptmrBase[gLptmrInstance_c]);
      LPTMR_HAL_Disable(g_lptmrBase[gLptmrInstance_c]);
    }

    /*check for radio wake-up*/
    if(PHY_isIrqPending())
    {
      bRadioIRQWakeUp = TRUE;
    }
  }
}

/*---------------------------------------------------------------------------
 * Name: Enter_RUN
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
pwrp_status_t Enter_RUN (void)
{
  OSA_EXT_InterruptDisable();

  mPWRP_Config.powerModeName = kPowerModeRun;
  SMC_HAL_SetMode(SMC, &mPWRP_Config);
#if gDCDC_Enabled_d
  /*prepare DCDC for run mode*/
  DCDC_PrepareForContinuousMode();
#endif
  CLOCK_SYS_SetConfiguration(&g_defaultClockConfigRun);
  OSA_EXT_InterruptEnable();

  return PWRP_OK;
}

/*---------------------------------------------------------------------------
 * Name: Enter_VLPR
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
pwrp_status_t Enter_VLPR ( void )
{
  OSA_EXT_InterruptDisable();

  CLOCK_SYS_SetConfiguration(&g_defaultClockConfigVlpr);

  mPWRP_Config.powerModeName = kPowerModeVlpr;
  SMC_HAL_SetMode(SMC, &mPWRP_Config);
#if gDCDC_Enabled_d
  /*prepare DCDC for low power mode*/
  DCDC_PrepareForPulsedMode();
#endif
  OSA_EXT_InterruptEnable();
  return PWRP_OK;
}

/*---------------------------------------------------------------------------
 * Name: Enter_Wait
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
pwrp_status_t Enter_Wait ( void )
{
  PWRLib_MCU_WakeupReason.AllBits = 0;

  if(gWakeUpSource == NO_WAKE_UP)
  {
    return PWRP_ERR;
  }
/*protection against battery monitor timer interrupt*/
#if gDCDC_Enabled_d
  StackTimer_Disable();
#endif
  Enter_LPMode(kPowerModeWait, (smc_stop_submode_t)0);
#if gDCDC_Enabled_d
  StackTimer_Enable();
#endif
  return PWRP_OK;
}

/*---------------------------------------------------------------------------
 * Name: Enter_Stop
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
pwrp_status_t Enter_Stop ( void )
{
  if(gWakeUpSource == NO_WAKE_UP)
  {
    return PWRP_ERR;
  }
/*protection against battery monitor timer interrupt*/
#if gDCDC_Enabled_d
  StackTimer_Disable();
#endif
  Enter_LPMode(kPowerModeStop, (smc_stop_submode_t)0);
#if gDCDC_Enabled_d
  StackTimer_Enable();
#endif
  return PWRP_OK;
}

/*---------------------------------------------------------------------------
 * Name: Enter_VLPW
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
pwrp_status_t Enter_VLPW ( void )
{
  if(gWakeUpSource == NO_WAKE_UP)
  {
    return PWRP_ERR;
  }
  Enter_LPMode(kPowerModeVlpw, (smc_stop_submode_t)0);
  return PWRP_OK;
}

/*---------------------------------------------------------------------------
 * Name: Enter_VLPS
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
pwrp_status_t Enter_VLPS ( void )
{
  if(gWakeUpSource == NO_WAKE_UP)
  {
    return PWRP_ERR;
  }
#if gDCDC_Enabled_d
  StackTimer_Disable();
#endif
  Enter_LPMode(kPowerModeVlps, kSmcStopSub1);
#if gDCDC_Enabled_d
  StackTimer_Enable();
#endif
  return PWRP_OK;
}

/*---------------------------------------------------------------------------
 * Name: Enter_LLS
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
pwrp_status_t Enter_LLS  ( void )
{
  PWRLib_MCU_WakeupReason.AllBits = 0;

  if(gWakeUpSource == NO_WAKE_UP || gWakeUpSource == UART_WAKE_UP)
  {
    return PWRP_ERR;
  }

  Enter_LPMode(kPowerModeLls, kSmcStopSub3);
  return PWRP_OK;
}

/*---------------------------------------------------------------------------
 * Name: Enter_VLLS
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
pwrp_status_t Enter_VLLS (uint8_t u8SubMode)
{
  PWRLib_MCU_WakeupReason.AllBits = 0;

  if(gWakeUpSource == NO_WAKE_UP || gWakeUpSource == UART_WAKE_UP)
  {
    return PWRP_ERR;
  }
  Enter_LPMode(kPowerModeVlls, (smc_stop_submode_t)u8SubMode);
  return PWRP_OK;
}