#ifndef _APP_PREINCLUDE_H_
  #define _APP_PREINCLUDE_H_

/*! *********************************************************************************
 *      Framework Configuration
 ********************************************************************************** */
/* Defines Num of Serial Manager interfaces */
  #define gSerialManagerMaxInterfaces_c   0

/* Defines Size for Timer Task*/
  #define gTmrTaskStackSize_c  384

/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.*/
  #define PoolsDetails_c \
         _block_size_  32  _number_of_blocks_    6 _eol_  \
         _block_size_  64  _number_of_blocks_    3 _eol_  \
         _block_size_ 128  _number_of_blocks_    3 _eol_

/* Defines number of timers needed by the application */
  #define gTmrApplicationTimers_c         4

/* Defines number of timers needed by the protocol stack */
  #define gTmrStackTimers_c               5

/* Set this define TRUE if the PIT frequency is an integer number of MHZ */
  #define gTMR_PIT_FreqMultipleOfMHZ_d    0

/* Enables / Disables the precision timers platform component */
  #define gTimestamp_Enabled_d            0

/* Enable/Disable Low Power Timer */
  #define gTMR_EnableLowPowerTimers       0

/*! *********************************************************************************
 *      RTOS Configuration
 ********************************************************************************** */
/* Defines the RTOS used */
//#define FSL_RTOS_FREE_RTOS

/* Defines number of OS events used */
  #define osNumberOfEvents        4

/* Defines number of OS semaphores used */
  #define osNumberOfSemaphores    1

/* Defines main task stack size */
  #define gMainThreadStackSize_c  1024

/* Defines total heap size used by the OS */
  #define gTotalHeapSize_c        8000//7200

/*! *********************************************************************************
 *      BLE Stack Configuration
 ********************************************************************************** */
/* Defines Controller task stack size */
  #define gControllerTaskStackSize_c 832

/* Defines Host task stack size */
  #define gHost_TaskStackSize_c 1100

/* Defines L2cap task stack size */
  #define gL2ca_TaskStackSize_c 472

  #define BD_ADDR             0x01,0x00,0x00,0x9F,0x04,0x00
  #define gUseHciTransport_d  0

#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
