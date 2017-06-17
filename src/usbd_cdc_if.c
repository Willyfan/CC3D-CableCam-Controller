/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @brief          :
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"
/* USER CODE BEGIN INCLUDE */
/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_CDC
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_CDC_Private_TypesDefinitions
  * @{
  */
/* USER CODE BEGIN PRIVATE_TYPES */
#define CDC_TIMEOUT 20000 /* Max number of loops before timeout */
/* USER CODE END PRIVATE_TYPES */
/**
  * @}
  */

/** @defgroup USBD_CDC_Private_Defines
  * @{
  */
/* USER CODE BEGIN PRIVATE_DEFINES */
/* Define size for the receive and transmit buffer over CDC */
/* It's up to user to redefine and/or remove those define */
#define APP_RX_DATA_SIZE  2048
#define APP_TX_DATA_SIZE  2048
#define RXBUFFERSIZE	80

/* USER CODE END PRIVATE_DEFINES */
/**
  * @}
  */

/** @defgroup USBD_CDC_Private_Macros
  * @{
  */
/* USER CODE BEGIN PRIVATE_MACRO */
/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_Private_Variables
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/* Received Data over USB are stored in this buffer       */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/* Send Data over USB CDC are stored in this buffer       */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */

uint8_t tempbuf[7];
uint8_t rxbuffer[RXBUFFERSIZE];
uint32_t bytes_received = 0;
uint32_t bytes_scanned = 0;
uint32_t last_string_start_pos = 0;
uint8_t rxbuffer_overflow = 0;

uint8_t UserRxBuffer[APP_RX_DATA_SIZE];/* Received Data over USB are stored in this buffer */
uint8_t UserTxBuffer[APP_TX_DATA_SIZE];/* Received Data over UART (CDC interface) are stored in this buffer */
uint32_t BuffLength;

uint32_t bytes_sent = 0;
uint32_t bytes_written = 0;
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables
  * @{
  */
  extern USBD_HandleTypeDef hUsbDeviceFS;
/* USER CODE BEGIN EXPORTED_VARIABLES */
/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_Private_FunctionPrototypes
  * @{
  */
static int8_t CDC_Init_FS     (void);
static int8_t CDC_DeInit_FS   (void);
static int8_t CDC_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS  (uint8_t* pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  CDC_Init_FS
  *         Initializes the CDC media low layer over the FS USB IP
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  CDC_DeInit_FS
  *         DeInitializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  CDC_Control_FS
  *         Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch (cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

  case CDC_SET_COMM_FEATURE:

    break;

  case CDC_GET_COMM_FEATURE:

    break;

  case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
  case CDC_SET_LINE_CODING:
		tempbuf[0]=pbuf[0];
		tempbuf[1]=pbuf[1];
		tempbuf[2]=pbuf[2];
		tempbuf[3]=pbuf[3];
		tempbuf[4]=pbuf[4];
		tempbuf[5]=pbuf[5];
		tempbuf[6]=pbuf[6];
    break;

  case CDC_GET_LINE_CODING:
		pbuf[0]=tempbuf[0];
		pbuf[1]=tempbuf[1];
		pbuf[2]=tempbuf[2];
		pbuf[3]=tempbuf[3];
		pbuf[4]=tempbuf[4];
		pbuf[5]=tempbuf[5];
		pbuf[6]=tempbuf[6];
    break;

  case CDC_SET_CONTROL_LINE_STATE:

    break;

  case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  CDC_Receive_FS
  *         Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS (uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
	uint32_t len1 = *Len;
	uint16_t current_pos;

	if (bytes_received + len1 > last_string_start_pos + RXBUFFERSIZE) {
		if (bytes_received > last_string_start_pos + RXBUFFERSIZE) {
			// overflow condition
			rxbuffer_overflow = 1;
			return USBD_OK;
		} else {
			len1 = last_string_start_pos + RXBUFFERSIZE - bytes_received;
		}
	}
	current_pos = bytes_received % RXBUFFERSIZE;
	if (len1 > RXBUFFERSIZE - current_pos) {
		memcpy(&rxbuffer[current_pos], Buf, RXBUFFERSIZE - current_pos);
		memcpy(&rxbuffer[0], &Buf[RXBUFFERSIZE - current_pos], len1 + current_pos - RXBUFFERSIZE);
	} else {
		memcpy(&rxbuffer[current_pos], Buf, len1);
	}
	bytes_received += len1;

	/* Prepare for the next reception of data */
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBuffer);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
  /* USER CODE END 6 */
}


uint16_t USB_ReceiveString(char *ptr, uint16_t maxsize) {
	size_t len;
	uint16_t rel_string_start_pos;

	while (bytes_scanned < bytes_received) {
		CDC_TransmitBuffer((uint8_t *) &rxbuffer[bytes_scanned % RXBUFFERSIZE], 1); // echo input text
		if (rxbuffer[bytes_scanned % RXBUFFERSIZE] == '\n' || rxbuffer[bytes_scanned % RXBUFFERSIZE] == '\r') {
			bytes_scanned++;
			len = bytes_scanned - last_string_start_pos;
			if (len <= maxsize) { // in case the string does not fit into the string buffer, it is ignored
				rel_string_start_pos = last_string_start_pos % RXBUFFERSIZE;
				if (rel_string_start_pos + len <= RXBUFFERSIZE) {
					memcpy(ptr, &rxbuffer[rel_string_start_pos], len);
				} else {
					memcpy(ptr, &rxbuffer[rel_string_start_pos], RXBUFFERSIZE - rel_string_start_pos);
					memcpy(&ptr[rel_string_start_pos], &rxbuffer[0], len + rel_string_start_pos - RXBUFFERSIZE);
				}
				last_string_start_pos = bytes_scanned; // next string starts here
				return len;
			} else {
				last_string_start_pos = bytes_scanned; // next string starts here
			}
		} else {
			bytes_scanned++;
		}
	}
	// No newline char was found
	if (rxbuffer_overflow == 1) {
		bytes_scanned = bytes_received;
		last_string_start_pos = bytes_received;
		rxbuffer_overflow = 0;
	}
	return 0;
}

/**
  * @brief  CDC_Transmit_FS
  *         Data send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be send
  * @param  Len: Number of data to be send (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);

	/* USER CODE END 7 */
	return result;

}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
uint8_t  CDC_TransmitBuffer(uint8_t *ptr, uint32_t len) {
	uint32_t l;
	uint32_t rel_pos = bytes_written % APP_TX_DATA_SIZE;
	uint32_t tick = HAL_GetTick();
	if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED && len < APP_RX_DATA_SIZE) {

		while (bytes_written + len >= bytes_sent + APP_RX_DATA_SIZE) {
			/*
			 * wait for the USB bus to send the data
			 * The USBPeriodElapsed() method is called in 20ms intervals by the systick interrupt
			 * As the USBPeriodElapsed() counts the bytes_sent upwards even if there is a problem, the condition should be true
			 * after a view ms. But adding another exit condition just to be sure.
			 */
			if (HAL_GetTick() - tick > 1000) {
				return USBD_FAIL;
			}
		}
		if (rel_pos + len > APP_TX_DATA_SIZE) {
			l = APP_TX_DATA_SIZE - rel_pos;
			memcpy(&UserTxBuffer[rel_pos], ptr, l);
			memcpy(UserTxBuffer, &ptr[l], len - l);
		} else {
			memcpy(&UserTxBuffer[rel_pos], ptr, len);
		}
		bytes_written += len;
	}
	return USBD_OK;
}

uint8_t  CDC_TransmitString(char *ptr) {
	return CDC_TransmitBuffer((uint8_t *)ptr, strlen(ptr));
}


/**
 * @brief  TIM period elapsed callback
 * @param  htim: TIM handle
 * @retval None
 */
void USBPeriodElapsed()
{
	uint32_t buffptr = bytes_sent % APP_TX_DATA_SIZE;
	uint32_t buffsize = bytes_written - bytes_sent;

	if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
		if(bytes_written != bytes_sent)
		{
			if (buffptr + buffsize > APP_TX_DATA_SIZE) {
				buffsize = APP_TX_DATA_SIZE - buffptr;
			}
			USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*)&UserTxBuffer[buffptr], buffsize);

			if(USBD_CDC_TransmitPacket(&hUsbDeviceFS) == USBD_OK)
			{
			}
		}
	}
	bytes_sent += buffsize;
}

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

