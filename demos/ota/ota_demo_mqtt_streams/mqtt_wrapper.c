/*
 * Copyright Amazon.com, Inc. and its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License. See the LICENSE accompanying this file
 * for the specific language governing permissions and limitations under
 * the License.
 */

#include <assert.h>
#include <string.h>

#include "mqtt_wrapper.h"
#include "ota_demo_mqtt_streams.h"

#define MQTT_AGENT_NOTIFY_IDX    ( 2 )

static MQTTContext_t * globalCoreMqttContext = NULL;

/**
 * @brief Defines the structure to use as the command callback context in this
 * demo.
 */
struct MQTTAgentCommandContext
{
    MQTTStatus_t xReturnStatus;
    TaskHandle_t xTaskToNotify;
};

static void handleIncomingMQTTMessage( char * topic, size_t topicLength, uint8_t * message, size_t messageLength )
{
    bool messageHandled = otaDemo_handleIncomingMQTTMessage( topic, topicLength, message, messageLength );

    if( !messageHandled ) {
        printf( "Unhandled incoming PUBLISH received on topic, message: "
                "%.*s\n%.*s\n",
                ( unsigned int ) topicLength,
                topic,
                ( unsigned int ) messageLength,
                ( char * ) message );
    }
}

void mqttWrapper_setCoreMqttContext( MQTTContext_t * mqttContext )
{
    globalCoreMqttContext = mqttContext;
}

MQTTContext_t * mqttWrapper_getCoreMqttContext( void )
{
    assert( globalCoreMqttContext != NULL );
    return globalCoreMqttContext;
}

bool mqttWrapper_connect( char * thingName, size_t thingNameLength )
{
    MQTTConnectInfo_t connectInfo = { 0 };
    MQTTStatus_t mqttStatus = MQTTSuccess;
    bool sessionPresent = false;

    assert( globalCoreMqttContext != NULL );

    connectInfo.pClientIdentifier = thingName;
    connectInfo.clientIdentifierLength = thingNameLength;
    connectInfo.pUserName = NULL;
    connectInfo.userNameLength = 0U;
    connectInfo.pPassword = NULL;
    connectInfo.passwordLength = 0U;
    connectInfo.keepAliveSeconds = 60U;
    connectInfo.cleanSession = true;
    mqttStatus = MQTT_Connect( globalCoreMqttContext, &connectInfo, NULL, 5000U, &sessionPresent );
    return mqttStatus == MQTTSuccess;
}

bool mqttWrapper_isConnected( void )
{
    bool isConnected = false;

    assert( globalCoreMqttContext != NULL );
    isConnected = globalCoreMqttContext->connectStatus == MQTTConnected;
    return isConnected;
}

static void prvPublishCommandCallback( MQTTAgentCommandContext_t * pCmdCallbackContext, MQTTAgentReturnInfo_t * pReturnInfo )
{
    TaskHandle_t xTaskHandle = ( struct tskTaskControlBlock * ) pCmdCallbackContext->xTaskToNotify;

    if( xTaskHandle != NULL ) {
        uint32_t ulNotifyValue = MQTTSuccess; /* ( pxReturnInfo->returnCode & 0xFFFFFF ); */
/* */
/*		if( pxReturnInfo->pSubackCodes ) */
/*		{ */
/*			ulNotifyValue += ( pxReturnInfo->pSubackCodes[ 0 ] << 24 ); */
/*		} */
#if (tskKERNEL_VERSION_MAJOR >= 10) && (tskKERNEL_VERSION_MINOR >= 4) && (tskKERNEL_VERSION_BUILD >= 0)
        ( void ) xTaskNotifyIndexed( xTaskHandle,
                                     MQTT_AGENT_NOTIFY_IDX,
                                     ulNotifyValue,
                                     eSetValueWithOverwrite );
#else
        ( void ) xTaskNotify( xTaskHandle,
                              ulNotifyValue,
                              eSetValueWithOverwrite );
#endif
    }
}

