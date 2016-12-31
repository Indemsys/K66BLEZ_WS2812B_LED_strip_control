/*! *********************************************************************************
 * \addtogroup GATT_DB
 * @{
 ********************************************************************************** */
/*!
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* All rights reserved.
*
* \file gatt_database.c
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

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "gatt_database.h"
#include "gatt_types.h"
#include "gap_types.h"
#include "board.h"

#if !gGattDbDynamic_d
/*! Macros and X-Macros */
  #include "gatt_db_macros.h"
  #include "gatt_db_x_macros.def"
  #include "gatt_db_handles.h"
#endif



/************************************************************************************
*************************************************************************************
* X-Macro expansions - enums, structs and memory allocations
*************************************************************************************
************************************************************************************/
#if !gGattDbDynamic_d

/*! Allocate custom 128-bit UUIDs, if any */
  #include "gatt_uuid_def_x.def"

/*! Allocate the arrays for Attribute Values */
  #include "gatt_alloc_x.def"

/*! Declare the Attribute database */
gattDbAttribute_t static_gattDatabase[] = {
  #include "gatt_decl_x.def"
};

/*

Воссозданная после расшифровки директив прерпроцессора база данных

enum gattDbHandle_tag
{
  service_gatt               = (1),
  char_service_changed       = (2),
  value_service_changed      = (3),
  cccd_service_changed       = (4),

  service_gap                = (6),
  char_device_name           = (7),
  value_device_name          = (8),
  char_appearance            = (9),
  value_appearance           = (10),
  char_ppcp                  = (11),
  value_ppcp                 = (12),

  service_heart_rate         = (14),
  char_hr_measurement        = (15),
  value_hr_measurement       = (16),
  cccd_hr_measurement        = (17),
  char_body_sensor_location  = (18),
  value_body_sensor_location = (19),
  char_hr_ctrl_point         = (20),
  value_hr_ctrl_point        = (21),

  service_battery            = (23),
  char_battery_level         = (24),
  value_battery_level        = (25),
  desc_bat_level             = (26),
  cccd_battery_level         = (27),

  service_device_info        = (29),
  char_manuf_name            = (30),
  value_manuf_name           = (31),
  char_model_no              = (32),
  value_model_no             = (33),
  char_serial_no             = (34),
  value_serial_no            = (35),
  char_hw_rev                = (36),
  value_hw_rev               = (37),
  char_fw_rev                = (38),
  value_fw_rev               = (39),
  char_sw_rev                = (40),
  value_sw_rev               = (41),
  char_system_id             = (42),
  value_system_id            = (43),
  char_rcdl                  = (44),
  value_rcdl                 = (45),
};

#define LB(X)   ((uint8_t)(X & 0xFF))
#define UB(X)   ((uint8_t)((X >> 8)& 0xFF))

//------------------------ service_gatt ---------------------------------------------------------------------------------------------
uint8_t service_gatt_valueArray[2]                 = { LB(gBleSig_GenericAttributeProfile_d),UB(gBleSig_GenericAttributeProfile_d) };

uint8_t char_service_changed_valueArray[5]         = { ((gGattCharPropRead_c | gGattCharPropNotify_c)),LB(value_service_changed),UB(value_service_changed),LB(gBleSig_GattServiceChanged_d),UB(gBleSig_GattServiceChanged_d) };
uint8_t value_service_changed_valueArray[(4)]      = { 0x00, 0x00, 0x00, 0x00 };
uint8_t cccd_service_changed_valueArray[2]         = { ((uint8_t)((gCccdEmpty_c)&0xFF)),((uint8_t)(((gCccdEmpty_c) >> 8) & 0xFF)) };

//------------------------ service_gap ----------------------------------------------------------------------------------------------
uint8_t service_gap_valueArray[2]                  = { LB(gBleSig_GenericAccessProfile_d),UB(gBleSig_GenericAccessProfile_d)  };

uint8_t char_device_name_valueArray[5]             = { ((gGattCharPropRead_c)),((uint8_t)(((7 + 1)) & 0xFF)),((uint8_t)((((7 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A00)) & 0xFF)),((uint8_t)((((0x2A00)) >> 8) & 0xFF)) };
uint8_t value_device_name_valueArray[(11)]         = { "FSL_BLE_HRS" };

uint8_t char_appearance_valueArray[5]              = { ((gGattCharPropRead_c)),((uint8_t)(((9 + 1)) & 0xFF)),((uint8_t)((((9 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A01)) & 0xFF)),((uint8_t)((((0x2A01)) >> 8) & 0xFF)) };
uint8_t value_appearance_valueArray[(2)]           = { (gGenericHeartrateSensor_c & 0xFF),(gGenericHeartrateSensor_c >> 8) };

uint8_t char_ppcp_valueArray[5]                    = { ((gGattCharPropRead_c)),((uint8_t)(((11 + 1)) & 0xFF)),((uint8_t)((((11 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A04)) & 0xFF)),((uint8_t)((((0x2A04)) >> 8) & 0xFF)) };
uint8_t value_ppcp_valueArray[(8)]                 = { 0x0A, 0x00, 0x10, 0x00, 0x64, 0x00, 0xE2, 0x04 };

//------------------------ service_heart_rate ---------------------------------------------------------------------------------------
uint8_t service_heart_rate_valueArray[2]           = { LB(gBleSig_HeartRateService_d),UB(gBleSig_HeartRateService_d)  };

uint8_t char_hr_measurement_valueArray[5]          = { ((gGattCharPropNotify_c)),((uint8_t)(((15 + 1)) & 0xFF)),((uint8_t)((((15 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A37)) & 0xFF)),((uint8_t)((((0x2A37)) >> 8) & 0xFF)) };
uint8_t value_hr_measurement_valueArray[(22)]      = { 0x00, 0xB4 };
uint8_t cccd_hr_measurement_valueArray[2]          = { ((uint8_t)((gCccdEmpty_c)&0xFF)),((uint8_t)(((gCccdEmpty_c) >> 8) & 0xFF)) };

uint8_t char_body_sensor_location_valueArray[5]    = { ((gGattCharPropRead_c)),((uint8_t)(((18 + 1)) & 0xFF)),((uint8_t)((((18 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A38)) & 0xFF)),((uint8_t)((((0x2A38)) >> 8) & 0xFF)) };
uint8_t value_body_sensor_location_valueArray[(1)] = { 0x01 };

uint8_t char_hr_ctrl_point_valueArray[5]           = { ((gGattCharPropWrite_c)),((uint8_t)(((20 + 1)) & 0xFF)),((uint8_t)((((20 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A39)) & 0xFF)),((uint8_t)((((0x2A39)) >> 8) & 0xFF)) };
uint8_t value_hr_ctrl_point_valueArray[(1)]        = { 0x00 };

//------------------------ service_battery -----------------------------------------------------------------------------------------
uint8_t service_battery_valueArray[2]              = { LB(gBleSig_BatteryService_d),UB(gBleSig_BatteryService_d)  };

uint8_t char_battery_level_valueArray[5]           = { ((gGattCharPropNotify_c | gGattCharPropRead_c)),((uint8_t)(((24 + 1)) & 0xFF)),((uint8_t)((((24 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A19)) & 0xFF)),((uint8_t)((((0x2A19)) >> 8) & 0xFF)) };
uint8_t value_battery_level_valueArray[(1)]        = { 0x5A };
uint8_t desc_bat_level_valueArray[(7)]             = { 0x04, 0x00, 0xAD, 0x27, 0x01, 0x00, 0x00 };
uint8_t cccd_battery_level_valueArray[2]           = { ((uint8_t)((gCccdEmpty_c)&0xFF)),((uint8_t)(((gCccdEmpty_c) >> 8) & 0xFF)) };

//------------------------ service_device_info  ------------------------------------------------------------------------------------
uint8_t service_device_info_valueArray[2]          = { LB(gBleSig_DeviceInformationService_d),UB(gBleSig_DeviceInformationService_d)  };

uint8_t char_manuf_name_valueArray[5]              = { ((gGattCharPropRead_c)),((uint8_t)(((30 + 1)) & 0xFF)),((uint8_t)((((30 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A29)) & 0xFF)),((uint8_t)((((0x2A29)) >> 8) & 0xFF)) };
uint8_t value_manuf_name_valueArray[(9)]           = { "Freescale" };

uint8_t char_model_no_valueArray[5]                = { ((gGattCharPropRead_c)),((uint8_t)(((32 + 1)) & 0xFF)),((uint8_t)((((32 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A24)) & 0xFF)),((uint8_t)((((0x2A24)) >> 8) & 0xFF)) };
uint8_t value_model_no_valueArray[(8)]             = { "HRS Demo" };

uint8_t char_serial_no_valueArray[5]               = { ((gGattCharPropRead_c)),((uint8_t)(((34 + 1)) & 0xFF)),((uint8_t)((((34 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A25)) & 0xFF)),((uint8_t)((((0x2A25)) >> 8) & 0xFF)) };
uint8_t value_serial_no_valueArray[(7)]            = { "BLESN01" };

uint8_t char_hw_rev_valueArray[5]                  = { ((gGattCharPropRead_c)),((uint8_t)(((36 + 1)) & 0xFF)),((uint8_t)((((36 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A27)) & 0xFF)),((uint8_t)((((0x2A27)) >> 8) & 0xFF)) };
uint8_t value_hw_rev_valueArray[(sizeof("K66BLEZv1"))] = { "K66BLEZv1" };

uint8_t char_fw_rev_valueArray[5]                  = { ((gGattCharPropRead_c)),((uint8_t)(((38 + 1)) & 0xFF)),((uint8_t)((((38 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A26)) & 0xFF)),((uint8_t)((((0x2A26)) >> 8) & 0xFF)) };
uint8_t value_fw_rev_valueArray[(5)]               = { "1.1.1" };

uint8_t char_sw_rev_valueArray[5]                  = { ((gGattCharPropRead_c)),((uint8_t)(((40 + 1)) & 0xFF)),((uint8_t)((((40 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A28)) & 0xFF)),((uint8_t)((((0x2A28)) >> 8) & 0xFF)) };
uint8_t value_sw_rev_valueArray[(5)]               = { "1.1.3" };

uint8_t char_system_id_valueArray[5]               = { ((gGattCharPropRead_c)),((uint8_t)(((42 + 1)) & 0xFF)),((uint8_t)((((42 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A23)) & 0xFF)),((uint8_t)((((0x2A23)) >> 8) & 0xFF)) };
uint8_t value_system_id_valueArray[(8)]            = { 0x00, 0x00, 0x00, 0xFE, 0xFF, 0x9F, 0x04, 0x00 };

uint8_t char_rcdl_valueArray[5]                    = { ((gGattCharPropRead_c)),((uint8_t)(((44 + 1)) & 0xFF)),((uint8_t)((((44 + 1)) >> 8) & 0xFF)),((uint8_t)(((0x2A2A)) & 0xFF)),((uint8_t)((((0x2A2A)) >> 8) & 0xFF)) };
uint8_t value_rcdl_valueArray[(4)]                 = { 0x00, 0x00, 0x00, 0x00 };
//----------------------------------------------------------------------------------------------------------------------------------

gattDbAttribute_t static_gattDatabase[] = {
  { service_gatt               ,  gPermissionFlagReadable_c,                              0x2800, service_gatt_valueArray,              2,  gBleUuidType16_c, 0,  },
  { char_service_changed       ,  gPermissionFlagReadable_c,                              0x2803, char_service_changed_valueArray,      5,  gBleUuidType16_c, 0,  },
  { value_service_changed      ,  gPermissionNone_c,                                      0x2A05, value_service_changed_valueArray,     4,  gBleUuidType16_c, 0,  },
  { cccd_service_changed       ,  gPermissionFlagReadable_c | gPermissionFlagWritable_c,  0x2902, cccd_service_changed_valueArray,      2,  gBleUuidType16_c, 0,  },

  { service_gap                ,  gPermissionFlagReadable_c,                              0x2800, service_gap_valueArray,               2,  gBleUuidType16_c, 0,  },
  { char_device_name           ,  gPermissionFlagReadable_c,                              0x2803, char_device_name_valueArray,          5,  gBleUuidType16_c, 0,  },
  { value_device_name          ,  gPermissionFlagReadable_c,                              0x2A00, value_device_name_valueArray,         11, gBleUuidType16_c, 0,  },
  { char_appearance            ,  gPermissionFlagReadable_c,                              0x2803, char_appearance_valueArray,           5,  gBleUuidType16_c, 0,  },
  { value_appearance           ,  gPermissionFlagReadable_c,                              0x2A01, value_appearance_valueArray,          2,  gBleUuidType16_c, 0,  },
  { char_ppcp                  ,  gPermissionFlagReadable_c,                              0x2803, char_ppcp_valueArray,                 5,  gBleUuidType16_c, 0,  },
  { value_ppcp                 ,  gPermissionFlagReadable_c,                              0x2A04, value_ppcp_valueArray,                8,  gBleUuidType16_c, 0,  },

  { service_heart_rate         ,  gPermissionFlagReadable_c,                              0x2800, service_heart_rate_valueArray,        2,  gBleUuidType16_c, 0,  },
  { char_hr_measurement        ,  gPermissionFlagReadable_c,                              0x2803, char_hr_measurement_valueArray,       5,  gBleUuidType16_c, 0,  },
  { value_hr_measurement       ,  gPermissionNone_c,                                      0x2A37, value_hr_measurement_valueArray,      2,  gBleUuidType16_c, 22, },
  { cccd_hr_measurement        ,  gPermissionFlagReadable_c | gPermissionFlagWritable_c,  0x2902, cccd_hr_measurement_valueArray,       2,  gBleUuidType16_c, 0,  },
  { char_body_sensor_location  ,  gPermissionFlagReadable_c,                              0x2803, char_body_sensor_location_valueArray, 5,  gBleUuidType16_c, 0,  },
  { value_body_sensor_location ,  gPermissionFlagReadable_c,                              0x2A38, value_body_sensor_location_valueArray,1,  gBleUuidType16_c, 0,  },
  { char_hr_ctrl_point         ,  gPermissionFlagReadable_c,                              0x2803, char_hr_ctrl_point_valueArray,        5,  gBleUuidType16_c, 0,  },
  { value_hr_ctrl_point        ,  gPermissionFlagWritable_c,                              0x2A39, value_hr_ctrl_point_valueArray,       1,  gBleUuidType16_c, 0,  },

  { service_battery            ,  gPermissionFlagReadable_c,                              0x2800, service_battery_valueArray,           2,  gBleUuidType16_c,  0, },
  { char_battery_level         ,  gPermissionFlagReadable_c,                              0x2803, char_battery_level_valueArray,        5,  gBleUuidType16_c,  0, },
  { value_battery_level        ,  gPermissionFlagReadable_c,                              0x2A19, value_battery_level_valueArray,       1,  gBleUuidType16_c,  0, },
  { desc_bat_level             ,  gPermissionFlagReadable_c,                              0x2904, desc_bat_level_valueArray,            7,  gBleUuidType16_c,  0, },
  { cccd_battery_level         ,  gPermissionFlagReadable_c | gPermissionFlagWritable_c,  0x2902, cccd_battery_level_valueArray,        2,  gBleUuidType16_c,  0, },

  { service_device_info        ,  gPermissionFlagReadable_c,                              0x2800, service_device_info_valueArray,       2,  gBleUuidType16_c,  0, },
  { char_manuf_name            ,  gPermissionFlagReadable_c,                              0x2803, char_manuf_name_valueArray,           5,  gBleUuidType16_c,  0, },
  { value_manuf_name           ,  gPermissionFlagReadable_c,                              0x2A29, value_manuf_name_valueArray,          9,  gBleUuidType16_c,  0, },
  { char_model_no              ,  gPermissionFlagReadable_c,                              0x2803, char_model_no_valueArray,             5,  gBleUuidType16_c,  0, },
  { value_model_no             ,  gPermissionFlagReadable_c,                              0x2A24, value_model_no_valueArray,            8,  gBleUuidType16_c,  0, },
  { char_serial_no             ,  gPermissionFlagReadable_c,                              0x2803, char_serial_no_valueArray,            5,  gBleUuidType16_c,  0, },
  { value_serial_no            ,  gPermissionFlagReadable_c,                              0x2A25, value_serial_no_valueArray,           7,  gBleUuidType16_c,  0, },
  { char_hw_rev                ,  gPermissionFlagReadable_c,                              0x2803, char_hw_rev_valueArray,               5,  gBleUuidType16_c,  0, },
  { value_hw_rev               ,  gPermissionFlagReadable_c,                              0x2A27, value_hw_rev_valueArray,              sizeof"K66BLEZv1", gBleUuidType16_c, 0, },
  { char_fw_rev                ,  gPermissionFlagReadable_c,                              0x2803, char_fw_rev_valueArray,               5,  gBleUuidType16_c,  0, },
  { value_fw_rev               ,  gPermissionFlagReadable_c,                              0x2A26, value_fw_rev_valueArray,              5,  gBleUuidType16_c,  0, },
  { char_sw_rev                ,  gPermissionFlagReadable_c,                              0x2803, char_sw_rev_valueArray,               5,  gBleUuidType16_c,  0, },
  { value_sw_rev               ,  gPermissionFlagReadable_c,                              0x2A28, value_sw_rev_valueArray,              5,  gBleUuidType16_c,  0, },
  { char_system_id             ,  gPermissionFlagReadable_c,                              0x2803, char_system_id_valueArray,            5,  gBleUuidType16_c,  0, },
  { value_system_id            ,  gPermissionFlagReadable_c,                              0x2A23, value_system_id_valueArray,           8,  gBleUuidType16_c,  0, },
  { char_rcdl                  ,  gPermissionFlagReadable_c,                              0x2803, char_rcdl_valueArray,                 5,  gBleUuidType16_c,  0, },
  { value_rcdl                 ,  gPermissionFlagReadable_c,                              0x2A2A, value_rcdl_valueArray,                4,  gBleUuidType16_c,  0, },
};


*/

