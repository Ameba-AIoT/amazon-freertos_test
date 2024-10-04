/*
Amazon FreeRTOS OTA PAL for Realtek Ameba V1.0.0
Copyright (C) 2018 Amazon.com, Inc. or its affiliates.	All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 http://aws.amazon.com/freertos
 http://www.FreeRTOS.org
*/

/* OTA PAL implementation for Realtek Ameba D platform. */
#include "ota_demo_config.h"
#include "ota_pal_streams.h"
#include "iot_crypto.h"
#include "core_pkcs11.h"
#include "platform_opts.h"
#include "osdep_service.h"
#include "flash_api.h"
#include <device_lock.h>
#include "platform_stdlib.h"

#define OTA_MEMDUMP 0
#define OTA_PRINT DiagPrintf

static uint32_t aws_ota_imgaddr = 0;
static uint32_t aws_ota_imgsz = 0;
static bool aws_ota_target_hdr_get = false;
static uint32_t ota_target_index = OTA_INDEX_2;
static uint32_t HdrIdx = 0;
static update_ota_target_hdr aws_ota_target_hdr;
static uint8_t aws_ota_signature[8] = {0};

#define AWS_OTA_IMAGE_SIGNATURE_LEN 	8
#define OTA1_FLASH_START_ADDRESS		LS_IMG2_OTA1_ADDR	//0x08006000
#define OTA2_FLASH_START_ADDRESS		LS_IMG2_OTA2_ADDR	//0x08106000

//move to platform_opts.h
//#define AWS_OTA_IMAGE_STATE_FLASH_OFFSET			( 0x101000 ) // 0x0810_0000 - 0x0810_2000-1
#define AWS_OTA_IMAGE_STATE_FLAG_IMG_NEW			0xffffffffU /* 11111111b A new image that hasn't yet been run. */
#define AWS_OTA_IMAGE_STATE_FLAG_PENDING_COMMIT 	0xfffffffeU /* 11111110b Image is pending commit and is ready for self test. */
#define AWS_OTA_IMAGE_STATE_FLAG_IMG_VALID			0xfffffffcU /* 11111100b The image was accepted as valid by the self test code. */
#define AWS_OTA_IMAGE_STATE_FLAG_IMG_INVALID		0xfffffff8U /* 11111000b The image was NOT accepted by the self test code. */

typedef struct {
	int32_t lFileHandle;
} ameba_ota_context_t;

static ameba_ota_context_t ota_ctx;

OtaPalStatus_New_t prvPAL_Streams_CheckFileSignature_rtl8721d(AfrOtaJobDocumentFields_t * const C);

extern void rtc_backup_timeinfo(void);
extern int ota_writestream_user(u32 address, u32 len, u8 * data);

static void prvPAL_Streams_SysReset_rtl8721d(u32 timeout_ms)
{
	WDG_InitTypeDef WDG_InitStruct;
	u32 CountProcess;
	u32 DivFacProcess;

#if defined(CONFIG_MBED_API_EN) && CONFIG_MBED_API_EN
	rtc_backup_timeinfo();
#endif

	WDG_Scalar(timeout_ms, &CountProcess, &DivFacProcess);
	WDG_InitStruct.CountProcess = CountProcess;
	WDG_InitStruct.DivFacProcess = DivFacProcess;
	WDG_Init(&WDG_InitStruct);

	WDG_Cmd(ENABLE);
}

OtaPalStatus_New_t prvPAL_Streams_Abort_rtl8721d(AfrOtaJobDocumentFields_t *C)
{
	if ( C != NULL && C->filepath != NULL ) {
		OTA_PRINT("[%s] Abort OTA update\n", __FUNCTION__);
		C->filepath = NULL;
		ota_ctx.lFileHandle = 0x0;
	}
	return OtaPalSuccess_New;
}

