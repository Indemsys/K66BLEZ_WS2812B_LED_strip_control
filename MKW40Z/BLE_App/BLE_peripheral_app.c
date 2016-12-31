/* Framework / Drivers */
#include "RNG_interface.h"
#include "TimersManager.h"
#include "MemManager.h"
#include "FunctionLib.h"
#include "fsl_os_abstraction.h"
#include "panic.h"
#include "PWR_Interface.h"


/* BLE Host Stack */
#include "l2ca_interface.h"
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "heart_rate_interface.h"

#include "board.h"
#include "BLE_common_app.h"
#include "BLE_peripheral_app.h"

#include "K66BLEZ.h"


#define smpEdiv                  0x1F99
#define mcEncryptionKeySize_c    16


/* Scanning and Advertising Data */
static const uint8_t adData0[1] =  { (gapAdTypeFlags_t)(gLeGeneralDiscoverableMode_c | gBrEdrNotSupported_c) };

#ifdef HEART_RATE_SERVICE_EN
static const uint8_t adData1[2] = { UuidArray(gBleSig_HeartRateService_d) };
static const gapAdStructure_t advScanStruct[3] = {
  {
    .length = NumberOfElements(adData0) + 1,
    .adType = gAdFlags_c,
    .aData = (void *)adData0
  },
  {
    .length = NumberOfElements(adData1) + 1,
    .adType = gAdIncomplete16bitServiceList_c,
    .aData = (void *)adData1
  },
  {
    .adType = gAdShortenedLocalName_c,
    .length = 7,
    .aData = "HR_MON"
  },
//  {
//    .adType = gAdManufacturerSpecificData_c, // Демонстрационные данные
//    .length = 9,
//    .aData = "12345678"
//  }


};
#else
//static const uint8_t adData1[16] = { uuid_service_wireless_uart };
static const gapAdStructure_t advScanStruct[3] = {
  {
    .length = NumberOfElements(adData0) + 1,
    .adType = gAdFlags_c,
    .aData = (void *)adData0
  },
  {
    .length = NumberOfElements(uuid_service_wireless_uart) + 1,
    .adType = gAdComplete128bitServiceList_c,
    .aData = (void *)uuid_service_wireless_uart
  },
  {
    .adType = gAdShortenedLocalName_c,
    .length = 8,
    .aData = "K66BLEZ"
  },
};
#endif

gapAdvertisingData_t gAppAdvertisingData =
{
  NumberOfElements(advScanStruct),
  (void *)advScanStruct
};

gapScanResponseData_t gAppScanRspData =
{
  0,
  NULL
};

/* SMP Data */
gapPairingParameters_t gPairingParameters = {
  TRUE,
  gSecurityMode_1_Level_3_c, // gSecurityMode_1_Level_3_c, Понизил уровень защиты !!!
  mcEncryptionKeySize_c,
  gIoDisplayOnly_c,
  TRUE,
  gIrk_c,
  gLtk_c
};

/* LTK */
static const uint8_t smpLtk[gcSmpMaxLtkSize_c] =
{ 0xD6, 0x93, 0xE8, 0xA4, 0x23, 0x55, 0x48, 0x99,
  0x1D, 0x77, 0x61, 0xE6, 0x63, 0x2B, 0x10, 0x8E };

/* RAND*/
static const uint8_t smpRand[gcSmpMaxRandSize_c] =
{ 0x26, 0x1E, 0xF6, 0x09, 0x97, 0x2E, 0xAD, 0x7E };

/* IRK */
static const uint8_t smpIrk[gcSmpIrkSize_c] =
{ 0x0A, 0x2D, 0xF4, 0x65, 0xE3, 0xBD, 0x7B, 0x49,
  0x1E, 0xB4, 0xC0, 0x95, 0x95, 0x13, 0x46, 0x73 };

/* CSRK */
static const uint8_t smpCsrk[gcSmpCsrkSize_c] =
{ 0x90, 0xD5, 0x06, 0x95, 0x92, 0xED, 0x91, 0xD7,
  0xA8, 0x9E, 0x2C, 0xDC, 0x4A, 0x93, 0x5B, 0xF9 };

gapSmpKeys_t gSmpKeys = {
  .cLtkSize = mcEncryptionKeySize_c,
  .aLtk = (void *)smpLtk,
  .aIrk = (void *)smpIrk,
  .aCsrk = (void *)smpCsrk,
  .aRand = (void *)smpRand,
  .cRandSize = gcSmpMaxRandSize_c,
  .ediv = smpEdiv,
};

/* Device Security Requirements */
static const gapSecurityRequirements_t        masterSecurity = gGapDefaultSecurityRequirements_d;

