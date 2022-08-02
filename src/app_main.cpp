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
//Sink<int> *reportSpeed;

extern "C" Properties properties;
extern int speedR, speedL;
extern  int pwmr, pwml;

uint32_t counter = 0;

Log logger;

float lastTime = 0;
float integral = 0;
float derivative = 0;
float lastError = 0;
float KP = -10.0;
float KI = -1.0;
float KD = 0.;

float PID(float error, float &integral, float &derivative, float &lastError, float &lastTime, float &KP, float &KI, float &KD)
{
    float output;
    float time = Sys::millis() / 1000.0;
    float dt = time - lastTime;
    integral += error * dt;
    derivative = (error - lastError) / dt;
    output = KP * error + KI * integral + KD * derivative;
    lastError = error;
    lastTime = time;
    return output;
}

void controlSteer()
{
    if (as5600)
    {
        properties.angleMeasured = as5600->degrees();
        int delta = PID(properties.angleTarget - properties.angleMeasured, integral, derivative, lastError, lastTime, KP, KI, KD);
        properties.steerTarget = CLAMP(delta,-600,600);
    }
}

typedef struct
{
    char type;
    const char *name;
    void *value;
} Property;

Property properties_list[] = {
    {'c', "src/hover/system/version", (void*)properties.version},
    {'i', "src/hover/motor/angleTarget", &properties.angleTarget},
    {'i', "src/hover/motor/angleMeasured", &properties.angleMeasured},
    {'i', "src/hover/motor/steerTarget", &properties.steerTarget},
    {'i', "src/hover/motor/speedTarget", &properties.speedTarget},
    {'i', "src/hover/motor/speedR", &speedR},
    {'i', "src/hover/motor/speedL", &speedL},
    {'f', "src/hover/motor/speedLeft", &properties.speedLeft},
    {'f', "src/hover/motor/speedRight", &properties.speedRight},
    {'i', "src/hover/motor/pwmr", &pwmr},
    {'i', "src/hover/motor/pwml", &pwml},
    {'u', "src/hover/motor/hallSkippedLeft", &properties.hallSkippedLeft},
    {'u', "src/hover/motor/hallSkippedRight", &properties.hallSkippedRight},
    {'f', "src/hover/motor/voltage", &properties.voltage},
    {'f', "src/hover/motor/temperature", &properties.temperature},
    {'f', "src/hover/motor/currentLeft", &properties.currentLeft},
    {'f', "src/hover/motor/currentRight", &properties.currentRight}};

#define PROPERTIES_LIST_SIZE (sizeof(properties_list) / sizeof(Property))

extern "C" void app_main_init()
{
    spineThread = new Thread("spineThread");
    uart2 = new Uart(*spineThread, &huart2);
    as5600 = new As5600(I2C::create(0, 0));
    properties.version = __DATE__ " " __TIME__;

    uart2->init();
    as5600->init();
    as5600->onFailure([](Error &error)
                      { WARN(" AS5600 failure: %s = %d ", error.message, error.code); });
    INFO("app_main() entry");
    Sys::hostname("hover");
    spine = new RedisSpine(*spineThread);
    ppp = new PPP(*spineThread, FRAME_MAX_SIZE);

    uart2->rxd() >> ppp->deframe() >> spine->rxdFrame;
    spine->txdFrame >> ppp->frame() >> uart2->txd();
    ppp->garbage() >> [](const Bytes &bs)
    { WARN("garbage [%d] ", bs.size()); };

    controlTimer = new TimerSource(*spineThread, 50, true, "controlTimer");
    reportTimer = new TimerSource(*spineThread, 100, true, "reportTimer");
    watchdogTimer = new TimerSource(*spineThread, 3000, true, "watchdogTimer");
    *controlTimer >> [](const TimerMsg &)
    {
        controlLoop();
        controlSteer();
    };

    // stop drive when offline
    *watchdogTimer >> [](const TimerMsg &)
    {
        properties.speedTarget = 0;
        properties.steerTarget = 0;
    };
    controlInit();
    spine->init();
    spine->subscriber<int32_t>("motor/speedTarget") >>
        [&](const int32_t &w)
    {
        properties.speedTarget = w;
        INFO("speed %d", w);
    };
    spine->subscriber<int32_t>("motor/steerTarget") >>
        [&](const int32_t &w)
    {
        properties.steerTarget = w;
        INFO("steer %d", w);
    };

    spine->subscriber<int32_t>("motor/angleTarget") >>
        [&](const int32_t &w)
    {
        properties.angleTarget = CLAMP(w, -90, 90);
        INFO("angleTarget %d", w);
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
        Property *p = &properties_list[counter % PROPERTIES_LIST_SIZE];
        properties.angleMeasured = as5600->degrees();
        switch (p->type)
        {
        case 'i':
            spine->publish<int32_t>(p->name, *((int32_t *)p->value));
            break;
        case 'u':
            spine->publish<uint32_t>(p->name, *((uint32_t *)p->value));
            break;
        case 'f':
            spine->publish<float>(p->name, *((float *)p->value));
            break;
        case 'c': {
            static std::string s = (const char*)p->value;
            spine->publish<std::string>(p->name, s);
            break;
        }
        default:
            break;
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