bool prvPAL_Streams_CreateFileForRx_rtl8721d(AfrOtaJobDocumentFields_t *C)
{
	OtaPalStatus_New_t mainErr = OtaPalSuccess_New;

	int sector_cnt = 0;
	OTA_PRINT("\n\r[%s] OTA filesize: %d\n", __FUNCTION__, C->fileSize);

	/* determine the segment to store the OTA download in */
	if ( ota_get_cur_index() == OTA_INDEX_1 ) {
		ota_target_index = OTA_INDEX_2;
		ota_ctx.lFileHandle = OTA2_FLASH_START_ADDRESS;
		sector_cnt = ((C->fileSize - 1) / (1024 * 4)) + 1;
		OTA_PRINT("\n\r[%s] OTA2 address space will be upgraded\n", __FUNCTION__);
	} else {
		ota_target_index = OTA_INDEX_1;
		ota_ctx.lFileHandle = OTA1_FLASH_START_ADDRESS;
		sector_cnt = ((C->fileSize - 1) / (1024 * 4)) + 1;
		OTA_PRINT("\n\r[%s] OTA1 address space will be upgraded\n", __FUNCTION__);
	}

	/* check the segment is valid and prepare the segment for write  */
	if ( ota_ctx.lFileHandle > SPI_FLASH_BASE ) {
		OTA_PRINT("[OTA] valid ota addr (0x%x) \r\n", ota_ctx.lFileHandle);
		aws_ota_imgaddr = ota_ctx.lFileHandle;
		aws_ota_imgsz = 0;
		aws_ota_target_hdr_get = false;
		memset((void *)&aws_ota_target_hdr, 0, sizeof(update_ota_target_hdr));
		memset((void *)aws_ota_signature, 0, sizeof(aws_ota_signature));

		for(int i = 0; i < sector_cnt; i++) {
			OTA_PRINT("[OTA] Erase sector_cnt @ 0x%x\n", ota_ctx.lFileHandle - SPI_FLASH_BASE + i * (1024*4));
			erase_ota_target_flash(aws_ota_imgaddr - SPI_FLASH_BASE + i * (1024*4), (1024*4));
		}
	} else {
		OTA_PRINT("[OTA] invalid ota addr (%d) \r\n", ota_ctx.lFileHandle);
		ota_ctx.lFileHandle = (int32_t) NULL; 	 /* Nullify the file handle in all error cases. (fix: cast warning) */
	}

	if( ota_ctx.lFileHandle <= SPI_FLASH_BASE ) {
		mainErr = OtaPalRxFileCreateFailed_New;
	}

	return mainErr;
}

static OtaPalStatus_New_t prvPAL_Streams_SignatureVerificationUpdate_rtl8721d(AfrOtaJobDocumentFields_t *C, void * pvContext)
{
	(void) C;	// unused

	OtaPalStatus_New_t mainErr = OtaPalSuccess_New;

	u32 i;
	flash_t flash;
	u8 * pTempbuf;
	int rlen;
	u32 len = aws_ota_imgsz;
	u32 addr = aws_ota_target_hdr.FileImgHdr[HdrIdx].FlashAddr;

	if( len <= 0 ) {
		mainErr = OtaPalSignatureCheckFailed_New;
		return mainErr;
	}

	pTempbuf = ota_update_malloc(BUF_SIZE);
	if( pTempbuf == NULL ) {
		mainErr = OtaPalSignatureCheckFailed_New;
		goto error;
	}

	/*add image signature(81958711)*/
	CRYPTO_SignatureVerificationUpdate(pvContext, aws_ota_signature, AWS_OTA_IMAGE_SIGNATURE_LEN);

	len = len-8;
	/* read flash data back to check signature of the image */
	for( i = 0; i < len; i += BUF_SIZE ) {
		rlen = (len - i) > BUF_SIZE ? BUF_SIZE : (len - i);
		flash_stream_read(&flash, addr - SPI_FLASH_BASE + i + AWS_OTA_IMAGE_SIGNATURE_LEN, rlen, pTempbuf);
		Cache_Flush();
		CRYPTO_SignatureVerificationUpdate(pvContext, pTempbuf, rlen);
	}

error:
	if( pTempbuf != NULL ) {
		ota_update_free(pTempbuf);
	}

	return mainErr;
}

