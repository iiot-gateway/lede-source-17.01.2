/*! *********************************************************************************
* \file EventManager.h
* This is a header file for the EventManager module.
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
#ifndef __EVTMGR__
#define __EVTMGR__

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include <stdint.h>

#ifdef _WINDLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */
/**
 * @brief An observer structure, containing a callee, callback and a pointer to the
 * next structure of its kind. Element of a linked list.
 */
typedef struct _observer {
    void *callee; /**< User defined object to be notified upon reception. */
    void (*Callback) (void *callee, void *object); /**< Function pointer to be executed upon reception. */
    struct _observer *next; /**< Pointer to the next object of its kind. */
} Observer;

/**
 * @brief The event manager is equivalent to a list of observers.
 */
typedef struct {
    Observer *obsList;	/**< A Linked List with Observer nodes. */
} EventManager;

/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
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
DLLEXPORT EventManager *CreateEventManager();
DLLEXPORT void DestroyEventManager(EventManager *);
DLLEXPORT void RegisterToEventManager(EventManager *evt, void *callee, void(*Callback) (void *callee, void *object));
DLLEXPORT void DeregisterFromEvent(EventManager *evt, void *callee);
DLLEXPORT void NotifyOnEvent(EventManager *evt, void *object);
DLLEXPORT void NotifyOnSameEvent(EventManager *evt, void *object, void *(*func) (void *));

#ifdef __cplusplus
}
#endif

#endif
