/*
 * FreeRTOS version 202406.01-LTS
 * Copyright (C) 2024 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */


/**
 * @brief Header file containing platform abstraction layer APIS for OTA update.
 */

#ifndef OTA_PAL_STREAMS_H
#define OTA_PAL_STREAMS_H


#include "jobs.h"
#include "job_parser.h"

/**
 * @ingroup ota_enum_types
 * @brief OTA Image states.
 *
 * After an OTA update image is received and authenticated, it is logically moved to
 * the Self Test state by the OTA agent pending final acceptance. After the image is
 * activated and tested by your user code, you should put it into either the Accepted
 * or Rejected state by calling @ref OTA_SetImageState ( OtaImageStateAccepted ) or
 * @ref OTA_SetImageState ( OtaImageStateRejected ). If the image is accepted, it becomes
 * the main firmware image to be booted from then on. If it is rejected, the image is
 * no longer valid and shall not be used, reverting to the last known good image.
 *
 * If you want to abort an active OTA transfer, you may do so by calling the API
 * @ref OTA_SetImageState ( OtaImageStateAborted ).
 */
typedef enum OtaImageState_New
{
    OtaImageStateUnknown_New = 0,  /*!< @brief The initial state of the OTA MCU Image. */
    OtaImageStateTesting_New = 1,  /*!< @brief The state of the OTA MCU Image post successful download and reboot. */
    OtaImageStateAccepted_New = 2, /*!< @brief The state of the OTA MCU Image post successful download and successful self_test. */
    OtaImageStateRejected_New = 3, /*!< @brief The state of the OTA MCU Image when the job has been rejected. */
    OtaImageStateAborted_New = 4,  /*!< @brief The state of the OTA MCU Image after a timeout publish to the stream request fails.
                                *   Also if the OTA MCU image is aborted in the middle of a stream. */
    OtaLastImageState_New = OtaImageStateAborted_New
} OtaImageState_New_t;

/**
 * @ingroup ota_enum_types
 * @brief OTA Platform Image State.
 *
 * The image state set by platform implementation.
 */
typedef enum OtaPalImageState_New
{
    OtaPalImageStateUnknown_New = 0,   /*!< @brief The initial state of the OTA PAL Image. */
    OtaPalImageStatePendingCommit_New, /*!< @brief OTA PAL Image awaiting update. */
    OtaPalImageStateValid_New,         /*!< @brief OTA PAL Image is valid. */
    OtaPalImageStateInvalid_New        /*!< @brief OTA PAL Image is invalid. */
} OtaPalImageState_New_t;

/**
 * @ingroup ota_enum_types
 * @brief OTA Platform Image State.
 *
 * The image state set by platform implementation.
 */
typedef enum OtaPalJobDocProcessingResult_New
{
    OtaPalJobDocFileCreated_New = 0,
    OtaPalJobDocFileCreateFailed_New,
    OtaPalNewImageBooted_New,
    OtaPalNewImageBootFailed_New,
    OtaPalJobDocProcessingStateInvalid_New
} OtaPalJobDocProcessingResult_New_t;

typedef enum OtaPalStatus_New
{
    OtaPalSuccess_New = 0,
    OtaPalUninitialized_New,
    OtaPalOutOfMemory_New,
    OtaPalNullFileContext_New,
    OtaPalSignatureCheckFailed_New,
    OtaPalRxFileCreateFailed_New,
    OtaPalRxFileTooLarge_New,
    OtaPalBootInfoCreateFailed_New,
    OtaPalBadSignerCert_New,
    OtaPalBadImageState_New,
    OtaPalAbortFailed_New,
    OtaPalRejectFailed_New,
    OtaPalCommitFailed_New,
    OtaPalActivateFailed_New,
    OtaPalFileAbort_New,
    OtaPalFileClose_New
} OtaPalStatus_New_t;

/* Specify the OTA signature algorithm we support on this platform. */
#define OTA_FILE_SIG_KEY_STR_MAX_LENGTH 32
#define OTA_SIG_KEY_STR "sig-sha256-ecdsa"

OtaPalStatus_New_t otaPal_Streams_Abort( AfrOtaJobDocumentFields_t * const pFileContext );
OtaPalJobDocProcessingResult_New_t otaPal_Streams_CreateFileForRx( AfrOtaJobDocumentFields_t * const pFileContext );
OtaPalStatus_New_t otaPal_Streams_CloseFile( AfrOtaJobDocumentFields_t * const pFileContext );
int16_t otaPal_Streams_WriteBlock( AfrOtaJobDocumentFields_t * const pFileContext, uint32_t ulOffset, uint8_t * const pcData, uint32_t ulBlockSize );
OtaPalStatus_New_t otaPal_Streams_ActivateNewImage( AfrOtaJobDocumentFields_t * const pFileContext );
OtaPalStatus_New_t otaPal_Streams_SetPlatformImageState( AfrOtaJobDocumentFields_t * const pFileContext, OtaImageState_New_t eState );
OtaPalImageState_New_t otaPal_Streams_GetPlatformImageState( AfrOtaJobDocumentFields_t * const pFileContext );
OtaPalStatus_New_t otaPal_Streams_ResetDevice( AfrOtaJobDocumentFields_t * const pFileContext );

#endif /* ifndef OTA_PAL_H_ */