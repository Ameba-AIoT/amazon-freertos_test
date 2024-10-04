/*
 * Copyright Amazon.com, Inc. and its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License. See the LICENSE accompanying this file
 * for the specific language governing permissions and limitations under
 * the License.
 */

/**
 * @file ota_event_freertos.h
 * @brief Function declarations for the example OTA OS Functional interface for
 * FreeRTOS. This file has been modified to coexist with the old ota demo
 */

#ifndef _OTA_OS_FREERTOS_H_
#define _OTA_OS_FREERTOS_H_

/* Standard library include. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/**
 * @ingroup ota_enum_types
 * @brief The OTA OS interface return status.
 */
typedef enum OtaOsStatus_New
{
    OtaOsSuccess_New = 0,                    /*!< @brief OTA OS interface success. */
    OtaOsEventQueueCreateFailed_New = 0x80U, /*!< @brief Failed to create the event
                                          * queue. */
    OtaOsEventQueueSendFailed_New,           /*!< @brief Posting event message to the event
                                          * queue failed. */
    OtaOsEventQueueReceiveFailed_New,        /*!< @brief Failed to receive from the event
                                          * queue. */
    OtaOsEventQueueDeleteFailed_New,         /*!< @brief Failed to delete the event queue.
                                          */
} OtaOsStatus_New_t;

/**
 * @brief Initialize the OTA events.
 *
 * This function initializes the OTA events mechanism for freeRTOS platforms.
 * 
 * @return               OtaOsStatus_t, OtaOsSuccess if success , other error
 * code on failure.
 */
OtaOsStatus_New_t OtaInitEvent_FreeRTOS_New( void );

/**
 * @brief Sends an OTA event.
 *
 * This function sends an event to OTA library event handler on FreeRTOS
 * platforms.

 * @param[pEventMsg]     Event to be sent to the OTA handler.
 *
 * @return               OtaOsStatus_t, OtaOsSuccess if success , other error
 * code on failure.
 */
OtaOsStatus_New_t OtaSendEvent_FreeRTOS_New( const void * pEventMsg );

/**
 * @brief Receive an OTA event.
 *
 * This function receives next event from the pending OTA events on FreeRTOS
 * platforms.
 *
 * @param[pEventMsg]     Pointer to store message.
 *
 * @return               OtaOsStatus_t, OtaOsSuccess if success , other error
 * code on failure.
 */
OtaOsStatus_New_t OtaReceiveEvent_FreeRTOS_New( void * pEventMsg );

/**
 * @brief Deinitialize the OTA Events mechanism.
 *
 * This function deinitialize the OTA events mechanism and frees any resources
 * used on FreeRTOS platforms.
 *
 * @return               OtaOsStatus_t, OtaOsSuccess if success , other error
 * code on failure.
 */
void OtaDeinitEvent_FreeRTOS_New();

#endif /* ifndef _OTA_OS_FREERTOS_H_ */