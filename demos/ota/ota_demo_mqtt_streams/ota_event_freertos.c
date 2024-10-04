/*
 * Copyright Amazon.com, Inc. and its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License. See the LICENSE accompanying this file
 * for the specific language governing permissions and limitations under
 * the License.
 */

/**
 * @file ota_event_freertos.c
 * @brief Example implementation of the OTA OS Functional Interface for
 * FreeRTOS. This file has been modified to coexist with the old ota library demo
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"

/* OTA OS POSIX Interface Includes.*/
#include "ota_event_freertos.h"

#include "ota_demo_mqtt_streams.h"

/* OTA Event queue attributes.*/
#define MAX_MESSAGES    20
#define MAX_MSG_SIZE    sizeof( OtaEventMsg_New_t )

/* Array containing pointer to the OTA event structures used to send events to the OTA task. */
static OtaEventMsg_New_t queueData[ MAX_MESSAGES * MAX_MSG_SIZE ];

/* The queue control structure.  .*/
static StaticQueue_t staticQueue;

/* The queue control handle.  .*/
static QueueHandle_t otaEventQueue;

OtaOsStatus_New_t OtaInitEvent_FreeRTOS_New()
{
    OtaOsStatus_New_t otaOsStatus = OtaOsSuccess_New;

    otaEventQueue = xQueueCreateStatic( ( UBaseType_t ) MAX_MESSAGES,
                                        ( UBaseType_t ) MAX_MSG_SIZE,
                                        ( uint8_t * ) queueData,
                                        &staticQueue );

    if( otaEventQueue == NULL ) {
        otaOsStatus = OtaOsEventQueueCreateFailed_New;
    } else {
/*        printf( "OTA Event Queue created.\n" ); */
    }

    return otaOsStatus;
}

OtaOsStatus_New_t OtaSendEvent_FreeRTOS_New( const void * pEventMsg )
{
    OtaOsStatus_New_t otaOsStatus = OtaOsSuccess_New;
    BaseType_t retVal = pdFALSE;

    /* Send the event to OTA event queue.*/
    retVal = xQueueSendToBack( otaEventQueue, pEventMsg, ( TickType_t ) 0 );

    if( retVal == pdTRUE )
    {
/*        printf( "OTA Event Sent.\n" ); */
    } else {
        otaOsStatus = OtaOsEventQueueSendFailed_New;
    }

    return otaOsStatus;
}

OtaOsStatus_New_t OtaReceiveEvent_FreeRTOS_New( void * pEventMsg )
{
    OtaOsStatus_New_t otaOsStatus = OtaOsSuccess_New;
    BaseType_t retVal = pdFALSE;

    retVal = xQueueReceive( otaEventQueue, ( OtaEventMsg_New_t * ) pEventMsg, pdMS_TO_TICKS( 3000U ) );

    if( retVal == pdTRUE ) {
        printf( "OTA Event received \n" );
    }
    else {
        otaOsStatus = OtaOsEventQueueReceiveFailed_New;
    }

    return otaOsStatus;
}

void OtaDeinitEvent_FreeRTOS_New()
{
    vQueueDelete( otaEventQueue );
}