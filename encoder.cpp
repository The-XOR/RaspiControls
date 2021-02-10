#include <pigpio.h>
#include "encoder.h"

RotaryEncoder::RotaryEncoder(int _dt, int _clk)
{
   Init(_dt, _clk);
}

RotaryEncoder::RotaryEncoder()
{
   dt = clk = -1;
   levA = levB = lastGpio = 0;
}

void RotaryEncoder::Init(int _dt, int _clk)
{
   dt = _dt;
   clk =_clk;
   levA = levB = lastGpio = 0;
   setGPIOs();
}

RotaryEncoder::~RotaryEncoder()
{
   Release();
}

/*

             +---------+         +---------+      0
             |         |         |         |
   A         |         |         |         |
             |         |         |         |
   +---------+         +---------+         +----- 1

       +---------+         +---------+            0
       |         |         |         |
   B   |         |         |         |
       |         |         |         |
   ----+         +---------+         +---------+  1

*/

void RotaryEncoder::_cback(int gpio, int level, uint32_t tick, void *user)
{
   ((RotaryEncoder *)user)->onEvent(gpio, level, tick);
}

void RotaryEncoder::onEvent(int gpio, int level, uint32_t tick)
{
   if(dt < 0 || clk < 0)
      return;

   if (gpio == dt) 
      levA = level; 
   else 
      levB = level;

   if (gpio != lastGpio) /* debounce */
   {
      lastGpio = gpio;

      if ((gpio == dt) && (level == 1))
      {
         if (levB) 
            onEncoder(1);
      } else if ((gpio == clk) && (level == 1))
      {
         if (levA) 
            onEncoder(-1);
      }
   }
}

void RotaryEncoder::setGPIOs()
{
   gpioSetMode(dt, PI_INPUT);
   gpioSetMode(clk, PI_INPUT);
   /* pull up is needed as encoder common is grounded */
   gpioSetPullUpDown(dt, PI_PUD_UP);
   gpioSetPullUpDown(clk, PI_PUD_UP);
   gpioSetAlertFuncEx(dt, _cback, this);
   gpioSetAlertFuncEx(clk, _cback, this);
}

void RotaryEncoder::Release()
{
   if(dt >= 0)
   {
      gpioSetAlertFunc(dt, 0);
      gpioSetAlertFunc(clk, 0);
      dt = clk = -1;
   }
}

