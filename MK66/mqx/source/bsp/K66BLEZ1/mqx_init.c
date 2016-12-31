/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
* THE POSSIBILITY OF SUCH DAMAGE.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* $FileName: mqx_init.c$
* $Version : 3.8.1.0$
* $Date    : Jun-6-2012$
*
* Comments:
*
*   This file contains the source for the default MQX INITIALIZATION
*   STRUCTURE
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"

extern TASK_TEMPLATE_STRUCT MQX_template_list[];

const MQX_INITIALIZATION_STRUCT  MQX_init_struct =
{
   /* PROCESSOR_NUMBER                */  BSP_DEFAULT_PROCESSOR_NUMBER,
   /* START_OF_KERNEL_MEMORY          */  BSP_DEFAULT_START_OF_KERNEL_MEMORY,
   /* END_OF_KERNEL_MEMORY            */  BSP_DEFAULT_END_OF_KERNEL_MEMORY,
   /* INTERRUPT_STACK_SIZE            */  BSP_DEFAULT_INTERRUPT_STACK_SIZE,
   /* TASK_TEMPLATE_LIST              */  MQX_template_list,
   /* MQX_HARDWARE_INTERRUPT_LEVEL_MAX*/  BSP_DEFAULT_MQX_HARDWARE_INTERRUPT_LEVEL_MAX,
   /* MAX_MSGPOOLS                    */  BSP_DEFAULT_MAX_MSGPOOLS,
   /* MAX_MSGQS                       */  BSP_DEFAULT_MAX_MSGQS,
   /* IO_CHANNEL                      */  BSP_DEFAULT_IO_CHANNEL,
   /* IO_OPEN_MODE                    */  BSP_DEFAULT_IO_OPEN_MODE,
   0,
   0,
#if MQX_ENABLE_USER_MODE   
   BSP_DEFAULT_START_OF_KERNEL_AREA,
   BSP_DEFAULT_END_OF_KERNEL_AREA,
   
   BSP_DEFAULT_START_OF_USER_DEFAULT_MEMORY,
   BSP_DEFAULT_END_OF_USER_DEFAULT_MEMORY,
   
   BSP_DEFAULT_START_OF_USER_HEAP,
   BSP_DEFAULT_END_OF_USER_HEAP,
   
   BSP_DEFAULT_START_OF_USER_RW_MEMORY,
   BSP_DEFAULT_END_OF_USER_RW_MEMORY,
   
   BSP_DEFAULT_START_OF_USER_RO_MEMORY,
   BSP_DEFAULT_END_OF_USER_RO_MEMORY,
   
   BSP_DEFAULT_START_OF_USER_NO_MEMORY,
   BSP_DEFAULT_END_OF_USER_NO_MEMORY,

   BSP_DEFAULT_MAX_USER_TASK_PRIORITY,
   BSP_DEFAULT_MAX_USER_TASK_COUNT,
#endif
};

/* EOF */
