#ifndef OTA_DEMO_H
#define OTA_DEMO_H

#include <stdint.h>
#include <stdbool.h>

#include "MQTTFileDownloader.h"
#include "MQTTFileDownloader_defaults.h"

#include "jobs.h"
#include "job_parser.h"

#include "ota_pal_streams.h"

/* Include header for connection configurations. */
#include "aws_clientcredential.h"

/* Include header for client credentials. */
#include "aws_clientcredential_keys.h"

/* Include header for root CA certificates. */
#include "iot_default_root_certificates.h"

#define OTA_DATA_BLOCK_SIZE    mqttFileDownloader_CONFIG_BLOCK_SIZE * 2 // additional space for json structures
#define JOB_DOC_SIZE           2048U

// following types are redefined so the old ota library types will not be used

typedef enum OtaEvent_New
{
    OtaAgentEventStart_New = 0,           /*!< @brief Start the OTA state machine */
    OtaAgentEventRequestJobDocument_New,  /*!< @brief Event for requesting job document. */
    OtaAgentEventReceivedJobDocument_New, /*!< @brief Event when job document is received. */
    OtaAgentEventCreateFile_New,          /*!< @brief Event to create a file. */
    OtaAgentEventRequestFileBlock_New,    /*!< @brief Event to request file blocks. */
    OtaAgentEventReceivedFileBlock_New,   /*!< @brief Event to trigger when file block is received. */
    OtaAgentEventCloseFile_New,           /*!< @brief Event to trigger closing file. */
    OtaAgentEventActivateImage_New,       /*!< @brief Event to activate the new image. */
    OtaAgentEventSuspend_New,             /*!< @brief Event to suspend ota task */
    OtaAgentEventResume_New,              /*!< @brief Event to resume suspended task */
    OtaAgentEventUserAbort_New,           /*!< @brief Event triggered by user to stop agent. */
    OtaAgentEventShutdown_New,            /*!< @brief Event to trigger ota shutdown */
    OtaAgentEventMax_New                  /*!< @brief Last event specifier */
} OtaEvent_New_t;

/**
 * @brief OTA Agent states.
 *
 * The current state of the OTA Task (OTA Agent).
 */
typedef enum OtaState_New
{
    OtaAgentStateNoTransition_New = -1,
    OtaAgentStateInit_New = 0,
    OtaAgentStateReady_New,
    OtaAgentStateRequestingJob_New,
    OtaAgentStateWaitingForJob_New,
    OtaAgentStateCreatingFile_New,
    OtaAgentStateRequestingFileBlock_New,
    OtaAgentStateWaitingForFileBlock_New,
    OtaAgentStateClosingFile_New,
    OtaAgentStateSuspended_New,
    OtaAgentStateShuttingDown_New,
    OtaAgentStateStopped_New,
    OtaAgentStateAll_New
} OtaState_New_t;

/**
 * @brief  The OTA Agent event and data structures.
 */

typedef struct OtaDataEvent_New
{
    uint8_t data[ OTA_DATA_BLOCK_SIZE * 2 ]; /*!< Buffer for storing event information. */
    size_t dataLength;                       /*!< Total space required for the event. */
    bool bufferUsed;                         /*!< Flag set when buffer is used otherwise cleared. */
} OtaDataEvent_New_t;

typedef struct OtaJobEventData_New
{
    uint8_t jobData[ JOB_DOC_SIZE ];
    size_t jobDataLength;
} OtaJobEventData_New_t;

/**
 * @brief Stores information about the event message.
 *
 */
typedef struct OtaEventMsg_New
{
    OtaDataEvent_New_t * dataEvent;   /*!< Data Event message. */
    OtaJobEventData_New_t * jobEvent; /*!< Job Event message. */
    OtaEvent_New_t eventId;           /*!< Identifier for the event. */
} OtaEventMsg_New_t;

/** Note: The device client certificate and private key credentials are
 * obtained by the transport interface implementation (with Secure Sockets)
 * from the demos/include/aws_clientcredential_keys.h file.
 *
 * The following macros SHOULD be defined for this demo which uses both server
 * and client authentications for TLS session:
 *   - keyCLIENT_CERTIFICATE_PEM for client certificate.
 *   - keyCLIENT_PRIVATE_KEY_PEM for client private key.
 */

/**
 * @brief The MQTT broker endpoint used for this demo.
 */
#ifndef democonfigMQTT_BROKER_ENDPOINT
    #define democonfigMQTT_BROKER_ENDPOINT    clientcredentialMQTT_BROKER_ENDPOINT
#endif

/**
 * @brief The root CA certificate belonging to the broker.
 */
#ifndef democonfigROOT_CA_PEM
    #define democonfigROOT_CA_PEM    tlsATS1_ROOT_CERTIFICATE_PEM
#endif

#ifndef democonfigCLIENT_IDENTIFIER

/**
 * @brief The MQTT client identifier used in this example.  Each client identifier
 * must be unique so edit as required to ensure no two clients connecting to the
 * same broker use the same client identifier.
 */
    #define democonfigCLIENT_IDENTIFIER    clientcredentialIOT_THING_NAME
#endif

#ifndef democonfigMQTT_BROKER_PORT

/**
 * @brief The port to use for the demo.
 */
    #define democonfigMQTT_BROKER_PORT    clientcredentialMQTT_BROKER_PORT
#endif


#endif