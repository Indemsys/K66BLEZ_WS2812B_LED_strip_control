/*******************************************************************************
*
* Copyright 2004-2013 Freescale Semiconductor, Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale FreeMASTER License
* distributed with this Material.
* See the LICENSE file distributed for more details.
*
****************************************************************************//*!
*
* @brief  FreeMASTER Driver MQX dependent stuff
*
*******************************************************************************/

#include "freemaster.h"
#include "freemaster_private.h"

#if (FMSTR_PLATFORM_MQX) && (!(FMSTR_DISABLE))
void FMSTR_CopyMemory(uint8_t*  nDestAddr, uint8_t*  nSrcAddr, uint8_t nSize)
{
    uint8_t* ps = (uint8_t*) nSrcAddr;
    uint8_t* pd = (uint8_t*) nDestAddr;

    while(nSize--)
        *pd++ = *ps++;
}

/**************************************************************************//*!
*
* @brief  Write-into the communication buffer memory
*
* @param  pDestBuff - pointer to destination memory in communication buffer
* @param  nSrcAddr  - source memory address
* @param  nSize     - buffer size (always in bytes)
*
* @return This function returns a pointer to next byte in comm. buffer
*
******************************************************************************/

uint8_t*  FMSTR_CopyToBuffer(uint8_t*  pDestBuff, uint8_t*  nSrcAddr, uint8_t nSize)
{
    uint8_t* ps = (uint8_t*) nSrcAddr;
    uint8_t* pd = (uint8_t*) pDestBuff;

    _int_disable();
    while(nSize--)
        *pd++ = *ps++;
    _int_enable();

    return (uint8_t* ) pd;
}

/**************************************************************************//*!
*
* @brief  Read-out memory from communication buffer
*
* @param  nDestAddr - destination memory address
* @param  pSrcBuff  - pointer to source memory in communication buffer
* @param  nSize     - buffer size (always in bytes)
*
* @return This function returns a pointer to next byte in comm. buffer
*
******************************************************************************/

uint8_t*  FMSTR_CopyFromBuffer(uint8_t*  nDestAddr, uint8_t*  pSrcBuff, uint8_t nSize)
{
    uint8_t* ps = (uint8_t*) pSrcBuff;
    uint8_t* pd = (uint8_t*) nDestAddr;

    while(nSize--)
        *pd++ = *ps++;

    return (uint8_t* ) ps;
}


/**************************************************************************//*!
*
* @brief  Read-out memory from communication buffer, perform AND-masking
*
* @param  nDestAddr - destination memory address
* @param  pSrcBuff  - source memory in communication buffer, mask follows data
* @param  nSize     - buffer size (always in bytes)
*
******************************************************************************/

void FMSTR_CopyFromBufferWithMask(uint8_t*  nDestAddr, uint8_t*  pSrcBuff, uint8_t nSize)
{
    uint8_t* ps = (uint8_t*) pSrcBuff;
    uint8_t* pd = (uint8_t*) nDestAddr;
    uint8_t* pm = ps + nSize;
    uint8_t mask, stmp, dtmp;

    while(nSize--)
    {
        mask = *pm++;
        stmp = *ps++;
        dtmp = *pd;

        /* perform AND-masking */
        stmp = (uint8_t) ((stmp & mask) | (dtmp & ~mask));

        /* put the result back */
        *pd++ = stmp;
    }
}

/******************************************************************************/

/* mixed EX and no-EX commands? */
#if (FMSTR_USE_EX_CMDS) && (FMSTR_USE_NOEX_CMDS)

/**************************************************************************//*!
*
* @brief  When mixed EX and no-EX command may occur, this variable is
*         here to remember what command is just being handled.
*
******************************************************************************/

static int32_t pcm_bNextAddrIsEx;

void FMSTR_SetExAddr(int32_t bNextAddrIsEx)
{
    pcm_bNextAddrIsEx = bNextAddrIsEx;
}

/**************************************************************************//*!
*
* @brief  Store address to communication buffer. The address may be
*         32 or 16 bit wide (based on previous call to FMSTR_SetExAddr)
*
******************************************************************************/

uint8_t*  FMSTR_AddressToBuffer(uint8_t*  pDest, uint8_t*  nAddr)
{
    if(pcm_bNextAddrIsEx)
    {
        /* fill in the 32bit address */
        *(uint32_t*) pDest = ((uint32_t)nAddr);
        pDest += 4;
    }
    else
    {
        /* fill in the 16bit address (never used) */
        *(uint16_t*) pDest = ((uint16_t)nAddr);
        pDest += 2;
    }

    return pDest;
}

/**************************************************************************//*!
*
* @brief  Fetch address from communication buffer
*
******************************************************************************/

uint8_t*  FMSTR_AddressFromBuffer(uint8_t* * pAddr, uint8_t*  pSrc)
{
    if(pcm_bNextAddrIsEx)
    {
        *pAddr = (uint8_t* ) *((uint32_t*) pSrc);
        pSrc += 4;
    }
    else
    {
        *pAddr = (uint8_t* ) *((uint16_t*) pSrc);
        pSrc += 2;
    }

    return pSrc;
}

#endif /* mixed EX and no-EX commands */

#else /* (FMSTR_PLATFORM_MQX) && (!(FMSTR_DISABLE)) */

/*lint -efile(766, freemaster_protocol.h) include file is not used in this case */

#endif /* (FMSTR_PLATFORM_MQX) && (!(FMSTR_DISABLE)) */

