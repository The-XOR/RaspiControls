#pragma once

class Button
{
   public:
   Button();
   Button(int gpioButton, bool detectDoubleClick=false);
   virtual ~Button();
   void Release();
   void Init(int gpioButton, bool detectDoubleClick=false);

protected:
   virtual void onButtonDown() {};
   virtual void onButtonUp() {};
   virtual void onDoubleClick() {};

private:
   void setGPIOs();
   void onEvent(int gpio, int level, uint32_t tick);
   static void _cback(int gpio, int level, uint32_t tick, void *user);

   int gpioBtn;
   uint32_t lastBtnDn;
   bool absorbe_up;
   uint32_t last_received;
   bool detectDoubleClick;
   const int dblclick_threshold=400;
};

class PulseButton : public Button
{
   public:
      PulseButton(int gpioButton) : Button(gpioButton) {cur = false;}
      PulseButton() : Button() {cur = false;}
      bool Down() const  {return cur; }

   private:
      virtual void onButtonDown() {cur=true; }
      virtual void onButtonUp()  {cur=false; }

   private:
      bool cur;
};