OtaPalStatus_New_t prvPAL_Streams_SetPlatformImageState_rtl8721d (OtaImageState_New_t eState);

/* Close the specified file. This will also authenticate the file if it is marked as secure. */
OtaPalStatus_New_t prvPAL_Streams_CloseFile_rtl8721d(AfrOtaJobDocumentFields_t *C)
{
	OtaPalStatus_New_t mainErr = OtaPalSuccess_New;

	OTA_PRINT("[OTA] Authenticating and closing file.\n");

	if ( C == NULL ) {
		mainErr = OtaPalNullFileContext_New;
		goto exit;
	}

	/* close the fw file */
	if ( C->signature != NULL ) {
		/* TODO: Verify the file signature, close the file and return the signature verification result. */
		mainErr = prvPAL_Streams_CheckFileSignature_rtl8721d(C);

	} else {
		mainErr = OtaPalSignatureCheckFailed_New;
	}

	if ( mainErr == OtaPalSuccess_New ) {
		OTA_PRINT("[%s] %s signature verification passed.\n", __FUNCTION__, OTA_SIG_KEY_STR);
	} else {
		OTA_PRINT("[%s] Failed to pass %s signature verification: %d.\n", __FUNCTION__, OTA_SIG_KEY_STR, mainErr);

		/* If we fail to verify the file signature that means the image is not valid. We need to set the image state to aborted. */
		prvPAL_Streams_SetPlatformImageState_rtl8721d(OtaImageStateAborted_New);
	}

exit:
	return mainErr;
}

