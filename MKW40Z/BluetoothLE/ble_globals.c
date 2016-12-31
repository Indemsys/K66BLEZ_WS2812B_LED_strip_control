#include "ble_general.h"
#include "att_errors.h"

#define gPrepareWriteQueueSize_c        30

uint8_t gcGattDbMaxPrepareWriteOperationsInQueue_c = gPrepareWriteQueueSize_c;
attPrepareWriteRequestParams_t gPrepareWriteQueues[gcGattDbMaxPrepareWriteClients_c][gPrepareWriteQueueSize_c];