#ifdef HEART_RATE_SERVICE_EN
static const gapServiceSecurityRequirements_t serviceSecurity[3] = {
  {
    .requirements = {
      .securityModeLevel = gSecurityMode_1_Level_3_c,
      .authorization = FALSE,
      .minimumEncryptionKeySize = gDefaultEncryptionKeySize_d
    },
    .serviceHandle = service_heart_rate
  },
#else
static const gapServiceSecurityRequirements_t serviceSecurity[2] = {
#endif
  {
    .requirements = {
      .securityModeLevel = gSecurityMode_1_Level_3_c,
      .authorization = FALSE,
      .minimumEncryptionKeySize = gDefaultEncryptionKeySize_d
    },
    .serviceHandle = service_battery
  },
  {
    .requirements = {
      .securityModeLevel = gSecurityMode_1_Level_3_c,
      .authorization = FALSE,
      .minimumEncryptionKeySize = gDefaultEncryptionKeySize_d
    },
    .serviceHandle = service_device_info
  }
};

gapDeviceSecurityRequirements_t deviceSecurityRequirements = {
  .pMasterSecurityRequirements    = (void *)&masterSecurity,
  .cNumServices                   = 3,
  .aServiceSecurityRequirements   = (void *)serviceSecurity
};

#define mHeartRateLowerLimit_c          (40) /* Heart beat lower limit, 8-bit value */
#define mHeartRateUpperLimit_c          (201) /* Heart beat upper limit, 8-bit value */
#define mHeartRateRange_c               (mHeartRateUpperLimit_c - mHeartRateLowerLimit_c) /* Range = [ADC16_HB_LOWER_LIMIT .. ADC16_HB_LOWER_LIMIT + ADC16_HB_DYNAMIC_RANGE] */
#define mHeartRateReportInterval_c      (1)        /* heart rate report interval in seconds  */
#define mBatteryLevelReportInterval_c   (1)        /* battery level report interval in seconds  */


typedef enum
{
#if gBondingSupported_d
  fastWhiteListAdvState_c,
#endif
  fastAdvState_c,
  slowAdvState_c
}advType_t;

typedef struct advState_tag
{
  bool_t      advOn;  // Состояние оповещений
  advType_t   advType;
} advState_t;


/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* Host Stack Data*/
static bleDeviceAddress_t         maBleDeviceAddress;
static deviceId_t                 mPeerDeviceId = gInvalidDeviceId_c;

/* Adv Parmeters */
static advState_t                 mAdvState;
static uint32_t                   mAdvTimeout;
static gapAdvertisingParameters_t advParams = gGapDefaultAdvertisingParameters_d;

#if gBondingSupported_d
static bleDeviceAddress_t         maPeerDeviceAddress;
static uint8_t                    mcBondedDevices = 0;
static bleAddressType_t           mPeerDeviceAddressType;
#endif

/* Service Data*/
static basConfig_t                basServiceConfig = { service_battery, 0 };
static disConfig_t                disServiceConfig = { service_device_info };
static hrsUserData_t              hrsUserData;

#ifdef HEART_RATE_SERVICE_EN
// Внутренняя структура используема сервисом HEART_RATE
static hrsConfig_t                hrsServiceConfig = { service_heart_rate, TRUE, TRUE, TRUE, gHrs_BodySensorLocChest_c,&hrsUserData };
// Массив индексов значений характеристик запись в которые вызовет генерацию событий gEvtAttributeWritten_c или gEvtAttributeWrittenWithoutResponse_c
static uint16_t                   cpHandles[3] = { value_hr_ctrl_point, value_manuf_name, value_model_no };
#endif

#ifdef K66BLEZ_PROFILE
// Массив индексов значения характеристик запись в которые вызовет генерацию событий gEvtAttributeWritten_c или gEvtAttributeWrittenWithoutResponse_c
static uint16_t                   intercepted_write_hndls[4] = { value_k66wrdata_w_resp, value_k66wrdata, value_k66rddata, value_k66_cmd_write };
static uint16_t                   intercepted_read_hndls[4] = {  value_k66wrdata_w_resp, value_k66wrdata, value_k66rddata, value_k66_cmd_read };


#endif




/* Application specific data*/
static bool_t                     mToggle16BitHeartRate = FALSE;
static bool_t                     mContactStatus = TRUE;
static tmrTimerID_t               mAdvTimerId;
static tmrTimerID_t               mMeasurementTimerId;
static tmrTimerID_t               mBatteryMeasurementTimerId;


/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent);
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent);
static void BleApp_GattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent);
static void BleApp_Config();

/* Timer Callbacks */
static void AdvertisingTimerCallback(void *);
static void TimerMeasurementCallback(void *);
static void BatteryMeasurementTimerCallback(void *);

static void BleApp_SetAdvertisingParameters(void);
static void GetRSSI(void);


