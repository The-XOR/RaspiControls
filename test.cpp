#include <stdio.h>
#include <pigpio.h>

#include "encoder.h"
#include "button.h"
#include "dispdrv.h"
#include "led.h"

static DisplayDriver oled;
static LED led;

class Button2 : public Button
{
   public:
   Button2(int gpbtn) : Button(gpbtn)
   {
   }

   virtual void onButtonDown() 
   {
      led.On();
   }

   virtual void onDoubleClick() 
   {
      oled.Erase();
         printf("dbl CLICK\n");
         oled.Print(0, 1, "DCLICK");
         oled.Render();
   }

   virtual void onButtonUp() 
   {
      led.Off();
      oled.Clr();
   }
};

class RotaryEncoder2 : public RotaryEncoder
{
   public:
      RotaryEncoder2(int gpioA, int gpioB) : RotaryEncoder(gpioA, gpioB)
      {
         pos = 0;     
      }
   int Position() const {return pos;}

   protected:
      virtual void onEncoder(int way) 
      {
         char buff[100];
         pos += way;
         sprintf(buff, "pos=%d", pos);
         printf("%s\n", buff);
         oled.Erase();
         oled.Print(0, 0, buff);
         oled.Render();
      }

   private:
      int pos;
};

int main(int argc, char *argv[])
{
	const int I2C_ADDRESS = 0x3C;
   if (gpioInitialise() < 0) 
   {
      puts("cannot initialize, for the misery");
      return 1;
   }
   if(!oled.Init(I2C_ADDRESS,128,64))
      puts("Where is the oled?");
   RotaryEncoder2 re(20, 21);
   Button2 b(12);
   led.Init(24);

   puts("pos =-5 to quit");
   while(re.Position() >-5) {

   }
  
   led.Release();
   re.Release();
   b.Release();
   oled.Release();
   gpioTerminate();
}

