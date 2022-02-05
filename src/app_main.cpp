//#include <tinycbor.h>
#include <cbor.h>
#include <CborReader.h>
#include <CborWriter.h>
#include <Spine.h>
#include <Uart.h>
#include <properties.h>

Thread *spineThread;
Spine *spine;
extern "C" void controlLoop();
extern "C" void controlInit();
extern "C" void softWatchdogReset();
extern "C" void inactivityReset();

Thread *controlThread;
TimerSource *controlTimer;
TimerSource *reportTimer;
TimerSource *watchdogTimer;

Uart *uart2;
extern UART_HandleTypeDef huart2;
Sink<int> *reportSpeed;

extern "C" Properties properties;

uint32_t counter = 0;

Log logger;

extern "C" void app_main_init()
{
    spineThread = new Thread("spineThread");
    uart2 = new Uart(*spineThread, &huart2);

    uart2->init();
    INFO("app_main() entry");
    Sys::hostname("hover");
    spine = new Spine(*spineThread);

    uart2->rxdFrame >> spine->rxdFrame;
    spine->txdFrame >> uart2->txdFrame;
    INFO("");

    controlThread = new Thread("controlThread");
    controlTimer = new TimerSource(*controlThread, 5, true, "controlTimer");
    reportTimer = new TimerSource(*controlThread, 100, true, "controlTimer");
    watchdogTimer = new TimerSource(*controlThread, 3000, true, "wathdogTimer");
    *controlTimer >> [](const TimerMsg &)
    { controlLoop(); };
    // stop drive when offline
    *watchdogTimer >> [](const TimerMsg &)
    {
        properties.speedTarget = 0;
        properties.steerTarget = 0;
    };
    controlInit();
    spine->init();
    spine->subscriber<int32_t>("motor/speed") >>
        [&](const int32_t &w)
    {
        properties.speedTarget = w;
        INFO("speed %d", w);
    };
    spine->subscriber<int32_t>("motor/steer") >>
        [&](const int32_t &w)
    {
        properties.steerTarget = w;
        INFO("steer %d", w);
    };
    // stop drive when offline
    spine->subscriber<bool>("motor/watchdogReset") >>
        [&](const int32_t &w)
    {
        watchdogTimer->reset();
        inactivityReset();
    };
    *reportTimer >> [&](const TimerMsg &)
    {
        counter++;
        switch (counter % 10)
        {
        case 0:
        {
            spine->publish<uint32_t>("src/hover/motor/speedTarget", properties.speedTarget);
            break;
        }
        case 1:
        {
            spine->publish<uint32_t>("src/hover/motor/steerTarget", properties.steerTarget);
            break;
        }
        case 2:
        {
            spine->publish<float>("src/hover/motor/speedLeft", properties.speedLeft);
            break;
        }
        case 3:
        {
            spine->publish<float>("src/hover/motor/speedRight", properties.speedRight);
            break;
        }
        case 4:
        {
            spine->publish<uint32_t>("src/hover/motor/hallSkippedLeft", properties.hallSkippedLeft);
            break;
        }
        case 5:
        {
            spine->publish<uint32_t>("src/hover/motor/hallSkippedRight", properties.hallSkippedRight);
            break;
        }
        case 6:
        {
            spine->publish<float>("src/hover/motor/voltage", properties.voltage);
            break;
        }
        case 7:
        {
            spine->publish<float>("src/hover/motor/temperature", properties.temperature);
            break;
        }
        case 8:
        {
            spine->publish<float>("src/hover/motor/currentLeft", properties.currentLeft);
            break;
        }
        case 9:
        {
            spine->publish<float>("src/hover/motor/currentRight", properties.currentRight);
            break;
        }
        }
    };
    INFO("app_main() exit");
    //   uart2->init();
}

extern "C" void app_main_loop()
{
    INFO("thread loop ");
    while (true)
        Thread::loopAll();
}