/*------------------------------------------------------------------------------
  Старт процесса оповещения

  Вызывается на старте приложения и каждый раз после события разрыва соединения


 \param void
 ------------------------------------------------------------------------------*/
void BleApp_Start(void)
{

  DEBUG_PRINT("BleApp_Start\r\n");

  Restart_perfomance_test();
  /* Device is not connected and not advertising*/
  if (!mAdvState.advOn)
  {
#if gBondingSupported_d
    if (mcBondedDevices > 0)
    {
      mAdvState.advType = fastWhiteListAdvState_c;
    }
    else
    {
#endif
      mAdvState.advType = fastAdvState_c;
#if gBondingSupported_d
    }
#endif
    BleApp_SetAdvertisingParameters();
  }

#if (cPWR_UsePowerDownMode)
  PWR_ChangeDeepSleepMode(1); /* MCU=LLS3, LL=DSM, wakeup on GPIO/LL */
  PWR_AllowDeviceToSleep();
#endif
}


/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback(gapGenericEvent_t *pGenericEvent)
{
  //SEGGER_RTT_printf(0, "BleApp_GenericCallback\r\n");

  switch (pGenericEvent->eventType)
  {
  case gInitializationComplete_c:
    {
      DEBUG_PRINT("BleApp_GenericCallback - gInitializationComplete_c\r\n");
      BleApp_Config();
    }
    break;

  case gPublicAddressRead_c:
    {
      DEBUG_PRINT("BleApp_GenericCallback - gPublicAddressRead_c\r\n");
      /* Use address read from the controller */
      FLib_MemCpyReverseOrder(maBleDeviceAddress, pGenericEvent->eventData.aAddress, sizeof(bleDeviceAddress_t));
    }
    break;

  case gAdvertisingDataSetupComplete_c:
    {
      DEBUG_PRINT("BleApp_GenericCallback - gAdvertisingDataSetupComplete_c\r\n");
    }
    break;

  case gAdvertisingParametersSetupComplete_c:
    {
      DEBUG_PRINT("BleApp_GenericCallback - gAdvertisingParametersSetupComplete_c\r\n");
      App_StartAdvertising(BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
    }
    break;

  case gInternalError_c:
    {
      DEBUG_PRINT("BleApp_GenericCallback - gInternalError_c\r\n");
      panic(0, 0, 0, 0);
    }
    break;

  case gAdvertisingSetupFailed_c:
    DEBUG_PRINT("BleApp_GenericCallback - gAdvertisingSetupFailed_c\r\n");
    break;
  case gWhiteListSizeRead_c:
    DEBUG_PRINT("BleApp_GenericCallback - gWhiteListSizeRead_c\r\n");
    break;
  case gDeviceAddedToWhiteList_c:
    DEBUG_PRINT("BleApp_GenericCallback - gDeviceAddedToWhiteList_c\r\n");
    break;
  case gDeviceRemovedFromWhiteList_c:
    DEBUG_PRINT("BleApp_GenericCallback - gDeviceRemovedFromWhiteList_c\r\n");
    break;
  case gWhiteListCleared_c:
    DEBUG_PRINT("BleApp_GenericCallback - gWhiteListCleared_c\r\n");
    break;
  case gRandomAddressReady_c:
    DEBUG_PRINT("BleApp_GenericCallback - gRandomAddressReady_c\r\n");
    break;
  case gCreateConnectionCanceled_c:
    DEBUG_PRINT("BleApp_GenericCallback - gCreateConnectionCanceled_c\r\n");
    break;
  case gAdvTxPowerLevelRead_c:
    DEBUG_PRINT("BleApp_GenericCallback - gAdvTxPowerLevelRead_c\r\n");
    break;
  case gPrivateResolvableAddressVerified_c:
    DEBUG_PRINT("BleApp_GenericCallback - gPrivateResolvableAddressVerified_c\r\n");
    break;
  case gRandomAddressSet_c:
    DEBUG_PRINT("BleApp_GenericCallback - gRandomAddressSet_c\r\n");
    break;

  default:
    DEBUG_PRINT("BleApp_GenericCallback - default\r\n");
    break;
  }
}

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BleApp_Config()
{
  DEBUG_PRINT("BleApp_Config\r\n");
  /* Read public address from controller */

  DEBUG_PRINT("  Gap_ReadPublicDeviceAddress\r\n");
  Gap_ReadPublicDeviceAddress();

#ifdef HEART_RATE_SERVICE_EN
  /* Register for callbacks*/
  DEBUG_PRINT("  GattServer_RegisterHandlesForWriteNotifications\r\n");
  GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);
#endif

#ifdef K66BLEZ_PROFILE
  DEBUG_PRINT("  GattServer_RegisterHandlesForWriteNotifications for K66BLEZ_PROFILE\r\n");
  GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(intercepted_write_hndls), intercepted_write_hndls);
  GattServer_RegisterHandlesForReadNotifications(NumberOfElements(intercepted_read_hndls), intercepted_read_hndls);
