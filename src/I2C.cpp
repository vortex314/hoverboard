#include <I2C.h>
extern I2C_HandleTypeDef hi2c2;

class I2C_STM32 : public I2C
{
  std::string _txd;
  std::string _rxd;

  PhysicalPin _scl;
  PhysicalPin _sda;
  uint32_t _clock;
  uint8_t _slaveAddress;
  uint32_t _timeout;

  FunctionPointer _onTxd = 0;
  I2C_HandleTypeDef _hi2c;

public:
  I2C_STM32();
  ~I2C_STM32();
  int init();
  int deInit();

  int setClock(uint32_t clock)
  {
    _clock = clock;
    return 0;
  }
  int setSlaveAddress(uint8_t slaveAddress)
  {
    _slaveAddress = slaveAddress;
    return 0;
  }

  int write(uint8_t *data, uint32_t size);
  int read(uint8_t *data, uint32_t size);
  int write(uint8_t data);
};

I2C_STM32::I2C_STM32()
{
  _hi2c = hi2c2;
  _clock = 100000;
  _slaveAddress = 0x1E; // HMC 5883L
  _timeout = 1;
}

I2C_STM32::~I2C_STM32()
{
}

int I2C_STM32::init()
{
  I2C_Init();
  return 0;
}

int I2C_STM32::deInit() { return 0; }

int I2C_STM32::write(uint8_t *data, uint32_t size)
{
  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&_hi2c, _slaveAddress, data, size, _timeout);
  if (status == HAL_OK)
    return 0;
  if (status == HAL_TIMEOUT)
    return ETIMEDOUT;
  return EFAULT;
}

int I2C_STM32::write(uint8_t b) { return write(&b, 1); }

int I2C_STM32::read(uint8_t *data, uint32_t size)
{
  HAL_StatusTypeDef status = HAL_I2C_Master_Receive(&_hi2c, _slaveAddress, data, size, _timeout);
  if (status == HAL_OK)
    return 0;
  if (status == HAL_TIMEOUT)
    return ETIMEDOUT;
  return EFAULT;
}

I2C &I2C::create(PhysicalPin scl, PhysicalPin sda)
{
  I2C_STM32 *ptr = new I2C_STM32();
  return *ptr;
}