int32_t prvPAL_Streams_WriteBlock_rtl8721d(AfrOtaJobDocumentFields_t *C, uint32_t ulOffset, uint8_t* pData, uint32_t ulBlockSize)
{
	(void) C;	// unused

	uint32_t address = ota_ctx.lFileHandle - SPI_FLASH_BASE;
	uint32_t WriteLen, offset;
	uint32_t version=0, major=0, minor=0, build=0;

	if ( aws_ota_target_hdr_get != true ) {
		u32 RevHdrLen;

		/* first block is downloaded, check if this is a valid OTA image by reading the header */
		if( ulOffset == 0 ) {
			memset((void *)&aws_ota_target_hdr, 0, sizeof(update_ota_target_hdr));
			memset((void *)aws_ota_signature, 0, sizeof(aws_ota_signature));
			memcpy((u8*)(&aws_ota_target_hdr.FileHdr), pData, sizeof(aws_ota_target_hdr.FileHdr));
			if(aws_ota_target_hdr.FileHdr.HdrNum > 2 || aws_ota_target_hdr.FileHdr.HdrNum <= 0) {
				OTA_PRINT("INVALID IMAGE BLOCK 0\r\n");
				return -1;
			}
			memcpy((u8*)(&aws_ota_target_hdr.FileImgHdr[HdrIdx]), pData+sizeof(aws_ota_target_hdr.FileHdr), AWS_OTA_IMAGE_SIGNATURE_LEN);
			RevHdrLen = (aws_ota_target_hdr.FileHdr.HdrNum * aws_ota_target_hdr.FileImgHdr[HdrIdx].ImgHdrLen) + sizeof(aws_ota_target_hdr.FileHdr);
			if ( !get_ota_tartget_header(pData, RevHdrLen, &aws_ota_target_hdr, ota_target_index) ) {
				OTA_PRINT("Get OTA header failed\n");
				return -1;
			}

			/* obtain the version info from the header and check against current running FW version */
			version = aws_ota_target_hdr.FileHdr.FwVer;
			major = version / 1000000;
			minor = (version - (major*1000000)) / 1000;
			build = (version - (major*1000000) - (minor * 1000))/1;
			if( aws_ota_target_hdr.FileHdr.FwVer <= (APP_VERSION_MAJOR*1000000 + APP_VERSION_MINOR * 1000 + APP_VERSION_BUILD)) {
				OTA_PRINT("\nOTA failed!!!\n");
				OTA_PRINT("New Firmware version(%d,%d,%d) must greater than current firmware version(%d,%d,%d)\n\n",major,minor,build,APP_VERSION_MAJOR,APP_VERSION_MINOR,APP_VERSION_BUILD);
				return -1;
			} else {
				OTA_PRINT("New Firmware version (%d,%d,%d), current firmware version(%d,%d,%d)\n",major,minor,build,APP_VERSION_MAJOR,APP_VERSION_MINOR,APP_VERSION_BUILD);
			}
			aws_ota_target_hdr_get = true;
		} else {
			aws_ota_target_hdr.FileImgHdr[HdrIdx].ImgLen = ota_ctx.lFileHandle;
			aws_ota_target_hdr.FileHdr.HdrNum = 0x1;
			aws_ota_target_hdr.FileImgHdr[HdrIdx].Offset = 0x20;
		}
	}

	// OTA_PRINT("[%s] C->fileSize %d, iOffset: 0x%x: iBlockSize: 0x%x", __FUNCTION__, C->fileSize, ulOffset, ulBlockSize);

	/* check if already downloaded beyond the size, drop additional downloaded data */
	if( aws_ota_imgsz >= aws_ota_target_hdr.FileImgHdr[HdrIdx].ImgLen ) {
		OTA_PRINT("[OTA] image download is already done, dropped, aws_ota_imgsz=0x%X, ImgLen=0x%X\n",aws_ota_imgsz,aws_ota_target_hdr.FileImgHdr[aws_ota_target_hdr.FileHdr.HdrNum].ImgLen);
		return ulBlockSize;
	}

	/* this is the first data block after header */
	if( ulOffset <= aws_ota_target_hdr.FileImgHdr[HdrIdx].Offset ) {
		uint32_t byte_to_write = (ulOffset + ulBlockSize) - aws_ota_target_hdr.FileImgHdr[HdrIdx].Offset;

		pData += (ulBlockSize - byte_to_write);

		if( ulOffset == 0 ) {
			memcpy(aws_ota_target_hdr.Sign[HdrIdx],pData,sizeof(aws_ota_signature));
			memcpy(aws_ota_signature, pData, sizeof(aws_ota_signature));
			memset(pData, 0xff, sizeof(aws_ota_signature)); 
		}

		OTA_PRINT("[OTA] FIRST Write %d bytes @ 0x%x\n", byte_to_write, address);
		if( ota_writestream_user(address, byte_to_write, pData) < 0 ) {
			OTA_PRINT("[%s] Write sector failed\n", __FUNCTION__);
			return -1;
		}
		OTA_PRINT("[%s] ok\n", __FUNCTION__);
		aws_ota_imgsz += byte_to_write;
		return ulBlockSize;
	}

	WriteLen = ulBlockSize;
	offset = ulOffset - aws_ota_target_hdr.FileImgHdr[HdrIdx].Offset;

	/* bounds check for last block */
	if ( (offset + ulBlockSize) >= aws_ota_target_hdr.FileImgHdr[HdrIdx].ImgLen ) {
		if( offset > aws_ota_target_hdr.FileImgHdr[HdrIdx].ImgLen ) {
			return ulBlockSize;
		}
		WriteLen -= (offset + ulBlockSize - aws_ota_target_hdr.FileImgHdr[HdrIdx].ImgLen);
		OTA_PRINT("[OTA] LAST image data arrived %d\n", WriteLen);
	}

	/* write block data for Nth block (N > 1) */
	OTA_PRINT("[OTA] Write %d bytes @ 0x%x \n", WriteLen, address + offset);
	if( ota_writestream_user(address + offset, WriteLen, pData) < 0 ) {
		OTA_PRINT("[%s] Write sector failed\n", __FUNCTION__);
		return -1;
	}

	aws_ota_imgsz += WriteLen;

	return ulBlockSize;
}