#endif



  DEBUG_PRINT("  App_RegisterGattServerCallback(BleApp_GattServerCallback)\r\n");
  App_RegisterGattServerCallback(BleApp_GattServerCallback);

  /* Register security requirements */
#if gUseServiceSecurity_d
  DEBUG_PRINT("  Gap_RegisterDeviceSecurityRequirements\r\n");
  Gap_RegisterDeviceSecurityRequirements(&deviceSecurityRequirements);
#endif

  /* Set local passkey */
#if gBondingSupported_d
  DEBUG_PRINT("  Gap_SetLocalPasskey\r\n");
  Gap_SetLocalPasskey(gPasskeyValue_c);
#endif

  /* Setup Advertising and scanning data */
  DEBUG_PRINT("  Gap_SetAdvertisingData\r\n");
  Gap_SetAdvertisingData(&gAppAdvertisingData, &gAppScanRspData);

  /* Populate White List if bonding is supported */
#if gBondingSupported_d
  bleDeviceAddress_t aBondedDevAdds[gcGapMaximumBondedDevices_d];
  DEBUG_PRINT("  Gap_GetBondedStaticAddresses\r\n");
  bleResult_t result = Gap_GetBondedStaticAddresses(aBondedDevAdds, gcGapMaximumBondedDevices_d, &mcBondedDevices);

  if (gBleSuccess_c == result && mcBondedDevices > 0)
  {
    for (uint8_t i = 0; i < mcBondedDevices; i++)
    {
      DEBUG_PRINT("  Gap_AddDeviceToWhiteList\r\n");
      Gap_AddDeviceToWhiteList(gBleAddrTypePublic_c, aBondedDevAdds[i]);
    }
  }
#endif

  mAdvState.advOn = FALSE;

#ifdef HEART_RATE_SERVICE_EN
  /* Start services */
  hrsServiceConfig.sensorContactDetected = mContactStatus;
  #if gHrs_EnableRRIntervalMeasurements_d
  hrsServiceConfig.pUserData->pStoredRrIntervals = MEM_BufferAlloc(sizeof(uint16_t) * gHrs_NumOfRRIntervalsRecorded_c);
  #endif
  DEBUG_PRINT("  Hrs_Start\r\n");
  Hrs_Start(&hrsServiceConfig);
#endif

  basServiceConfig.batteryLevel = 0; //  BOARD_GetBatteryLevel(); Передавать будем RSSI подключенного клиента
  DEBUG_PRINT("  Bas_Start\r\n");
  Bas_Start(&basServiceConfig);

  /* Allocate application timers */
  DEBUG_PRINT("  TMR_AllocateTimer mAdvTimerId\r\n");
  mAdvTimerId = TMR_AllocateTimer();
  DEBUG_PRINT("  TMR_AllocateTimer mMeasurementTimerId\r\n");
  mMeasurementTimerId = TMR_AllocateTimer();
  DEBUG_PRINT("  TMR_AllocateTimer mBatteryMeasurementTimerId\r\n");
  mBatteryMeasurementTimerId = TMR_AllocateTimer();

#if (cPWR_UsePowerDownMode)
  PWR_ChangeDeepSleepMode(3); /* MCU=LLS3, LL=IDLE, wakeup on GPIO/LL */
  PWR_AllowDeviceToSleep();
#endif
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will satrt after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_SetAdvertisingParameters(void)
{

  switch (mAdvState.advType)
  {
#if gBondingSupported_d
  case fastWhiteListAdvState_c:
    {
      DEBUG_PRINT("BleApp_SetAdvertisingParameters - fastWhiteListAdvState_c\r\n");
      advParams.minInterval = gFastConnMinAdvInterval_c;
      advParams.maxInterval = gFastConnMaxAdvInterval_c;
      advParams.filterPolicy = (gapAdvertisingFilterPolicyFlags_t)(gProcessConnWhiteListFlag_c | gProcessScanWhiteListFlag_c);
      mAdvTimeout = gFastConnWhiteListAdvTime_c;
    }
    break;
#endif
  case fastAdvState_c:
    {
      DEBUG_PRINT("BleApp_SetAdvertisingParameters - fastAdvState_c\r\n");
      advParams.minInterval = gFastConnMinAdvInterval_c;
      advParams.maxInterval = gFastConnMaxAdvInterval_c;
      advParams.filterPolicy = gProcessAll_c;
      mAdvTimeout = gFastConnAdvTime_c - gFastConnWhiteListAdvTime_c;
    }
    break;

  case slowAdvState_c:
    {
      DEBUG_PRINT("BleApp_SetAdvertisingParameters - slowAdvState_c\r\n");
      advParams.minInterval = gReducedPowerMinAdvInterval_c;
      advParams.maxInterval = gReducedPowerMinAdvInterval_c;
      advParams.filterPolicy = gProcessAll_c;
      mAdvTimeout = gReducedPowerAdvTime_c;
    }
    break;
  default:
    DEBUG_PRINT("BleApp_SetAdvertisingParameters - default\r\n");
    break;
  }

  /* Set advertising parameters */
  DEBUG_PRINT("Gap_SetAdvertisingParameters.\r\n");
  Gap_SetAdvertisingParameters(&advParams);
}

