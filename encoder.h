#pragma once
#include "button.h"

class RotaryEncoder 
{
   public:
   virtual ~RotaryEncoder();
   void Release();
   RotaryEncoder(int dt, int clk);
   void Init(int dt, int clk);
   RotaryEncoder();

protected:
   virtual void onEncoder(int way) = 0;

private:
   void setGPIOs();
   void onEvent(int gpio, int level, uint32_t tick);
   static void _cback(int gpio, int level, uint32_t tick, void *user);

   int dt;
   int clk;
   int levA;
   int levB;
   int lastGpio;
};