OtaPalStatus_New_t prvPAL_Streams_ActivateNewImage_rtl8721d(void)
{
	flash_t flash;
	OTA_PRINT("[OTA] Download new firmware %d bytes completed @ 0x%x\n", aws_ota_imgsz, aws_ota_imgaddr);
	OTA_PRINT("[OTA] FirmwareSize = %d, OtaTargetHdr.FileImgHdr.ImgLen = %d\n", aws_ota_imgsz, aws_ota_target_hdr.FileImgHdr[HdrIdx].ImgLen);

	/*------------- verify checksum and update signature-----------------*/
	if( verify_ota_checksum(&aws_ota_target_hdr) ) {
		if( !change_ota_signature(&aws_ota_target_hdr, ota_target_index) ) {
			OTA_PRINT("[OTA] change signature failed\n");
			return OtaPalActivateFailed_New;
		} else {
			flash_erase_sector(&flash, AWS_OTA_IMAGE_STATE_FLASH_OFFSET);
			flash_write_word(&flash, AWS_OTA_IMAGE_STATE_FLASH_OFFSET, AWS_OTA_IMAGE_STATE_FLAG_PENDING_COMMIT);
			OTA_PRINT("[OTA] Update OTA success!\n");
		}
	} else {
		/*if checksum error, clear the signature zone which has been written in flash in case of boot from the wrong firmware*/
		flash_erase_sector(&flash, aws_ota_imgaddr - SPI_FLASH_BASE);
		OTA_PRINT("[OTA] The checksum is wrong!\n");
		return OtaPalActivateFailed_New;
	}
	OTA_PRINT("[OTA] Ready to reset MCU to new image!\n");
	//vTaskDelay( 500 );
	//prvPAL_Streams_SysReset_rtl8721d(10);
	return OtaPalSuccess_New;
}

OtaPalStatus_New_t prvPAL_Streams_ResetDevice_rtl8721d ( void )
{
	prvPAL_Streams_SysReset_rtl8721d(10);
	return OtaPalSuccess_New;
}

OtaPalStatus_New_t prvPAL_Streams_SetPlatformImageState_rtl8721d (OtaImageState_New_t eState)
{
	OtaPalStatus_New_t mainErr = OtaPalSuccess_New;
	flash_t flash;

	if ( (eState != OtaImageStateUnknown_New) && (eState <= OtaLastImageState_New) ) {
		/* write state to file */
		flash_erase_sector(&flash, AWS_OTA_IMAGE_STATE_FLASH_OFFSET);
		flash_write_word(&flash, AWS_OTA_IMAGE_STATE_FLASH_OFFSET, eState);
	} else { /* Image state invalid. */
		OTA_PRINT("[%s] Invalid image state provided.", __FUNCTION__);
		mainErr = OtaPalBadImageState_New;
	}

	return mainErr;
}

OtaPalImageState_New_t prvPAL_Streams_GetPlatformImageState_rtl8721d( void )
{
	OtaPalImageState_New_t eImageState = OtaPalImageStateUnknown_New;
	uint32_t eSavedAgentState  =  OtaImageStateUnknown_New;
	flash_t flash;

	flash_read_word(&flash, AWS_OTA_IMAGE_STATE_FLASH_OFFSET, &eSavedAgentState);

	switch ( eSavedAgentState  )
	{
		case OtaImageStateTesting_New:
			/* Pending Commit means we're in the Self Test phase. */
			eImageState = OtaPalImageStatePendingCommit_New;
			break;
		case OtaImageStateAccepted_New:
			eImageState = OtaPalImageStateValid_New;
			break;
		case OtaImageStateRejected_New:
		case OtaImageStateAborted_New:
		default:
			eImageState = OtaPalImageStateInvalid_New;
			break;
	}
	OTA_PRINT( "Image current state (0x%02x).", eImageState );

	return eImageState;
}


/* Read the specified signer certificate from the filesystem into a local buffer. The
 * allocated memory becomes the property of the caller who is responsible for freeing it.
 */
