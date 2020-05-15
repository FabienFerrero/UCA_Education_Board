#include "Module.h"

Module::Module(RADIOLIB_PIN_TYPE cs, RADIOLIB_PIN_TYPE irq, RADIOLIB_PIN_TYPE rst) {
  _cs = cs;
  _rx = RADIOLIB_NC;
  _tx = RADIOLIB_NC;
  _irq = irq;
  _rst = rst;
  _spi = &RADIOLIB_DEFAULT_SPI;
  _spiSettings = SPISettings(2000000, MSBFIRST, SPI_MODE0);
  _initInterface = true;
}

Module::Module(RADIOLIB_PIN_TYPE cs, RADIOLIB_PIN_TYPE irq, RADIOLIB_PIN_TYPE rst, RADIOLIB_PIN_TYPE gpio) {
  _cs = cs;
  _rx = gpio;
  _tx = RADIOLIB_NC;
  _irq = irq;
  _rst = rst;
  _spi = &RADIOLIB_DEFAULT_SPI;
  _spiSettings = SPISettings(2000000, MSBFIRST, SPI_MODE0);
  _initInterface = true;
}

Module::Module(RADIOLIB_PIN_TYPE rx, RADIOLIB_PIN_TYPE tx, HardwareSerial* useSer, RADIOLIB_PIN_TYPE rst) {
  _cs = RADIOLIB_NC;
  _rx = rx;
  _tx = tx;
  _irq = RADIOLIB_NC;
  _rst = rst;
  _initInterface = true;

#ifdef RADIOLIB_SOFTWARE_SERIAL_UNSUPPORTED
  ModuleSerial = useSer;
#else
  ModuleSerial = new SoftwareSerial(_rx, _tx);
  (void)useSer;
#endif
}

Module::Module(RADIOLIB_PIN_TYPE cs, RADIOLIB_PIN_TYPE irq, RADIOLIB_PIN_TYPE rst, SPIClass& spi, SPISettings spiSettings) {
  _cs = cs;
  _rx = RADIOLIB_NC;
  _tx = RADIOLIB_NC;
  _irq = irq;
  _rst = rst;
  _spi = &spi;
  _spiSettings = spiSettings;
  _initInterface = false;
}

Module::Module(RADIOLIB_PIN_TYPE cs, RADIOLIB_PIN_TYPE irq, RADIOLIB_PIN_TYPE rst, RADIOLIB_PIN_TYPE gpio, SPIClass& spi, SPISettings spiSettings) {
  _cs = cs;
  _rx = gpio;
  _tx = RADIOLIB_NC;
  _irq = irq;
  _rst = rst;
  _spi = &spi;
  _spiSettings = spiSettings;
  _initInterface = false;
}

Module::Module(RADIOLIB_PIN_TYPE cs, RADIOLIB_PIN_TYPE irq, RADIOLIB_PIN_TYPE rst, RADIOLIB_PIN_TYPE rx, RADIOLIB_PIN_TYPE tx, SPIClass& spi, SPISettings spiSettings, HardwareSerial* useSer) {
  _cs = cs;
  _rx = rx;
  _tx = tx;
  _irq = irq;
  _rst = rst;
  _spi = &spi;
  _spiSettings = spiSettings;
  _initInterface = false;

#ifdef RADIOLIB_SOFTWARE_SERIAL_UNSUPPORTED
  ModuleSerial = useSer;
#else
  ModuleSerial = new SoftwareSerial(_rx, _tx);
  (void)useSer;
#endif
}

void Module::init(uint8_t interface) {
  // select interface
  switch(interface) {
    case RADIOLIB_USE_SPI:
      Module::pinMode(_cs, OUTPUT);
      Module::digitalWrite(_cs, HIGH);
      if(_initInterface) {
        _spi->begin();
      }
      break;
    case RADIOLIB_USE_UART:
      if(_initInterface) {
#if defined(ESP32)
        ModuleSerial->begin(baudrate, SERIAL_8N1, _rx, _tx);
#else
        ModuleSerial->begin(baudrate);
#endif
      }
      break;
    case RADIOLIB_USE_I2C:
      break;
  }
}

