/**
 ******************************************************************************
 *This file contains general configurations for ameba platform
 ******************************************************************************
*/

#ifndef __PLATFORM_OPTS_AMAZON_H__
#define __PLATFORM_OPTS_AMAZON_H__

/* For Amazon FreeRTOS SDK example */
#define CONFIG_EXAMPLE_AMAZON_FREERTOS 		1
#define CONFIG_EXAMPLE_AMAZON_AFQP_TESTS	0

#define AUTO_RECONNECT_COUNT	999

#if (defined(CONFIG_EXAMPLE_AMAZON_FREERTOS) && CONFIG_EXAMPLE_AMAZON_FREERTOS) || \
    (defined(CONFIG_EXAMPLE_AMAZON_AFQP_TESTS) && CONFIG_EXAMPLE_AMAZON_AFQP_TESTS)
#undef CONFIG_INCLUDE_SIMPLE_CONFIG
#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#undef CONFIG_FAST_DHCP
#define CONFIG_INCLUDE_SIMPLE_CONFIG		0
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT	1
#define CONFIG_FAST_DHCP 1
#if defined(CONFIG_BUILD_SECURE)
#define ENABLE_AMAZON_COMMON
#endif
#if defined(CONFIG_BUILD_NONSECURE) && (CONFIG_SSL_CLIENT_PRIVATE_IN_TZ == 0)
#undef SUPPORT_LOG_SERVICE
#define SUPPORT_LOG_SERVICE					0
#endif
#endif

#if defined(CONFIG_PLATFORM_8710C)
/**
 * For Amazon FreeRTOS usage
 */
// OTA use
#define AWS_OTA_IMAGE_STATE_FLASH_OFFSET     0x00003000 // Flash reserved section 0x0000_3000 - 0x0000_4000-1

// PKCS11 use
#define pkcs11OBJECT_CERT_FLASH_OFFSET       (0x200000 - 0xB000) //Flash location for CERT
#define pkcs11OBJECT_PRIV_KEY_FLASH_OFFSET   (0x200000 - 0xC000) //Flash location for Priv Key
#define pkcs11OBJECT_PUB_KEY_FLASH_OFFSET    (0x200000 - 0xD000) //Flash location for Pub Key
#define pkcs11OBJECT_VERIFY_KEY_FLASH_OFFSET (0x200000 - 0xE000) //Flash location for code verify Key

#endif /* CONFIG_PLATFORM_87XXX */

#endif /* __PLATFORM_OPTS_AMAZON_H__ */
