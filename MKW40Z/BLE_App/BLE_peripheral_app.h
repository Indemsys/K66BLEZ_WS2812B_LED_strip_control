#ifndef _APP_H_
  #define _APP_H_

// Выбор используемых профилей. Макросы взаимоисключающие
//#define HEART_RATE_SERVICE_EN  //Разкоментировать чтобы разрешить работу профиля пульсометра
  #define K66BLEZ_PROFILE        //

//#define ENABLE_SLOW_ADVERTISING  // Разрешаем переключение на удалененный период передачи объявлений в режиме адвертайсинг


/* App Configuration */

/*! Enable/disable bonding capability */
  #ifndef gBondingSupported_d
    #define gBondingSupported_d  TRUE
  #endif

/*! Enable/disable service security */
  #ifndef gUseServiceSecurity_d
    #define gUseServiceSecurity_d  TRUE
  #endif


  #define gPasskeyValue_c                   999999   // PIN код

/* Profile Parameters */

  #define gFastConnMinAdvInterval_c         32 /* 20 ms */
  #define gFastConnMaxAdvInterval_c         48 /* 30 ms */

  #define gReducedPowerMinAdvInterval_c     1600 /* 1 s */
  #define gReducedPowerMaxAdvInterval_c     4000 /* 2.5 s */


  #define gFastConnAdvTime_c                30  /* s */
  #define gReducedPowerAdvTime_c            300 /* s */


  #if gBondingSupported_d
    #define gFastConnWhiteListAdvTime_c     10 /* s */
  #else
    #define gFastConnWhiteListAdvTime_c     0
  #endif

extern gapAdvertisingData_t     gAppAdvertisingData;
extern gapScanResponseData_t    gAppScanRspData;

  #if gBondingSupported_d
extern gapSmpKeys_t gSmpKeys;
extern gapPairingParameters_t gPairingParameters;
  #endif

extern gapDeviceSecurityRequirements_t deviceSecurityRequirements;

  #ifdef __cplusplus
extern "C"
{
  #endif


void BleApp_Start(void);

  #ifdef __cplusplus
}
  #endif


#endif /* _APP_H_ */