/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent)
{
  //SEGGER_RTT_printf(0, "BleApp_AdvertisingCallback\r\n");

  switch (pAdvertisingEvent->eventType)
  {
  case gAdvertisingStateChanged_c:
    {
      // Переключился режим передачи объявлений
      DEBUG_PRINT("BleApp_AdvertisingCallback - gAdvertisingStateChanged_c\r\n");


      mAdvState.advOn = !mAdvState.advOn;

#if (cPWR_UsePowerDownMode)
      if (!mAdvState.advOn)
      {
        PWR_ChangeDeepSleepMode(3);
        PWR_AllowDeviceToSleep();
      }
      else
      {
        PWR_ChangeDeepSleepMode(1);

  #ifdef ENABLE_SLOW_ADVERTISING
        // Задаем через некоторое время поменять частоту передачи объявлений в функции AdvertisingTimerCallback
        TMR_StartLowPowerTimer(mAdvTimerId, gTmrLowPowerSecondTimer_c, TmrSeconds(mAdvTimeout), AdvertisingTimerCallback, NULL);
  #endif
      }
#else

      if (mAdvState.advOn)
      {
  #ifdef ENABLE_SLOW_ADVERTISING
        // Задаем через некоторое время поменять частоту передачи объявлений в функции AdvertisingTimerCallback
        TMR_StartLowPowerTimer(mAdvTimerId, gTmrLowPowerSecondTimer_c, TmrSeconds(mAdvTimeout), AdvertisingTimerCallback, NULL);
  #endif
      }
#endif
    }
    break;

  case gAdvertisingCommandFailed_c:
    {
      DEBUG_PRINT("BleApp_AdvertisingCallback - gAdvertisingCommandFailed_c\r\n");
      panic(0, 0, 0, 0);
    }
    break;

  default:
    DEBUG_PRINT("BleApp_AdvertisingCallback - default\r\n");
    break;
  }
}

