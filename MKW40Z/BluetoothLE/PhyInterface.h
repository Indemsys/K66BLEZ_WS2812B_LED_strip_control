/*!
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* All rights reserved.
*
* \file PhyInterface.h
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

#ifndef _PHY_INTERFACE_H
#define _PHY_INTERFACE_H


/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */
#include "PhyTypes.h"
#include "Messaging.h"


/*! *********************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
********************************************************************************** */
#ifndef gPhyInstancesCnt_c
#define gPhyInstancesCnt_c  1
#endif

#ifndef gPhyTaskStackSize_c
#define gPhyTaskStackSize_c 600
#endif

#ifndef gPhyTaskPriority_c
#define gPhyTaskPriority_c  0
#endif

#ifndef gAfcEnabled_d
#define gAfcEnabled_d		0
#endif

/* Constants */
#ifndef gMaxPhyTimers_c
#define gMaxPhyTimers_c                 (5)
#endif

#define gPhySeqStartAsap_c              ((phyTime_t)(-1))


/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */

typedef enum
{
#include "PhyMessages.h"
}phyMessageId_t;

typedef enum{
    gPhyPwrIdle_c,
    gPhyPwrAutodoze_c,
    gPhyPwrDoze_c,
    gPhyPwrHibernate_c,
    gPhyPwrReset_c
}phyPwrMode_t;

typedef  struct pdDataReq_tag
{ 
    phyTime_t               startTime;  /* absolute */  
    uint32_t                txDuration; /* relative */
    phySlottedMode_t        slottedTx;
    phyCCAType_t            CCABeforeTx;
    phyAckRequired_t        ackRequired;
    uint8_t                 psduLength;
#ifdef gPHY_802_15_4g_d	
    phyPHR_t                phyHeader;
    uint8_t                 macDataIndex;
    uint8_t                 fillFifoBlockLength;    
#endif  /* gPHY_802_15_4g_d */	
    uint8_t *               pPsdu; 
} pdDataReq_t;

typedef  struct pdDataCnf_tag
{
    phyStatus_t             status;
} pdDataCnf_t;

typedef  struct pdDataInd_tag
{
    phyTime_t               timeStamp;
    uint8_t                 ppduLinkQuality;
    uint8_t                 psduLength;
    uint8_t *               pPsdu;
#ifdef gPHY_802_15_4g_d	
    uint32_t                crcValue;
    bool_t                  crcValid;
#endif
} pdDataInd_t;

typedef  struct pdIndQueueInsertReq_tag
{
    uint8_t                 index;
    uint16_t                checksum;
} pdIndQueueInsertReq_t;

typedef  struct pdIndQueueInsertCnf_tag
{
    phyStatus_t             status;
} pdIndQueueInsertCnf_t;

typedef  struct pdIndQueueRemoveReq_tag
{
    uint8_t                 index;
} pdIndQueueRemoveReq_t;

typedef  struct plmeEdReq_tag
{
    phyTime_t               startTime;  /* absolute */
} plmeEdReq_t;

typedef  struct plmeCcaReq_tag
{
    phyCCAType_t            ccaType;
    phyContCCAMode_t        contCcaMode;
} plmeCcaReq_t;

typedef  struct plmeCcaCnf_tag
{
    phyStatus_t             status;
} plmeCcaCnf_t;

typedef  struct plmeEdCnf_tag
{
    phyStatus_t             status;
    uint8_t                 energyLevel;
    uint8_t                 energyLeveldB;
} plmeEdCnf_t;

typedef  struct plmeSetTRxStateReq_tag
{
    phyState_t              state;
    phySlottedMode_t        slottedMode;
    phyTime_t               startTime;   /* absolute */
    uint32_t                rxDuration;  /* relative */
} plmeSetTRxStateReq_t;

typedef struct phyTimeEvent_tag
{
    phyTime_t          timestamp; /* absolute */
    phyTimeCallback_t  callback;
    uint32_t           parameter;
}phyTimeEvent_t;

typedef  struct plmeSetTRxStateCnf_tag
{
    phyStatus_t             status;
} plmeSetTRxStateCnf_t;

typedef  struct plmeSetReq_tag
{
    phyPibId_t              PibAttribute;
    uint64_t                PibAttributeValue;
} plmeSetReq_t;

typedef  struct plmeSetCnf_tag
{
    phyStatus_t             status;
    phyPibId_t              PibAttribute;
} plmeSetCnf_t;

typedef  struct plmeGetReq_tag
{
    phyPibId_t              PibAttribute;
    uint64_t *              pPibAttributeValue;
} plmeGetReq_t;

typedef  struct plmeGetCnf_tag
{
    phyStatus_t             status;
    phyPibId_t              PibAttribute;
    uint64_t                PibAttributeValue;
} plmeGetCnf_t;