extern "C" void HardFault_handler()
{
    while (1)
        ;
}

extern "C" void WWDG_IRQHandler()
{
    while (1)
        ;
}

extern "C" void USB_HP_CAN1_TX_IRQHandler()
{
    while (1)
        ;
}
extern "C" void USB_LP_CAN1_RX0_IRQHandler()
{
    while (1)
        ;
}
extern "C" void CAN1_RX1_IRQHandler()
{
    while (1)
        ;
}
extern "C" void CAN1_SCE_IRQHandler()
{
    while (1)
        ;
}
/*extern "C" void  EXTI9_5_IRQHandler()
{
    while (1)
        ;
}*/
extern "C" void TIM1_BRK_IRQHandler()
{
    while (1)
        ;
}
extern "C" void TIM1_UP_IRQHandler()
{
    while (1)
        ;
}
extern "C" void TIM1_TRG_COM_IRQHandler()
{
    while (1)
        ;
}
extern "C" void TIM1_CC_IRQHandler()
{
    while (1)
        ;
}
extern "C" void TIM2_IRQHandler()
{
    while (1)
        ;
}
/*extern "C" void  TIM3_IRQHandler()
{
    while (1)
        ;
}*/
/*extern "C" void  TIM4_IRQHandler()
{
    while (1)
        ;
}*/
extern "C" void I2C1_EV_IRQHandler()
{
    while (1)
        ;
}
extern "C" void I2C1_ER_IRQHandler()
{
    while (1)
        ;
}
extern "C" void I2C2_EV_IRQHandler()
{
    while (1)
        ;
}
extern "C" void I2C2_ER_IRQHandler()
{
    while (1)
        ;
}
extern "C" void SPI1_IRQHandler()
{
    while (1)
        ;
}
extern "C" void SPI2_IRQHandler()
{
    while (1)
        ;
}
extern "C" void USART1_IRQHandler()
{
    while (1)
        ;
}
/*extern "C" void  USART2_IRQHandler()
{
    while (1)
        ;
}*/
extern "C" void USART3_IRQHandler()
{
    while (1)
        ;
}
/*extern "C" void  EXTI15_10_IRQHandler()
{
    while (1)
        ;
}*/
extern "C" void RTC_Alarm_IRQHandler()
{
    while (1)
        ;
}
extern "C" void USBWakeUp_IRQHandler()
{
    while (1)
        ;
}
extern "C" void TIM8_BRK_IRQHandler()
{
    while (1)
        ;
}
extern "C" void TIM8_UP_IRQHandler()
{
    while (1)
        ;
}
extern "C" void TIM8_TRG_COM_IRQHandler()
{
    while (1)
        ;
}
extern "C" void TIM8_CC_IRQHandler()
{
    while (1)
        ;
}
extern "C" void ADC3_IRQHandler()
{
    while (1)
        ;
}
extern "C" void FSMC_IRQHandler()
{
    while (1)
        ;
}
extern "C" void SDIO_IRQHandler()
{
    while (1)
        ;
}
extern "C" void TIM5_IRQHandler()
{
    while (1)
        ;
}
extern "C" void SPI3_IRQHandler()
{
    while (1)
        ;
}
extern "C" void UART4_IRQHandler()
{
    while (1)
        ;
}
extern "C" void UART5_IRQHandler()
{
    while (1)
        ;
}
extern "C" void TIM6_IRQHandler()
{
    while (1)
        ;
}
extern "C" void TIM7_IRQHandler()
{
    while (1)
        ;
}
extern "C" void DMA2_Channel1_IRQHandler()
{
    while (1)
        ;
}
extern "C" void DMA2_Channel2_IRQHandler()
{
    while (1)
        ;
}
extern "C" void DMA2_Channel3_IRQHandler()
{
    while (1)
        ;
}
extern "C" void DMA2_Channel4_5_IRQHandler()
{
    while (1)
        ;
}