/*! *********************************************************************************
* \brief        Handles BLE Connection callback from host stack.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pConnectionEvent    Pointer to gapConnectionEvent_t.
********************************************************************************** */
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent)
{
  //SEGGER_RTT_printf(0, "BleApp_ConnectionCallback\r\n");
  switch (pConnectionEvent->eventType)
  {
  case gConnEvtConnected_c:
    {
      DEBUG_PRINT_ARG("BleApp_ConnectionCallback - gConnEvtConnected_c, peerDeviceId=%d\r\n", peerDeviceId);

      mPeerDeviceId = peerDeviceId;

      /* Advertising stops when connected */
      mAdvState.advOn = FALSE;

#if gBondingSupported_d
      /* Copy peer device address information */
      mPeerDeviceAddressType = pConnectionEvent->eventData.connectedEvent.peerAddressType;
      FLib_MemCpy(maPeerDeviceAddress, pConnectionEvent->eventData.connectedEvent.peerAddress, sizeof(bleDeviceAddress_t));
#endif
#if gUseServiceSecurity_d
      {
        bool_t isBonded = FALSE;

        if (gBleSuccess_c == Gap_CheckIfBonded(peerDeviceId, &isBonded) && FALSE == isBonded)
        {
          DEBUG_PRINT("Gap_SendSlaveSecurityRequest\r\n");
          Gap_SendSlaveSecurityRequest(peerDeviceId, TRUE, gSecurityMode_1_Level_3_c);
        }
      }
#endif
      /* Subscribe client*/
      Bas_Subscribe(peerDeviceId);
#ifdef HEART_RATE_SERVICE_EN
      Hrs_Subscribe(peerDeviceId);
#endif

/* Fast Connection Parameters used for Power Vector Notifications */
#define gFastConnMinInterval_c          6 // Minimum connection interval (7.5 ms). При установке меньшего значения PC возвращается к низкой скорости
#define gFastConnMaxInterval_c          6 // Minimum connection interval (7.5 ms)
#define gFastConnSlaveLatency_c         0
#define gFastConnTimeoutMultiplier_c    0x03E8
      L2ca_UpdateConnectionParameters(peerDeviceId, gFastConnMinInterval_c, gFastConnMaxInterval_c, gFastConnSlaveLatency_c, gFastConnTimeoutMultiplier_c, gcConnectionEventMinDefault_c, gcConnectionEventMaxDefault_c);

      /* Stop Advertising Timer*/
      mAdvState.advOn = FALSE;
      DEBUG_PRINT("TMR_StopTimer(mAdvTimerId)\r\n");
      TMR_StopTimer(mAdvTimerId);

      /* Start measurements */
      DEBUG_PRINT("TMR_StartLowPowerTimer(mMeasurementTimerId...\r\n");
#ifdef HEART_RATE_SERVICE_EN
      TMR_StartLowPowerTimer(mMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c, TmrSeconds(mHeartRateReportInterval_c), TimerMeasurementCallback, NULL);
#endif

      /* Start battery measurements */
      DEBUG_PRINT("TMR_StartLowPowerTimer(mBatteryMeasurementTimerId...\r\n");
      TMR_StartLowPowerTimer(mBatteryMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c, TmrSeconds(mBatteryLevelReportInterval_c), BatteryMeasurementTimerCallback, NULL);
    }
    break;

  case gConnEvtDisconnected_c:
    {
      DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtDisconnected_c\r\n");

      /* Unsubscribe client */
      Bas_Unsubscribe();
#ifdef HEART_RATE_SERVICE_EN
      Hrs_Unsubscribe();
#endif
      mPeerDeviceId = gInvalidDeviceId_c;

      TMR_StopTimer(mMeasurementTimerId);
      TMR_StopTimer(mBatteryMeasurementTimerId);

#if (cPWR_UsePowerDownMode)

      /* Go to sleep */
      PWR_ChangeDeepSleepMode(3); /* MCU=LLS3, LL=IDLE, wakeup on swithes/LL */
      PWR_AllowDeviceToSleep();
#else
      if (pConnectionEvent->eventData.disconnectedEvent.reason == gHciConnectionTimeout_c)
      {
        /* Link loss detected*/
        BleApp_Start();
      }
      else
      {
        /* Connection was terminated by peer or application */
        BleApp_Start();
      }
#endif
    }
    break;

#if gBondingSupported_d
  case gConnEvtKeysReceived_c:
    {
      DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtKeysReceived_c\r\n");

      /* Copy peer device address information when IRK is used */
      if (pConnectionEvent->eventData.keysReceivedEvent.pKeys->aIrk != NULL)
      {
        mPeerDeviceAddressType = pConnectionEvent->eventData.keysReceivedEvent.pKeys->addressType;
        FLib_MemCpy(maPeerDeviceAddress, pConnectionEvent->eventData.keysReceivedEvent.pKeys->aAddress, sizeof(bleDeviceAddress_t));
      }
    }
    break;

  case gConnEvtPairingComplete_c:
    {
      DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtPairingComplete_c\r\n");

      if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful && pConnectionEvent->eventData.pairingCompleteEvent.pairingCompleteData.withBonding)
      {
        /* If a bond is created, write device address in controller’s White List */
        Gap_AddDeviceToWhiteList(mPeerDeviceAddressType, maPeerDeviceAddress);
      }
    }
    break;

  case gConnEvtPairingRequest_c:
    {
      DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtPairingRequest_c\r\n");

      gPairingParameters.centralKeys = pConnectionEvent->eventData.pairingEvent.centralKeys;
      Gap_AcceptPairingRequest(peerDeviceId, &gPairingParameters);
    }
    break;

  case gConnEvtKeyExchangeRequest_c:
    {
      gapSmpKeys_t sentSmpKeys = gSmpKeys;

      DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtKeyExchangeRequest_c\r\n");

      if (!(pConnectionEvent->eventData.keyExchangeRequestEvent.requestedKeys & gLtk_c))
      {
        sentSmpKeys.aLtk = NULL;
        /* When the LTK is NULL EDIV and Rand are not sent and will be ignored. */
      }

      if (!(pConnectionEvent->eventData.keyExchangeRequestEvent.requestedKeys & gIrk_c))
      {
        sentSmpKeys.aIrk = NULL;
        /* When the IRK is NULL the Address and Address Type are not sent and will be ignored. */
      }

      if (!(pConnectionEvent->eventData.keyExchangeRequestEvent.requestedKeys & gCsrk_c))
      {
        sentSmpKeys.aCsrk = NULL;
      }

      Gap_SendSmpKeys(peerDeviceId, &sentSmpKeys);
      break;
    }

  case gConnEvtLongTermKeyRequest_c:
    {
      DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtLongTermKeyRequest_c\r\n");

      if (pConnectionEvent->eventData.longTermKeyRequestEvent.ediv == gSmpKeys.ediv &&
          pConnectionEvent->eventData.longTermKeyRequestEvent.randSize == gSmpKeys.cRandSize)
      {
        /* EDIV and RAND both matched */
        Gap_ProvideLongTermKey(peerDeviceId, gSmpKeys.aLtk, gSmpKeys.cLtkSize);
      }
      else
      /* EDIV or RAND size did not match */
      {
        Gap_DenyLongTermKey(peerDeviceId);
      }
    }
    break;
#endif


  case gConnEvtSlaveSecurityRequest_c:
    DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtSlaveSecurityRequest_c\r\n");
    break;
  case gConnEvtPairingResponse_c:
    DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtPairingResponse_c\r\n");
    break;
  case gConnEvtAuthenticationRejected_c:
    DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtAuthenticationRejected_c\r\n");
    break;
  case gConnEvtPasskeyRequest_c:
    DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtPasskeyRequest_c\r\n");
    break;
  case gConnEvtOobRequest_c:
    DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtOobRequest_c\r\n");
    break;
  case gConnEvtPasskeyDisplay_c:
    DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtPasskeyDisplay_c\r\n");
    break;
  case gConnEvtEncryptionChanged_c:
    DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtEncryptionChanged_c\r\n");
    break;
  case gConnEvtRssiRead_c:
    DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtRssiRead_c\r\n");
    basServiceConfig.batteryLevel = pConnectionEvent->eventData.rssi_dBm;

    break;
  case gConnEvtTxPowerLevelRead_c:
    DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtTxPowerLevelRead_c\r\n");
    break;
  case gConnEvtPowerReadFailure_c:
    DEBUG_PRINT("BleApp_ConnectionCallback - gConnEvtPowerReadFailure_c\r\n");
    break;

  default:
    DEBUG_PRINT("BleApp_ConnectionCallback - default\r\n");
    break;
  }
}