typedef  struct macToPlmeMessage_tag
{
    phyMessageId_t             msgType;
    uint8_t                    macInstance;
    union
    {
        plmeEdReq_t                edReq;
        plmeCcaReq_t               ccaReq;
        plmeSetTRxStateReq_t       setTRxStateReq;
        plmeSetReq_t               setReq;
        plmeGetReq_t               getReq;
    }msgData;
} macToPlmeMessage_t;

typedef  struct macToPdDataMessage_tag
{
    phyMessageId_t             msgType;
    uint8_t                    macInstance;
    union
    {
        pdDataReq_t             dataReq;
        pdIndQueueInsertReq_t   indQueueInsertReq;
        pdIndQueueRemoveReq_t   indQueueRemoveReq;
    }msgData;
} macToPdDataMessage_t;

typedef  struct plmeToMacMessage_tag
{
    phyMessageId_t             msgType;
    uint8_t                    macInstance;
    union
    {
        plmeCcaCnf_t            ccaCnf;
        plmeEdCnf_t             edCnf;
        plmeSetTRxStateCnf_t    setTRxStateCnf;
        plmeSetCnf_t            setCnf;
        plmeGetCnf_t            getCnf;
    }msgData;
} plmeToMacMessage_t;

typedef  struct pdDataToMacMessage_tag
{
    phyMessageId_t             msgType;
    uint8_t                    macInstance;
    union
    {
        pdDataCnf_t             dataCnf;
        pdDataInd_t             dataInd;
        pdIndQueueInsertCnf_t   indQueueInsertCnf;
    }msgData;
} pdDataToMacMessage_t;

typedef  struct phyMessageHeader_tag
{
    phyMessageId_t             msgType;
    uint8_t                    macInstance;
} phyMessageHeader_t;

typedef struct phyRxParams_tag {
    phyTime_t   timeStamp; /* [symbols] : Rx startTime / DataIndication timestamp */
    uint8_t     psduLength;
    uint8_t     linkQuality;
#ifdef gPHY_802_15_4g_d	    
    uint8_t     headerLength; 
    uint8_t     macDataIndex;
    uint8_t     fifoBlockLen;
    phyPHR_t    phyHeader;
#else
    uint32_t              duration; /* [symbols] */
    pdDataToMacMessage_t *pRxData;
    phySlottedMode_t      phyRxMode;
#endif  /* gPHY_802_15_4g_d */  
} phyRxParams_t;

typedef struct phyChannelParams_tag {
    union{
        phyStatus_t channelStatus;
        uint8_t     energyLeveldB;
    };
#ifdef gPHY_802_15_4g_d
    uint8_t     ccaParam;
#endif   /* gPHY_802_15_4g_d */	
} phyChannelParams_t;

typedef struct phyTxParams_tag {
  uint8_t           numOfCca;
  phyAckRequired_t  ackRequired;
} phyTxParams_t;

typedef phyStatus_t ( * PD_MAC_SapHandler_t)(pdDataToMacMessage_t * pMsg, instanceId_t instanceId);

typedef phyStatus_t ( * PLME_MAC_SapHandler_t)(plmeToMacMessage_t * pMsg, instanceId_t instanceId);

#ifdef __cplusplus
extern "C" {
#endif 

/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
extern const uint8_t gPhyIndirectQueueSize_c;


/*! *********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
********************************************************************************** */

void Phy_Init( void );

instanceId_t BindToPHY( instanceId_t macInstance );

/* Service Access Points */
void Phy_RegisterSapHandlers(PD_MAC_SapHandler_t pPD_MAC_SapHandler, PLME_MAC_SapHandler_t pPLME_MAC_SapHandler, instanceId_t instanceId);

phyStatus_t MAC_PD_SapHandler(macToPdDataMessage_t * pMsg, instanceId_t phyInstance);

phyStatus_t MAC_PLME_SapHandler(macToPlmeMessage_t * pMsg, instanceId_t phyInstance);

/* PHY Time API */
void PhyTime_Maintenance ( void );

void PhyTime_RunCallback ( void );

void PhyTime_ISR         ( void );

phyTimeStatus_t PhyTime_TimerInit ( void (*cb)(void) );

phyTime_t          PhyTime_GetTimestamp ( void );

phyTimeTimerId_t   PhyTime_ScheduleEvent( phyTimeEvent_t *pEvent );

phyTimeStatus_t    PhyTime_CancelEvent  ( phyTimeTimerId_t timerId );

phyTimeStatus_t    PhyTime_CancelEventsWithParam ( uint32_t param );

#ifdef gPHY_802_15_4g_d
phyTime_t PhyTime_GetTimestampUs(void);
phyTime_t PhyTime_GetSymbolsToUs(phyTime_t tsSymbols);
phyTime_t PhyTime_GetUsToSymbols(phyTime_t tsUs);
#endif

/* PHY PLME API */
phyStatus_t        PhyPlmeSetPwrState( uint8_t state );

#ifdef __cplusplus
}
#endif 
    
#endif  /* _PHY_INTERFACE_H */