uint8_t * prvPAL_Streams_ReadAndAssumeCertificate_rtl8721d(const uint8_t * const pucCertName, int32_t * const lSignerCertSize)
{
	uint8_t*	pucCertData;
	uint32_t	ulCertSize;
	uint8_t 	*pucSignerCert = NULL;

	extern BaseType_t PKCS11_PAL_GetObjectValue( const char * pcFileName, uint8_t ** ppucData, uint32_t * pulDataSize );

	if ( PKCS11_PAL_GetObjectValue( (const char *) pucCertName, &pucCertData, &ulCertSize ) != pdTRUE ) {	/* Use the back up "codesign_keys.h" file if the signing credentials haven't been saved in the device. */
		pucCertData = (uint8_t*) otapalconfigCODE_SIGNING_CERTIFICATE;
		ulCertSize = sizeof( otapalconfigCODE_SIGNING_CERTIFICATE );
		OTA_PRINT("Assume Cert - No such file: %s. Using header file", (const char*)pucCertName );
	} else {
		OTA_PRINT( "Assume Cert - file: %s OK", (const char*)pucCertName );
	}

	/* Allocate memory for the signer certificate plus a terminating zero so we can load it and return to the caller. */
	pucSignerCert = pvPortMalloc( ulCertSize +	1);
	if ( pucSignerCert != NULL ) {
		memcpy( pucSignerCert, pucCertData, ulCertSize );
		/* The crypto code requires the terminating zero to be part of the length so add 1 to the size. */
		pucSignerCert[ ulCertSize ] = '\0';
		*lSignerCertSize = ulCertSize + 1;
	}
	return pucSignerCert;
}

OtaPalStatus_New_t prvPAL_Streams_CheckFileSignature_rtl8721d(AfrOtaJobDocumentFields_t * const C)
{
	OtaPalStatus_New_t mainErr = OtaPalSuccess_New;

	int32_t lSignerCertSize;
	void *pvSigVerifyContext;
	uint8_t *pucSignerCert = NULL;

#if (defined(__ICCARM__))
	extern void *calloc_freertos(size_t nelements, size_t elementSize);
	mbedtls_platform_set_calloc_free(calloc_freertos, vPortFree);
#endif

	/* Verify an ECDSA-SHA256 signature. */
	if ( CRYPTO_SignatureVerificationStart( &pvSigVerifyContext, cryptoASYMMETRIC_ALGORITHM_ECDSA, cryptoHASH_ALGORITHM_SHA256) == pdFALSE ) {
		mainErr = OtaPalSignatureCheckFailed_New;
		goto exit;
	}

	OTA_PRINT("[%s] Started %s signature verification, file: %s", __FUNCTION__, OTA_SIG_KEY_STR, (const char *)C->certfile);
	if ( (pucSignerCert = prvPAL_Streams_ReadAndAssumeCertificate_rtl8721d((const uint8_t *const)C->certfile, &lSignerCertSize)) == NULL ) {
		mainErr = OtaPalBadSignerCert_New;
		goto exit;
	}

	
	if ( prvPAL_Streams_SignatureVerificationUpdate_rtl8721d(C, pvSigVerifyContext) != OtaPalSuccess_New ) {
		mainErr = OtaPalSignatureCheckFailed_New;
		goto exit;
	}

	if ( CRYPTO_SignatureVerificationFinal(pvSigVerifyContext, (char *)pucSignerCert, lSignerCertSize, (uint8_t *)C->signature, C->signatureLen) == pdFALSE ) {
		mainErr = OtaPalSignatureCheckFailed_New;
		prvPAL_Streams_SetPlatformImageState_rtl8721d(OtaImageStateRejected_New);
		goto exit;
	}

exit:
	/* Free the signer certificate that we now own after prvPAL_Streams_ReadAndAssumeCertificate(). */
	if ( pucSignerCert != NULL ) {
		vPortFree(pucSignerCert);
	}
	return mainErr;
}