void Module::term() {
  // stop hardware interfaces (if they were initialized by the library)
  if(!_initInterface) {
    return;
  }

  if(_spi != nullptr) {
    _spi->end();
  }

  if(ModuleSerial != nullptr) {
    ModuleSerial->end();
  }
}

void Module::ATemptyBuffer() {
  while(ModuleSerial->available() > 0) {
    ModuleSerial->read();
  }
}

bool Module::ATsendCommand(const char* cmd) {
  ATemptyBuffer();
  ModuleSerial->print(cmd);
  ModuleSerial->print(AtLineFeed);
  return(ATgetResponse());
}

bool Module::ATsendData(uint8_t* data, uint32_t len) {
  ATemptyBuffer();
  for(uint32_t i = 0; i < len; i++) {
    ModuleSerial->write(data[i]);
  }

  ModuleSerial->print(AtLineFeed);
  return(ATgetResponse());
}

bool Module::ATgetResponse() {
  char data[128];
  char* dataPtr = data;
  uint32_t start = millis();
  while(millis() - start < _ATtimeout) {
    while(ModuleSerial->available() > 0) {
      char c = ModuleSerial->read();
      RADIOLIB_VERBOSE_PRINT(c);
      *dataPtr++ = c;
    }

    if(strstr(data, "OK") == 0) {
      RADIOLIB_VERBOSE_PRINTLN();
      return(true);
    } else if(strstr(data, "ERROR") == 0) {
      RADIOLIB_VERBOSE_PRINTLN();
      return(false);
    }

  }
  RADIOLIB_VERBOSE_PRINTLN();
  return(false);
}

int16_t Module::SPIgetRegValue(uint8_t reg, uint8_t msb, uint8_t lsb) {
  if((msb > 7) || (lsb > 7) || (lsb > msb)) {
    return(ERR_INVALID_BIT_RANGE);
  }

  uint8_t rawValue = SPIreadRegister(reg);
  uint8_t maskedValue = rawValue & ((0b11111111 << lsb) & (0b11111111 >> (7 - msb)));
  return(maskedValue);
}

int16_t Module::SPIsetRegValue(uint8_t reg, uint8_t value, uint8_t msb, uint8_t lsb, uint8_t checkInterval) {
  if((msb > 7) || (lsb > 7) || (lsb > msb)) {
    return(ERR_INVALID_BIT_RANGE);
  }

  uint8_t currentValue = SPIreadRegister(reg);
  uint8_t mask = ~((0b11111111 << (msb + 1)) | (0b11111111 >> (8 - lsb)));
  uint8_t newValue = (currentValue & ~mask) | (value & mask);
  SPIwriteRegister(reg, newValue);

  // check register value each millisecond until check interval is reached
  // some registers need a bit of time to process the change (e.g. SX127X_REG_OP_MODE)
  uint32_t start = micros();
  uint8_t readValue = 0;
  while(micros() - start < (checkInterval * 1000)) {
    readValue = SPIreadRegister(reg);
    if(readValue == newValue) {
      // check passed, we can stop the loop
      return(ERR_NONE);
    }
  }

  // check failed, print debug info
  RADIOLIB_DEBUG_PRINTLN();
  RADIOLIB_DEBUG_PRINT(F("address:\t0x"));
  RADIOLIB_DEBUG_PRINTLN(reg, HEX);
  RADIOLIB_DEBUG_PRINT(F("bits:\t\t"));
  RADIOLIB_DEBUG_PRINT(msb);
  RADIOLIB_DEBUG_PRINT(' ');
  RADIOLIB_DEBUG_PRINTLN(lsb);
  RADIOLIB_DEBUG_PRINT(F("value:\t\t0b"));
  RADIOLIB_DEBUG_PRINTLN(value, BIN);
  RADIOLIB_DEBUG_PRINT(F("current:\t0b"));
  RADIOLIB_DEBUG_PRINTLN(currentValue, BIN);
  RADIOLIB_DEBUG_PRINT(F("mask:\t\t0b"));
  RADIOLIB_DEBUG_PRINTLN(mask, BIN);
  RADIOLIB_DEBUG_PRINT(F("new:\t\t0b"));
  RADIOLIB_DEBUG_PRINTLN(newValue, BIN);
  RADIOLIB_DEBUG_PRINT(F("read:\t\t0b"));
  RADIOLIB_DEBUG_PRINTLN(readValue, BIN);
  RADIOLIB_DEBUG_PRINTLN();

  return(ERR_SPI_WRITE_FAILED);
}

