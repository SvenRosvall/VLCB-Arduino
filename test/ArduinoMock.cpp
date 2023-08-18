#include <map>
#include <Arduino.h>
#include "Arduino.hpp"
#include "ArduinoMock.hpp"

/* Mocking implementation */

std::map<int, int> analogReadValues;
void setAnalogRead(int pin, int value)
{
        analogReadValues[pin] = value;
}

std::map<int, PinState> digitalReadValues;
void setDigitalRead(int pin, PinState value)
{
        digitalReadValues[pin] = value;
}

std::map<int, int> analogWrittenValues;
int getAnalogWrite(int pin)
{
        return analogWrittenValues[pin];
}

std::map<int, PinState> digitalWrittenValues;
PinState getDigitalWrite(int pin)
{
        return digitalWrittenValues[pin];
}

unsigned long nextMillis;
void addMillis(unsigned long newMillis)
{
        nextMillis += newMillis;
}

void clearArduinoValues()
{
        digitalReadValues.clear();
        analogWrittenValues.clear();
        nextMillis = 0L;
}

/* Arduino methods */

void analogWrite(int pin, int value)
{
        analogWrittenValues[pin] = value;
}
int analogRead(int pin)
{
        return analogReadValues[pin];
}
void digitalWrite(int pin, PinState value)
{
	digitalWrittenValues[pin] = value;
}
byte digitalRead(int pin)
{
        return digitalReadValues[pin];
}
void pinMode(int pin, PinMode mode)
{
}

unsigned long millis()
{
        return nextMillis;
}

int map(int value, int fromLower, int fromUpper, int toLower, int toUpper)
{
        return (value - fromLower) * (toUpper - toLower) / (fromUpper - fromLower) + toLower;
}

//struct Print Serial;

void Serial::begin(int baudRate)
{
}
void Serial::println(const char *)
{
}

// Hardware values used by ADC free running mode.
uint8_t DIDR0;
uint8_t ADMUX;
uint8_t ADCSRA;
uint8_t ADCSRB;
uint8_t ADCL;
uint8_t ADCH;

void sei()
{
}

void bitWrite(int, int, int)
{
}
