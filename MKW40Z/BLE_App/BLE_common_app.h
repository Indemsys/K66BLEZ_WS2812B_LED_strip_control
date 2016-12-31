#ifndef _APPL_MAIN_H_
  #define _APPL_MAIN_H_

  #include "EmbeddedTypes.h"
  #include "fsl_os_abstraction.h"
  #include "l2ca_cb_interface.h"


void App_StartAdvertising(gapAdvertisingCallback_t    advertisingCallback, gapConnectionCallback_t     connectionCallback);
void App_StartScanning(gapScanningParameters_t *pScanningParameters, gapScanningCallback_t       scanningCallback);
void App_RegisterGattClientNotificationCallback(gattClientNotificationCallback_t  callback);
void App_RegisterGattClientIndicationCallback(gattClientIndicationCallback_t  callback);
void App_RegisterGattServerCallback(gattServerCallback_t  serverCallback);
void App_RegisterGattClientProcedureCallback(gattClientProcedureCallback_t  callback);
void App_RegisterLePsm(uint16_t lePsm, l2caLeCbDataCallback_t pCallback, l2caLeCbControlCallback_t   pCtrlCallback, uint16_t lePsmMtu);
void App_RegisterL2caControlCallback(l2caControlCallback_t pCallback);


/*------------------------------------------------------------------------------
 \brief  Performs full initialization of the BLE stack.

 \param[in] genericCallback  Callback used by the Host Stack to propagate GAP generic
 events to the application.

 \return  gBleSuccess_c or error.

 \remarks The gInitializationComplete_c generic GAP event is triggered on completion.
 ------------------------------------------------------------------------------*/
bleResult_t Ble_Initialize(gapGenericCallback_t gapGenericCallback);


#endif /* _APPL_MAIN_H_ */
