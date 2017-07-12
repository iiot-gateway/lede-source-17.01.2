/*! *********************************************************************************
* \file hsdkLogger.h
* This is a header file for the Logger module.
*
* Copyright 2013-2016 Freescale Semiconductor, Inc.
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
********************************************************************************** */
#ifndef __HSDK_LOGGER__
#define __HSDK_LOGGER__

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "hsdkOSCommon.h"
#include "MessageQueue.h"

#ifdef __cplusplus
extern "C" {
#endif
/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */
/**
 * @brief The logger structure, containing a thread, a queue of messages to be
 * logged, a message count and an event signaling the end of logging.
 */
typedef struct {
    Thread loggerThread;	/**< A separate thread for the Logger. */
    MessageQueue *queue;	/**< Queue for Logger messages. */
    int referenceCount;		/**< The number of the logged messages. */
    Event stopThread;		/**< Event for signaling the Logger to stop. */
} Logger;

/*! *********************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
********************************************************************************** */
#define HSDK_ERROR 1
#define HSDK_WARNING 2
#define HSDK_INFO 3

/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */
void logMessage(int prio, const char *messageTag, const char *message, int threadId);
void initLogger(char *filename);
void closeLogger();

#ifdef __cplusplus
}
#endif

#endif