void Module::SPIreadRegisterBurst(uint8_t reg, uint8_t numBytes, uint8_t* inBytes) {
  SPItransfer(SPIreadCommand, reg, NULL, inBytes, numBytes);
}

uint8_t Module::SPIreadRegister(uint8_t reg) {
  uint8_t resp = 0;
  SPItransfer(SPIreadCommand, reg, NULL, &resp, 1);
  return(resp);
}

void Module::SPIwriteRegisterBurst(uint8_t reg, uint8_t* data, uint8_t numBytes) {
  SPItransfer(SPIwriteCommand, reg, data, NULL, numBytes);
}

void Module::SPIwriteRegister(uint8_t reg, uint8_t data) {
  SPItransfer(SPIwriteCommand, reg, &data, NULL, 1);
}

void Module::SPItransfer(uint8_t cmd, uint8_t reg, uint8_t* dataOut, uint8_t* dataIn, uint8_t numBytes) {
  // start SPI transaction
  _spi->beginTransaction(_spiSettings);

  // pull CS low
  Module::digitalWrite(_cs, LOW);

  // send SPI register address with access command
  _spi->transfer(reg | cmd);
  #ifdef RADIOLIB_VERBOSE
    if(cmd == SPIwriteCommand) {
      RADIOLIB_VERBOSE_PRINT('W');
    } else if(cmd == SPIreadCommand) {
      RADIOLIB_VERBOSE_PRINT('R');
    }
    RADIOLIB_VERBOSE_PRINT('\t')
    RADIOLIB_VERBOSE_PRINT(reg, HEX);
    RADIOLIB_VERBOSE_PRINT('\t');
  #endif

  // send data or get response
  if(cmd == SPIwriteCommand) {
    for(size_t n = 0; n < numBytes; n++) {
      _spi->transfer(dataOut[n]);
      RADIOLIB_VERBOSE_PRINT(dataOut[n], HEX);
      RADIOLIB_VERBOSE_PRINT('\t');
    }
  } else if (cmd == SPIreadCommand) {
    for(size_t n = 0; n < numBytes; n++) {
      dataIn[n] = _spi->transfer(0x00);
      RADIOLIB_VERBOSE_PRINT(dataIn[n], HEX);
      RADIOLIB_VERBOSE_PRINT('\t');
    }
  }
  RADIOLIB_VERBOSE_PRINTLN();

  // release CS
  Module::digitalWrite(_cs, HIGH);

  // end SPI transaction
  _spi->endTransaction();
}

void Module::pinMode(RADIOLIB_PIN_TYPE pin, RADIOLIB_PIN_MODE mode) {
  if(pin != RADIOLIB_NC) {
    ::pinMode(pin, mode);
  }
}

void Module::digitalWrite(RADIOLIB_PIN_TYPE pin, RADIOLIB_PIN_STATUS value) {
  if(pin != RADIOLIB_NC) {
    ::digitalWrite(pin, value);
  }
}

RADIOLIB_PIN_STATUS Module::digitalRead(RADIOLIB_PIN_TYPE pin) {
  if(pin != RADIOLIB_NC) {
    return(::digitalRead(pin));
  }
  return(LOW);
}

void Module::tone(RADIOLIB_PIN_TYPE pin, uint16_t value) {
  #ifndef RADIOLIB_TONE_UNSUPPORTED
  if(pin != RADIOLIB_NC) {
    ::tone(pin, value);
  }
  #endif
}

void Module::noTone(RADIOLIB_PIN_TYPE pin) {
  #ifndef RADIOLIB_TONE_UNSUPPORTED
  if(pin != RADIOLIB_NC) {
    ::noTone(pin);
  }
  #endif
}
