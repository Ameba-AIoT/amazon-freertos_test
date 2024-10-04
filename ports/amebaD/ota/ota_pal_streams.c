/*
 * FreeRTOS OTA PAL for Example Vendor port V2.0.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

/* C Runtime includes. */
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS include. */
#include "FreeRTOS.h"

/* OTA PAL Port include. */
#include "ota_pal_streams.h"

/* public prototypes */
OtaPalStatus_New_t otaPal_Streams_Abort( AfrOtaJobDocumentFields_t * const pFileContext );
OtaPalJobDocProcessingResult_New_t otaPal_Streams_CreateFileForRx( AfrOtaJobDocumentFields_t * const pFileContext );
OtaPalStatus_New_t otaPal_Streams_CloseFile( AfrOtaJobDocumentFields_t * const pFileContext );
int16_t otaPal_Streams_WriteBlock( AfrOtaJobDocumentFields_t * const pFileContext, uint32_t ulOffset, uint8_t * const pcData, uint32_t ulBlockSize );
OtaPalStatus_New_t otaPal_Streams_ActivateNewImage( AfrOtaJobDocumentFields_t * const pFileContext );
OtaPalStatus_New_t otaPal_Streams_SetPlatformImageState( AfrOtaJobDocumentFields_t * const pFileContext, OtaImageState_New_t eState );
OtaPalImageState_New_t otaPal_Streams_GetPlatformImageState( AfrOtaJobDocumentFields_t * const pFileContext );
OtaPalStatus_New_t otaPal_Streams_ResetDevice( AfrOtaJobDocumentFields_t * const pFileContext );

/* private prototypes declared by internal code */
OtaPalStatus_New_t prvPAL_Streams_Abort_rtl8721d(AfrOtaJobDocumentFields_t *C);
OtaPalStatus_New_t prvPAL_Streams_CreateFileForRx_rtl8721d(AfrOtaJobDocumentFields_t *C);
OtaPalStatus_New_t prvPAL_Streams_CloseFile_rtl8721d(AfrOtaJobDocumentFields_t *C);
OtaPalStatus_New_t prvPAL_Streams_CheckFileSignature_rtl8721d(AfrOtaJobDocumentFields_t * const C);
int16_t prvPAL_Streams_WriteBlock_rtl8721d(AfrOtaJobDocumentFields_t *C, int32_t iOffset, uint8_t* pacData, uint32_t iBlockSize);
OtaPalStatus_New_t prvPAL_Streams_ActivateNewImage_rtl8721d(void);
OtaPalStatus_New_t prvPAL_Streams_SetPlatformImageState_rtl8721d (OtaImageState_New_t eState);
OtaPalImageState_New_t prvPAL_Streams_GetPlatformImageState_rtl8721d( void );
OtaPalStatus_New_t prvPAL_Streams_ResetDevice_rtl8721d(void);
uint8_t * prvPAL_Streams_ReadAndAssumeCertificate_rtl8721d(const uint8_t * const pucCertName, int32_t * const lSignerCertSize);

/*-----------------------------------------------------------*/

OtaPalStatus_New_t otaPal_Streams_Abort( AfrOtaJobDocumentFields_t * const pFileContext )
{
    return prvPAL_Streams_Abort_rtl8721d(pFileContext);
}

/*-----------------------------------------------------------*/

OtaPalJobDocProcessingResult_New_t otaPal_Streams_CreateFileForRx( AfrOtaJobDocumentFields_t * const pFileContext )
{
    return prvPAL_Streams_CreateFileForRx_rtl8721d(pFileContext) == 0 ? OtaPalJobDocFileCreated_New : OtaPalJobDocFileCreateFailed_New;
}

/*-----------------------------------------------------------*/

OtaPalStatus_New_t otaPal_Streams_CloseFile( AfrOtaJobDocumentFields_t * const pFileContext )
{
    return prvPAL_Streams_CloseFile_rtl8721d(pFileContext);
}

/*-----------------------------------------------------------*/

int16_t otaPal_Streams_WriteBlock( AfrOtaJobDocumentFields_t * const pFileContext, uint32_t ulOffset, uint8_t * const pcData, uint32_t ulBlockSize )
{
    return prvPAL_Streams_WriteBlock_rtl8721d(pFileContext, ulOffset, pcData, ulBlockSize);
}

/*-----------------------------------------------------------*/

OtaPalStatus_New_t otaPal_Streams_ActivateNewImage( AfrOtaJobDocumentFields_t * const pFileContext )
{
    return prvPAL_Streams_ActivateNewImage_rtl8721d();
}

/*-----------------------------------------------------------*/

OtaPalStatus_New_t otaPal_Streams_SetPlatformImageState( AfrOtaJobDocumentFields_t * const pFileContext, OtaImageState_New_t eState )
{
    return prvPAL_Streams_SetPlatformImageState_rtl8721d(eState);
}

/*-----------------------------------------------------------*/

OtaPalImageState_New_t otaPal_Streams_GetPlatformImageState( AfrOtaJobDocumentFields_t * const pFileContext )
{
    return prvPAL_Streams_GetPlatformImageState_rtl8721d();
}

/*-----------------------------------------------------------*/

OtaPalStatus_New_t otaPal_Streams_ResetDevice( AfrOtaJobDocumentFields_t * const pFileContext )
{
    (void) pFileContext;
    return prvPAL_Streams_ResetDevice_rtl8721d();
}

/*-----------------------------------------------------------*/

static OtaPalStatus_New_t otaPal_Streams_CheckFileSignature( AfrOtaJobDocumentFields_t * const pFileContext )
{
    return prvPAL_Streams_CheckFileSignature_rtl8721d(pFileContext);
}

/*-----------------------------------------------------------*/

static uint8_t * otaPal_Streams_ReadAndAssumeCertificate( const uint8_t * const pucCertName, uint32_t * const ulSignerCertSize )
{
    return (uint8_t *) prvPAL_Streams_ReadAndAssumeCertificate_rtl8721d( pucCertName, ulSignerCertSize);
}

/*-----------------------------------------------------------*/