gattDbAttribute_t *gattDatabase = static_gattDatabase;

/*! Declare structure to compute the database size */
typedef struct sizeCounterStruct_tag {
  #include "gatt_size_x.def"
} sizeCounterStruct_t;

/*! Compute the database size at compile time */
  #define localGattDbAttributeCount_d  ((sizeof(sizeCounterStruct_t))/4)
uint16_t gGattDbAttributeCount_c;

#else
gattDbAttribute_t *gattDatabase;
uint16_t            gGattDbAttributeCount_c;
#endif

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief    Function performing runtime initialization of the GATT database.
*
* \remarks  This function should be called only once at device startup.
*
********************************************************************************** */
bleResult_t GattDb_Init()
{
#if !gGattDbDynamic_d
  /*! Assign the database size to the global */
  gGattDbAttributeCount_c = localGattDbAttributeCount_d;

  /*! Attribute-specific initialization by X-Macro expansion */
  #include "gatt_init_x.def"
#else
  gGattDbAttributeCount_c = 0;
#endif
  return gBleSuccess_c;
}

/*! *********************************************************************************
* \brief    Database searching function, return the index for a given attribute handle.
*
* \param[in] handle  The attribute handle.
*
* \return  The index of the given attribute in the database or gGattDbInvalidHandleIndex_d.
*
********************************************************************************** */
uint16_t GattDb_GetIndexOfHandle(uint16_t handle)
{
#if !gGattDbDynamic_d
  uint16_t init = (handle >= gGattDbAttributeCount_c) ? (gGattDbAttributeCount_c - 1) : handle;
  for (uint16_t j = init; j != 0xFFFF && gattDatabase[j].handle >= handle; j--)
  {
    if (gattDatabase[j].handle == handle)
    {
      return j;
    }
  }
  return gGattDbInvalidHandleIndex_d;
#else
  if (0x0000 == handle || handle > gGattDbAttributeCount_c)
  {
    return gGattDbInvalidHandleIndex_d;
  }
  else
  {
    return handle - 1;
  }
#endif
}

/*! *********************************************************************************
* @}
********************************************************************************** */
