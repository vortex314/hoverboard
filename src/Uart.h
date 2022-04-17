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

class Uart : public Actor
{
	static const size_t FRAME_MAX = 128;

	UART_HandleTypeDef *_huart;
	Bytes _frameRxd;
	size_t _wrPtr, _rdPtr;
	QueueFlow<Bytes> _rxd;
	QueueFlow<Bytes> _txd;

public:
	uint8_t _rxdBuffer[FRAME_MAX];
	uint8_t _txdBuffer[FRAME_MAX];
	volatile bool txdDMAdone = true;
	uint32_t _txdOverflow = 0;
	uint32_t _rxdOverflow = 0;

	inline Source<Bytes> &rxd()
	{
		return _rxd;
	};
	inline Sink<Bytes> &txd() { return _txd; };

	Uart(Thread &thread, UART_HandleTypeDef *huart);
	bool init();
	void rxdIrq(UART_HandleTypeDef *huart);
	void rxdBytes(uint8_t *, size_t);
	void sendBytes(Bytes data);
};

void uartSendBytes(uint8_t *, size_t, int retries);

#endif /* SRC_UART_H_ */
