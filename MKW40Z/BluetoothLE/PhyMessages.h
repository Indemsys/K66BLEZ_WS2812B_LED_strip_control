/*!
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* All rights reserved.
*
* \file PhyMessages.h
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

#ifndef _PHY_MESSAGES_H
#define _PHY_MESSAGES_H


/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
********************************************************************************** */

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

/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
********************************************************************************** */

/* 802.15.4-2006 standard PHY PD and PLME API messages */
    gPdDataReq_c,
    gPdDataCnf_c,
    gPdDataInd_c,

    gPdIndQueueInsertReq_c,
    gPdIndQueueInsertCnf_c,
    gPdIndQueueRemoveReq_c,

    gPlmeCcaReq_c,
    gPlmeCcaCnf_c,

    gPlmeEdReq_c,
    gPlmeEdCnf_c,

    gPlmeSetTRxStateReq_c,
    gPlmeSetTRxStateCnf_c,

    gPlmeSetReq_c,
    gPlmeSetCnf_c,

    gPlmeGetReq_c,
    gPlmeGetCnf_c,

    gPlmeTimeoutInd_c,

    gPlme_StartEventInd_c,
    gPlme_SyncLossInd_c,
    gPlme_RxSfdDetectInd_c,
    gPlme_FilterFailInd_c,
    gPlme_UnexpectedRadioResetInd_c,


#endif  /* _PHY_MESSAGES_H */
