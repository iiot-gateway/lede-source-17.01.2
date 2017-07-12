/*! *********************************************************************************
* \file UARTDiscovery.h
* This is the header file for the UARTDiscovery module.
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
#ifndef __UART_STATE__
#define __UART_STATE__

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include <stdint.h>

#include "EventManager.h"
#include "hsdkOSCommon.h"

#ifdef _WINDLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The availability state of a UART device.
 */
typedef enum {
    NoInfo,
    Available,
    NotPluggedIn,
    InUse,
    DeviceError,
} Availability;

/**
 * @brief A structure for representing the state of a discovered device.
 */
typedef struct {
    Availability state;         /**< The state in which the device is currently in. */
    char *friendlyName;         /**< The human-readable name used in the system for the device. */
    char *deviceName;           /**< The system path of the device. */
    uint8_t isKinetisWDevice;   /**< Boolean for whether the device is a NXP Kinetis-W. */
    char *vid;                  /**< The vendor id of the device. */
    char *pid;                  /**< The product id of the device. */
} DeviceState;

/**
 * @brief The type of the hotplug triggered event.
 */
typedef enum {
    DeviceAdded,
    DeviceRemoved,
} DeviceNotificationEvent;

/**
 * @brief Structure to describe a device notification.
 */
typedef struct {
    DeviceNotificationEvent notifyEvent;    /**< The type of the event. */
    DeviceState *state;                     /**< The state of the device. */
} DeviceNotification;

/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */
DLLEXPORT DeviceState *GetAllDevices(uint32_t *);
DLLEXPORT void InitializeDeviceManager();
DLLEXPORT void DestroyDeviceManager();
DLLEXPORT void AttachToDeviceNotification(void *observer, void(*Callback) (void *, void *));
DLLEXPORT void DetachFromDeviceNotification(void *observer);
DLLEXPORT void DestroyDeviceState(DeviceState *deviceState);
DLLEXPORT void DestroyDeviceNotification(DeviceNotification *deviceNotification);
DLLEXPORT int isKinetisWDevice(const char *, const char *);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
