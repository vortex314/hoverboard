//#include <tinycbor.h>

#include <RedisSpine.h>
#include <StringUtility.h>
#include <PPP.h>
#include <Uart.h>
#include <As5600.h>
#include <I2C.h>
#include <properties.h>

Thread *spineThread;
RedisSpine *spine;
extern "C" void controlLoop();
extern "C" void controlInit();
extern "C" void softWatchdogReset();
extern "C" void inactivityReset();

TimerSource *controlTimer;
TimerSource *reportTimer;
TimerSource *watchdogTimer;

Uart *uart2;
As5600 *as5600;
PPP *ppp;
extern UART_HandleTypeDef huart2;
Sink<int> *reportSpeed;

extern "C" Properties properties;

uint32_t counter = 0;

Log logger;

extern "C" void app_main_init()
{
    spineThread = new Thread("spineThread");
    uart2 = new Uart(*spineThread, &huart2);
    as5600 = new As5600(I2C::create(0, 0));

    uart2->init();
    as5600->init();
    as5600->onFailure(NULL, [](void *pv, const char *s) { /*WARN(" StepperServo AS5600 : %s ", s);*/ });
    INFO("app_main() entry");
    Sys::hostname("hover");
    spine = new RedisSpine(*spineThread);
    ppp = new PPP(*spineThread, FRAME_MAX_SIZE);

    uart2->rxd() >> ppp->deframe() >> spine->rxdFrame;
    spine->txdFrame >> ppp->frame() >> uart2->txd();
    ppp->garbage() >> [](const Bytes &bs) { WARN("garbage [%d] ", bs.size()); };
  //  uart2->rxd() >> [](const Bytes& bs){ WARN("rxd [%d]", bs.size()); };

    controlTimer = new TimerSource(*spineThread, 50, true, "controlTimer");
    reportTimer = new TimerSource(*spineThread, 100, true, "reportTimer");
    watchdogTimer = new TimerSource(*spineThread, 3000, true, "watchdogTimer");
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
        switch (counter % 11)
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
        case 10:
        {
            spine->publish<int>("src/hover/motor/angle", as5600->degrees());
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
