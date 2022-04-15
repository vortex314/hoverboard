/*
 * Uart.h
 *
 *  Created on: Nov 21, 2021
 *      Author: lieven
 */

#ifndef SRC_UART_H_
#define SRC_UART_H_

#include "limero.h"
#include "stm32f1xx_hal.h"
#include "Legacy/stm32_hal_legacy.h"
extern "C" UART_HandleTypeDef huart2;
extern "C" DMA_HandleTypeDef hdma_usart2_rx;
extern "C" DMA_HandleTypeDef hdma_usart2_tx;


class Uart: public Actor {
		static const size_t FRAME_MAX = 128;

		UART_HandleTypeDef *_huart;
		Bytes _frameRxd;
		size_t _wrPtr, _rdPtr;


	public:
		uint8_t rxBuffer[FRAME_MAX];
		volatile bool crcDMAdone=true;
		uint32_t _txdOverflow = 0;
		uint32_t _rxdOverflow = 0;

		QueueFlow<Bytes> rxdFrame;
		SinkFunction<Bytes> txdFrame;
		ValueFlow<Bytes> txd;

		Uart(Thread &thread, UART_HandleTypeDef *huart);
		bool init();
		void rxdIrq(UART_HandleTypeDef *huart);
		void rxdBytes(uint8_t*,size_t);
		void sendBytes(const uint8_t*, size_t);

};

void uartSendBytes(uint8_t*, size_t, int retries);

#endif /* SRC_UART_H_ */
