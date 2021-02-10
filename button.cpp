#include <pigpio.h>
#include "button.h"
#define ABS(a) ((a)<0 ? -(a) : (a))

Button::Button()
{
   detectDoubleClick=false;
   gpioBtn = -1;
}

Button::Button(int _gpioButton, bool _detectDoubleClick)
{
   Init(_gpioButton), _detectDoubleClick;
}

void Button::Init(int _gpioButton, bool _detectDoubleClick)
{
   gpioBtn = _gpioButton;
   detectDoubleClick = _detectDoubleClick;
   lastBtnDn=0;
   absorbe_up=false;
   last_received=0;
   setGPIOs();
};

Button::~Button()
{
   Release();
}

void Button::_cback(int gpio, int level, uint32_t tick, void *user)
{
   ((Button *)user)->onEvent(gpio, level, tick);
}

void Button::onEvent(int gpio, int level, uint32_t tick)
{
   if(gpioBtn < 0)
      return;

   if(gpio == gpioBtn)
   {
      tick /= 1000; // si passa a MILLI secondi
      bool is_dn = level == 0;
      uint32_t bounce_check = ABS(tick - last_received);

      if(bounce_check>50)
      {
         last_received = tick;
         if(is_dn)
         {
            uint32_t delta = tick-lastBtnDn;
            lastBtnDn=tick;
            if(!detectDoubleClick || delta > dblclick_threshold)
            {
               absorbe_up = false;
               onButtonDown();
            } else
            {
               absorbe_up = true;
               onDoubleClick();
            }
         } else
         {
            if(absorbe_up)
               absorbe_up = false;
            else
               onButtonUp();
         }
      } 
   }
}

void Button::setGPIOs()
{
   gpioSetMode(gpioBtn, PI_INPUT);
   gpioSetPullUpDown(gpioBtn, PI_PUD_UP);
   gpioSetAlertFuncEx(gpioBtn, _cback, this);
}

void Button::Release()
{
   if(gpioBtn >=0)
      gpioSetAlertFunc(gpioBtn, 0);
   gpioBtn = -1;
}