/*! *********************************************************************************
* \brief        Handles GATT server callback from host stack.
*
* \param[in]    deviceId        Peer device ID.
* \param[in]    pServerEvent    Pointer to gattServerEvent_t.
********************************************************************************** */
static void BleApp_GattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent)
{
  uint16_t handle;
  uint8_t status;

  //SEGGER_RTT_printf(0, "BleApp_GattServerCallback\r\n");

  switch (pServerEvent->eventType)
  {
  case gEvtAttributeWritten_c:
    {
      DEBUG_PRINT("BleApp_GattServerCallback - gEvtAttributeWritten_c\r\n");

      handle = pServerEvent->eventData.attributeWrittenEvent.handle;
      status = gAttErrCodeNoError_c;
#ifdef HEART_RATE_SERVICE_EN
      if (handle == value_hr_ctrl_point)
      {
        status = Hrs_ControlPointHandler(&hrsUserData, pServerEvent->eventData.attributeWrittenEvent.aValue[0]);
      }
#endif
#ifdef K66BLEZ_PROFILE
      if (handle == value_k66wrdata_w_resp )
      {
        status = PerfomanceTest_write_test(handle, pServerEvent->eventData.attributeWrittenEvent.aValue, pServerEvent->eventData.attributeWrittenEvent.cValueLength);
      }
      if (handle == value_k66rddata)
      {
        status = PerfomanceTest_read_test_init(deviceId, handle, pServerEvent->eventData.attributeWrittenEvent.aValue, pServerEvent->eventData.attributeWrittenEvent.cValueLength);
      }
      if (handle == value_k66_cmd_write)
      {
        status = CommandService_write(deviceId, handle, pServerEvent->eventData.attributeWrittenEvent.aValue, pServerEvent->eventData.attributeWrittenEvent.cValueLength);
      }
#endif
      GattServer_SendAttributeWrittenStatus(deviceId, handle, status);
    }
    break;


  case gEvtAttributeWrittenWithoutResponse_c:
    {
      DEBUG_PRINT("BleApp_GattServerCallback - gEvtAttributeWrittenWithoutResponse_c\r\n");
#ifdef K66BLEZ_PROFILE
      handle = pServerEvent->eventData.attributeWrittenEvent.handle;
      status = gAttErrCodeNoError_c;
      if (handle == value_k66wrdata)
      {
        status = PerfomanceTest_write_test(handle, pServerEvent->eventData.attributeWrittenEvent.aValue, pServerEvent->eventData.attributeWrittenEvent.cValueLength);
      }
#endif
    }
    break;


  case gEvtMtuChanged_c:
    DEBUG_PRINT("BleApp_GattServerCallback - gEvtMtuChanged_c\r\n");
    break;
  case gEvtHandleValueConfirmation_c:
    DEBUG_PRINT("BleApp_GattServerCallback - gEvtHandleValueConfirmation_c\r\n");
    break;
  case gEvtCharacteristicCccdWritten_c:
    DEBUG_PRINT_ARG("BleApp_GattServerCallback - gEvtCharacteristicCccdWritten_c %04X = %08X\r\n", pServerEvent->eventData.charCccdWrittenEvent.handle, (uint32_t)pServerEvent->eventData.charCccdWrittenEvent.newCccd);
    break;
  case gEvtError_c:
    DEBUG_PRINT("BleApp_GattServerCallback - gEvtError_c\r\n");
    break;
  case gEvtLongCharacteristicWritten_c:
    DEBUG_PRINT("BleApp_GattServerCallback - gEvtLongCharacteristicWritten_c\r\n");
    break;
  case gEvtAttributeRead_c:
    // Событие чтения подписанного на генерацию этого события аттрибута
    // Сначала происходит это событие и после него производится реальное чтение
    DEBUG_PRINT("BleApp_GattServerCallback - gEvtAttributeRead_c\r\n");
#ifdef K66BLEZ_PROFILE
    handle = pServerEvent->eventData.attributeWrittenEvent.handle;
    if (handle == value_k66wrdata_w_resp)
    {
      Restart_perfomance_test();
      status = gAttErrCodeNoError_c;
      GattServer_SendAttributeReadStatus(deviceId, handle, status);
    }
    if (handle == value_k66wrdata)
    {
      Restart_perfomance_test();
      status = gAttErrCodeNoError_c;
      GattServer_SendAttributeReadStatus(deviceId, handle, status);
    }
    if (handle == value_k66rddata)
    {
      PerfomanceTest_read_test();
      status = gAttErrCodeNoError_c;
      GattServer_SendAttributeReadStatus(deviceId, handle, status);
    }
    if (handle == value_k66_cmd_read)
    {
      CommandService_read();
      status = gAttErrCodeNoError_c;
      GattServer_SendAttributeReadStatus(deviceId, handle, status);
    }

#endif
    break;

  default:
    DEBUG_PRINT("BleApp_GattServerCallback - default\r\n");
    break;
  }
}

