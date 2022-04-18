#include "Uart.h"

extern Uart *uart2;
void HAL_UART_MspInit(UART_HandleTypeDef *huart);

Uart *fromHandle(UART_HandleTypeDef *huart)
{
	return uart2;
}

Uart::Uart(Thread &thread, UART_HandleTypeDef *huart)
	: Actor(thread), _huart(huart),
	  _rxdAvailable(15, "Uart:rxd"),
	  _txd(7, "Uart:txd"), _rxdData(FRAME_MAX * 2)
{
	_rxdAvailable.async(thread);
	_txd.async(thread);

	_txd >> [this](const Bytes &bs) // send data
	{
		sendBytes(bs);
	};

	_rxdAvailable >> [&](const bool &) // empty circular buffer
	{
		Bytes bs;
		bs.reserve(128);
		while (_rxdData.hasData() && bs.size() < 128)
			bs.push_back(_rxdData.read());
		_rxd.on(bs);
	};

	_rdPtr = 0;
	_wrPtr = 0;
	uart2 = this;
	_dmaTxdDone = true;
}

void HAL_UART2_MspInit(UART_HandleTypeDef *huart);

bool Uart::init()
{
	huart2.Instance = USART2;
	HAL_UART2_MspInit(&huart2);

	__HAL_UART_ENABLE_IT(_huart, UART_IT_IDLE);
	__DMA1_CLK_ENABLE();
	HAL_UART_Receive_DMA(_huart, _rxdBuffer, sizeof(_rxdBuffer));
	return true;
}


// empty DMA buffer


extern "C" void uartSendBytes(uint8_t *data, size_t size, uint32_t retries)
{
	uart2->sendBytes(Bytes(data, data + size));
}

void Uart::sendBytes(Bytes data)
{
	if (_dmaTxdDone == false)
	{
		_txdOverflow++;
		return;
	}
	_dmaTxdDone = false;
	size_t size = data.size() < sizeof(_txdBuffer) ? data.size() : sizeof(_txdBuffer);
	memcpy(_txdBuffer, data.data(), size);
	if (HAL_UART_Transmit_DMA(&huart2, _txdBuffer, size) != HAL_OK)
		_txdOverflow++;
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	fromHandle(huart)->_dmaTxdDone = true;
}

/* This function handles DMA1 stream6 global interrupt. */
extern "C" void DMA1_Stream6_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_usart2_tx);
}

extern "C" void USART2_IRQHandler(void)
{
	/* USER CODE BEGIN USART2_IRQn 0 */
	extern void UART_IDLECallback(UART_HandleTypeDef * huart);
	if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) != RESET)
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart2);
		UART_IDLECallback(&huart2);
	}
	HAL_UART_IRQHandler(&huart2);
}

extern "C" void UART_IDLECallback(UART_HandleTypeDef *huart)
{
	fromHandle(huart)->rxdIrq(huart);
	//	HAL_UART_Receive_DMA(huart, rxBuffer, sizeof(rxBuffer));
}
// get first half of buffer, to be ready before buffer full
extern "C" void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	fromHandle(huart)->rxdIrq(huart);
	//	HAL_UART_Receive_DMA(huart, rxBuffer, sizeof(rxBuffer));
}
// restart DMA as first before getting data when buffer overflows.
// circular mode is activated so will continue to read&write data from the beginning of the buffer
extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	fromHandle(huart)->rxdIrq(huart);
//	HAL_UART_Receive_DMA(huart, fromHandle(huart)->_rxdBuffer, sizeof(Uart::_rxdBuffer));
}


void Uart::rxdIrq(UART_HandleTypeDef *huart)
{
	logger.stop();
	_wrPtr = sizeof(_rxdBuffer) - huart->hdmarx->Instance->CNDTR;
	if (_wrPtr != _rdPtr)
	{
		if (_wrPtr > _rdPtr)
		{
			rxdBytes(_rxdBuffer + _rdPtr, _wrPtr - _rdPtr);
		}
		else
		{
			rxdBytes(_rxdBuffer + _rdPtr, sizeof(_rxdBuffer) - _rdPtr);
			rxdBytes(_rxdBuffer, _wrPtr);
		}
		_rdPtr = _wrPtr;
	} else {
		rxdBytes(_rxdBuffer, 0);
	}
	logger.resume();
}

void Uart::rxdBytes(uint8_t *data, size_t length)
{
	for (size_t i = 0; i < length; i++)
		_rxdData.write(data[i]);
	_rxdAvailable.on(true);
}

extern "C" void DMA1_Channel6_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_usart2_rx);
}

extern "C" void UART_DMAError(DMA_HandleTypeDef *hdma)
{
	while(1);
}



/**
 * @brief This function handles DMA1 channel7 global interrupt.
 */
extern "C" void DMA1_Channel7_IRQHandler(void)
{
	/* USER CODE BEGIN DMA1_Channel7_IRQn 0 */

	/* USER CODE END DMA1_Channel7_IRQn 0 */
	HAL_DMA_IRQHandler(&hdma_usart2_tx);
	/* USER CODE BEGIN DMA1_Channel7_IRQn 1 */

	/* USER CODE END DMA1_Channel7_IRQn 1 */
}

void Error_Handler(const char *file, uint32_t line)
{
	//	static volatile const char *s = file;
	//	static volatile uint32_t l = line;
	while (1)
		;
}

/**
 * @brief UART MSP Initialization
 * This function configures the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART2_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* USER CODE BEGIN USART2_MspInit 0 */

	/* USER CODE END USART2_MspInit 0 */
	/* Peripheral clock enable */
	__HAL_RCC_USART2_CLK_ENABLE();

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/*
	USART2 GPIO Configuration
	PA2     ------> USART2_TX
	PA3     ------> USART2_RX
*/
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 921600;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

	/* USART2 DMA Init */
	/* USART2_RX Init */
	hdma_usart2_rx.Instance = DMA1_Channel6;
	hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;
	hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
	if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

	__HAL_LINKDMA(huart, hdmarx, hdma_usart2_rx);

	/* USART2_TX Init */
	hdma_usart2_tx.Instance = DMA1_Channel7;
	hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_usart2_tx.Init.Mode = DMA_NORMAL;
	hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
	if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

	__HAL_LINKDMA(huart, hdmatx, hdma_usart2_tx);

	/* USART2 interrupt Init */
	HAL_NVIC_SetPriority(USART2_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
	/* DMA interrupt init */
	/* DMA1_Channel6_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
	/* DMA1_Channel7_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
}

/**
 * @brief UART MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
		/* USER CODE BEGIN USART2_MspDeInit 0 */

		/* USER CODE END USART2_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART2_CLK_DISABLE();

		/**USART2 GPIO Configuration
	PA2     ------> USART2_TX
	PA3     ------> USART2_RX
	*/
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);

		/* USART2 DMA DeInit */
		HAL_DMA_DeInit(huart->hdmarx);
		HAL_DMA_DeInit(huart->hdmatx);

		/* USART2 interrupt DeInit */
		HAL_NVIC_DisableIRQ(USART2_IRQn);
		/* USER CODE BEGIN USART2_MspDeInit 1 */

		/* USER CODE END USART2_MspDeInit 1 */
	}
}


