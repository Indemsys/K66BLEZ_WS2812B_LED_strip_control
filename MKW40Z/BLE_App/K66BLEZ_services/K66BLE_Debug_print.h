#ifndef DEBUG_PRINT_H
  #define DEBUG_PRINT_H

  #include "SEGGER_RTT.h"

//#define TRACE_BLE // Макрос разрешающий вывод отладочныхх сообщений при вызове основных функций стека BLE
//#define LOG_PERFOMANCE_TEST
#define LOG_CMDSERV_TEST

  #ifdef TRACE_BLE
    #define DEBUG_PRINT( X )           SEGGER_RTT_printf( 0, X )
    #define DEBUG_PRINT_ARG( X, ... )  SEGGER_RTT_printf( 0, X, __VA_ARGS__)
  #else
    #define DEBUG_PRINT( X )
    #define DEBUG_PRINT_ARG( X, ... )
  #endif

  #ifdef LOG_PERFOMANCE_TEST
    #define  DEBUG_PERF_TEST_PRINT_ARG(X, ...) SEGGER_RTT_printf(0, X, __VA_ARGS__)
  #else
    #define  DEBUG_PERF_TEST_PRINT_ARG(X, ...)
  #endif


  #ifdef LOG_CMDSERV_TEST
    #define  DEBUG_CMDSERV_PRINT_ARG(X, ...) SEGGER_RTT_printf(0, X, __VA_ARGS__)
  #else
    #define  DEBUG_CMDSERV_PRINT_ARG(X, ...)
  #endif


#endif // DEBUG_PRINT_H