/*! *********************************************************************************
* \brief        Handles advertising timer callback.
*
* \param[in]    pParam        Calback parameters.
********************************************************************************** */
static void AdvertisingTimerCallback(void *pParam)
{
  DEBUG_PRINT("AdvertisingTimerCallback\r\n");

  DEBUG_PRINT("Gap_StopAdvertising\r\n");
  Gap_StopAdvertising(); // Останавливаем оповещение чтобы инициализировать новые параметры оповещения
  switch (mAdvState.advType)
  {
#if gBondingSupported_d
  case fastWhiteListAdvState_c:
    {
      mAdvState.advType = fastAdvState_c;
    }
    break;
#endif
  case fastAdvState_c:
    {
      mAdvState.advType = slowAdvState_c;
    }
    break;

  default:
    break;
  }
  BleApp_SetAdvertisingParameters();
}

/*! *********************************************************************************
* \brief        Handles measurement timer callback.
*
* \param[in]    pParam        Calback parameters.
********************************************************************************** */
static void TimerMeasurementCallback(void *pParam)
{


#ifdef HEART_RATE_SERVICE_EN
  uint16_t hr = rand();
  hr = (hr * mHeartRateRange_c) >> 12;

  #if gHrs_EnableRRIntervalMeasurements_d
  Hrs_RecordRRInterval(&hrsUserData, (hr & 0x0F));
  Hrs_RecordRRInterval(&hrsUserData, (hr & 0xF0));
  #endif

  if (mToggle16BitHeartRate)
  {
    Hrs_RecordHeartRateMeasurement(service_heart_rate, 0x0100 + (hr & 0xFF), &hrsUserData);
  }
  else
  {
    Hrs_RecordHeartRateMeasurement(service_heart_rate, mHeartRateLowerLimit_c + hr, &hrsUserData);
  }

  Hrs_AddExpendedEnergy(&hrsUserData, 100);
#endif

#if (cPWR_UsePowerDownMode)
  PWR_SetDeepSleepTimeInMs(900);
  PWR_ChangeDeepSleepMode(6);
  PWR_AllowDeviceToSleep();
#endif
}

/*! *********************************************************************************
* \brief        Handles battery measurement timer callback.
*
* \param[in]    pParam        Calback parameters.
********************************************************************************** */
static void BatteryMeasurementTimerCallback(void *pParam)
{
  GetRSSI(); // Вместо уровня заряду будем передавать RSSI подключенного клиента

  //basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
  Bas_RecordBatteryMeasurement(basServiceConfig.serviceHandle, basServiceConfig.batteryLevel);
}

/*------------------------------------------------------------------------------
  Получить значение RSSI для подключенного клиента

 ------------------------------------------------------------------------------*/
static void GetRSSI(void)
{
  Gap_ReadRssi(mPeerDeviceId);
}