bool mqttWrapper_publish( char * topic, size_t topicLength, uint8_t * message, size_t messageLength )
{
    bool success = false;

    assert( globalCoreMqttContext != NULL );

    success = mqttWrapper_isConnected();

    if( success ) {
        MQTTStatus_t mqttStatus = MQTTSuccess;
        /* TODO: This should be static or should we wait? */
        static MQTTPublishInfo_t pubInfo = { 0 };
        MQTTAgentContext_t * xAgentHandle = xGetMqttAgentHandle();
        pubInfo.qos = 0;
        pubInfo.retain = false;
        pubInfo.dup = false;
        pubInfo.pTopicName = topic;
        pubInfo.topicNameLength = topicLength;
        pubInfo.pPayload = message;
        pubInfo.payloadLength = messageLength;

        MQTTAgentCommandContext_t xCommandContext =
        {
            .xTaskToNotify = xTaskGetCurrentTaskHandle(),
            .xReturnStatus = MQTTIllegalState,
        };

        MQTTAgentCommandInfo_t xCommandParams =
        {
            .blockTimeMs                 = 100,
            .cmdCompleteCallback         = prvPublishCommandCallback,
            .pCmdCompleteCallbackContext = &xCommandContext,
        };

#if (tskKERNEL_VERSION_MAJOR >= 10) && (tskKERNEL_VERSION_MINOR >= 4) && (tskKERNEL_VERSION_BUILD >= 0)
        ( void ) xTaskNotifyStateClearIndexed( NULL, MQTT_AGENT_NOTIFY_IDX );
#else
        ( void ) xTaskNotifyStateClear( NULL );
#endif

        mqttStatus = MQTTAgent_Publish( xAgentHandle, &pubInfo, &xCommandParams );

        if( mqttStatus == MQTTSuccess ) {
            uint32_t ulNotifyValue = 0;
#if (tskKERNEL_VERSION_MAJOR >= 10) && (tskKERNEL_VERSION_MINOR >= 4) && (tskKERNEL_VERSION_BUILD >= 0)
            if( xTaskNotifyWaitIndexed( MQTT_AGENT_NOTIFY_IDX,
                                        0x0,
                                        0xFFFFFFFF,
                                        &ulNotifyValue,
                                        portMAX_DELAY ) )
            {
                mqttStatus = ( ulNotifyValue & 0x00FFFFFF );
            }
            else
            {
                mqttStatus = MQTTKeepAliveTimeout;
            }
#else
            if( xTaskNotifyWait( 0x0, 0xFFFFFFFF, &ulNotifyValue, portMAX_DELAY ) ) {
                mqttStatus = ( ulNotifyValue & 0x00FFFFFF );
            } else {
                mqttStatus = MQTTKeepAliveTimeout;
            }
#endif
        }

        success = mqttStatus == MQTTSuccess;
    }

    return success;
}

void handleIncomingPublish( void * pvIncomingPublishCallbackContext, MQTTPublishInfo_t * pxPublishInfo )
{
    (void) pvIncomingPublishCallbackContext;

    char * topic = NULL;
    size_t topicLength = 0U;
    uint8_t * message = NULL;
    size_t messageLength = 0U;

    topic = ( char * ) pxPublishInfo->pTopicName;
    topicLength = pxPublishInfo->topicNameLength;
    message = ( uint8_t * ) pxPublishInfo->pPayload;
    messageLength = pxPublishInfo->payloadLength;
    handleIncomingMQTTMessage( topic, topicLength, message, messageLength );
}

bool mqttWrapper_subscribe( char * topic, size_t topicLength ) {
    bool success = false;

    assert( globalCoreMqttContext != NULL );

    success = mqttWrapper_isConnected();

    if( success ) {
        MQTTStatus_t mqttStatus = MQTTSuccess;

        mqttStatus = MqttAgent_SubscribeSync( topic, topicLength, 0, handleIncomingPublish, NULL );

        configASSERT( mqttStatus == MQTTSuccess );

        success = mqttStatus == MQTTSuccess;
    }

    return success;
}