#include <pigpio.h>
#include "led.h"

LED::LED()
{
    io_pin = -1;
}

LED::LED(int pin)
{
    Init(pin);
}

LED::~LED()
{
    Release();
}

void LED::Release()
{
    if(io_pin >=0)
        Off();
    io_pin = -1;
}

void LED::Init(int pin)
{
    io_pin = pin;
    gpioSetMode(io_pin, PI_OUTPUT);
    gpioSetPullUpDown(io_pin, PI_PUD_DOWN);
    Off();
}

void LED::Set(bool on)
{
    if(on)
        On();
    else
        Off();
}

void LED::On()
{
    status = true;
    gpioWrite(io_pin, 1);
}

void LED::Off()
{
    status = false;
    gpioWrite(io_pin, 0);
}

void LED::Toggle()
{
    Set(!status);
}