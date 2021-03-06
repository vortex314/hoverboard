#include <As5600.h>
#include <StringUtility.h>

#define STRINGIFY(X) #X
#define S(X) STRINGIFY(X)
#define VALIDATE(xxx)       \
  {                         \
    int erc = xxx;          \
    if (erc)                \
    {                       \
      failure({erc, __FILE__ ":" S(__LINE__) "=>"  #xxx}); \
      return erc;           \
    }                       \
  }

As5600::As5600(I2C &i2c) : _i2c(i2c) {}
As5600::As5600(Uext &connector) : _i2c(connector.getI2C()) {}
int As5600::init()
{
  VALIDATE(_i2c.init());
  VALIDATE(_i2c.setSlaveAddress(I2C_AS5600));
  return 0;
}

uint8_t As5600::readReg8(uint8_t address)
{
  uint8_t reg[20];
  VALIDATE(_i2c.write(address));
  VALIDATE(_i2c.read(reg, 1));
  return reg[0];
}

uint16_t As5600::readReg16(uint8_t address)
{
  uint8_t reg[10];
  VALIDATE(_i2c.write(address));
  VALIDATE(_i2c.read(reg, 3)); // read more bytes than necessary to avoid STM32 bug :-(
  return (reg[0] << 8) | reg[1];
}

int As5600::writeReg8(uint8_t address, uint8_t value)
{
  uint8_t reg[20];
  reg[0] = address;
  reg[1] = value;
  VALIDATE(_i2c.write(reg, 2));
  return 0;
}

int As5600::writeReg16(uint8_t address, uint16_t value)
{
  uint8_t reg[20];
  reg[0] = address;
  reg[1] = value << 8;
  reg[2] = value & 0xFF;
  VALIDATE(_i2c.write(reg, 3));
  return 0;
}

uint16_t As5600::rawAngle() { return readReg16(AS_RAW_ANGLE); }
uint16_t As5600::angle() { return readReg16(AS_ANGLE); }
uint16_t As5600::zpos() { return readReg16(AS_ZPOS); }
uint16_t As5600::mpos() { return readReg16(AS_MPOS); }
uint8_t As5600::status() { return readReg8(AS_STATUS); }
uint16_t As5600::mang() { return readReg16(AS_MANG); }
uint16_t As5600::conf() { return readReg16(AS_CONF); }
uint16_t As5600::magnitude() { return readReg16(AS_MAGNITUDE); }
uint8_t As5600::agc() { return readReg8(AS_AGC); }
int As5600::degrees() { return ((angle() * 360.0) / 4096) - 180; }
