#pragma once
#include "fonts.h"

typedef char (*FontTableLookupFunction)(const uint8_t ch);
char DefaultFontTableLookup(const uint8_t ch);

class DisplayDriver
{
	public:
		enum PixelColor
		{
			WHITE,
			BLACK,
			INVERSE
		};

		enum OLEDDISPLAY_TEXT_ALIGNMENT 
		{
			TEXT_ALIGN_LEFT = 0,
			TEXT_ALIGN_RIGHT = 1,
			TEXT_ALIGN_CENTER = 2,
			TEXT_ALIGN_CENTER_BOTH = 3
		};

		DisplayDriver();
		DisplayDriver(int addr, int width, int height, int port=1);
		~DisplayDriver();

		void Release();
		bool Init(int addr, int width, int height, int port=1);
		void Print(int col, int row, const char *str, PixelColor color= WHITE);
		void Println(int col, int row, int fntsize, const char *txt, PixelColor color= WHITE);
		void Render(unsigned char *from, int len);
		void Render();
		void Clr();
		void Erase();
		void SetPixel(int x, int y, PixelColor color = WHITE);
		void Line(int x1, int y1, int x2, int y2, PixelColor color=WHITE);
		void InvertLine(int x1, int y1, int x2, int y2);
		void Box(int x1, int y1, int x2, int y2, PixelColor color=WHITE);
		void FillArea(int x1, int y1, int x2, int y2, PixelColor color = WHITE);
		void InvertArea(int x1, int y1, int w, int h);
		void Circle(int x, int y, int radius, PixelColor color=WHITE);
		void FillCircle(int x, int y, int radius, PixelColor color = WHITE);
		void Flip();
		void Cleanln(int line);
		void DrawInfoBar(int inL, int inR, int outL, int outR);
   		void SetTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT textAlignment);
		void SetFont(FontSize sz);

	private:
    	OLEDDISPLAY_TEXT_ALIGNMENT   textAlignment;
		unsigned char *frame;
		unsigned char *buffered_frame;
		int frameSize;
		int i2cd;
		int i2c_address;
		int width;
		int height;
		int port;
		int pages;
		const uint8_t *fontData;

	private:
		void sendCommand(int command);
		bool openi2c();
		char defaultFontTableLookup(const uint8_t ch);
		void drawHorizontalLine(int x, int y, int length, PixelColor color);
		int getStringWidth(const char* text);
		void print_line(int col, int row, const char* text, PixelColor color);
		void drawInternal(int col, int row, int W, int H, const uint8_t *data, int offset, int bytesInData, PixelColor color) ;
};
