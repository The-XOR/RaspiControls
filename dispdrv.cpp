#include <pigpio.h>
#include <memory.h>

#include "dispdrv.h"
#define MIN(a,b) (((a)<(b))?(a):(b))
#define ABS(a) ((a)<0 ? -(a) : (a))
#define pgm_read_byte(addr)   (*(const unsigned char *)(addr))

#define CHARGEPUMP  0x8D
#define COLUMNADDR  0x21
#define COMSCANDEC  0xC8
#define COMSCANINC  0xC0
#define DISPLAYALLON  0xA5
#define DISPLAYALLON_RESUME  0xA4
#define DISPLAYOFF  0xAE
#define DISPLAYON  0xAF
#define EXTERNALVCC  0x1
#define INVERTDISPLAY  0xA7
#define MEMORYMODE  0x20
#define NORMALDISPLAY  0xA6
#define PAGEADDR  0x22
#define SEGREMAP  0xA0
#define SETCOMPINS  0xDA
#define SETCONTRAST  0x81
#define SETDISPLAYCLOCKDIV  0xD5
#define SETDISPLAYOFFSET  0xD3
#define SETHIGHCOLUMN  0x10
#define SETLOWCOLUMN  0x00
#define SETPAGE			0xb0
#define SETMULTIPLEX  0xA8
#define SETPRECHARGE  0xD9
#define SETSEGMENTREMAP  0xA0
#define SETSTARTLINE  0x40
#define SETVCOMDETECT  0xDB
#define SWITCHCAPVCC  0x2
#define ALTERNATE		0x12

bool DisplayDriver::Init(int addr, int w, int h, int p)
{
	i2cd = PI_NO_HANDLE;
	i2c_address = addr;
	width=w;
	height=h;
	port = p;
    pages = int(height / 8);
	frameSize = width * pages;
	frame = new unsigned char[frameSize];
	buffered_frame = new unsigned char[frameSize];
	SetTextAlignment(TEXT_ALIGN_LEFT);
	SetFont(MEDIUM);

	return openi2c();
}

DisplayDriver::~DisplayDriver()
{
	Release();
}

void DisplayDriver::Release()
{
	if(i2cd >= 0)
	{
		Clr();
		i2cClose(i2cd);
		i2cd = PI_NO_HANDLE;
	}

	if(frame)
	{
		delete []frame;
		frame = NULL;
	}

	if(buffered_frame)
	{
		delete []buffered_frame;
		buffered_frame = NULL;
	}
	frameSize=0;
}

DisplayDriver::DisplayDriver(int addr, int w, int h, int p)
{
	Init(addr,w,h,p);
}

DisplayDriver::DisplayDriver()
{
	i2cd = PI_NO_HANDLE;
	i2c_address = width=height=	port = pages=frameSize=0;
	buffered_frame=frame=NULL;
}

void DisplayDriver::SetFont(FontSize sz) 
{
	switch(sz)
	{
		case SMALL: fontData = ArialMT_Plain_10; break;
		case MEDIUM: fontData = ArialMT_Plain_16; break;
		case LARGE: fontData = ArialMT_Plain_24; break;
	}
}

void DisplayDriver::SetTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT textAlignment) 
{
  this->textAlignment = textAlignment;
}

bool DisplayDriver::openi2c()
{
	unsigned char compIns, multiplex, displayoffset;
 
  	switch(height)
	{
		default: return false;
		case 128: compIns=0x22; multiplex=0xFF; displayoffset=0x02; break;
		case 64:  compIns= 0x12; multiplex=0x3F; displayoffset=0x00; break;
		case 32:  compIns = 0x02; multiplex=0x20; displayoffset=0x0F; break;
    }
	
	const bool extVcc=true;

	unsigned char init_command[] =
	{
 		DISPLAYOFF, // 0 disp off
	    SETDISPLAYCLOCKDIV, // 1 clk div
        0x80, // 2 suggested ratio
        SETMULTIPLEX, multiplex, // 3 set multiplex, height-1
        SETDISPLAYOFFSET, displayoffset, // 5 display offset
        SETSTARTLINE | 0x00, // 7 start line
        CHARGEPUMP, extVcc?0x10:0x14, // 8 charge pump
		SETHIGHCOLUMN,
        MEMORYMODE,0x0, // 10 0x0 act like ks0108
        SETSEGMENTREMAP | 0x01, // 12 seg remap 1
    	COMSCANDEC, // 13 comscandec
        SETCOMPINS, compIns, // 14 set compins, height==64 ? 0x12:0x02,
        SETCONTRAST, extVcc?0x9F:0xCF, // 16 set contrast
        SETPRECHARGE, extVcc?0x22:0xF1, // 18 set precharge
        SETVCOMDETECT, 0x40, // 20 set vcom detect
        DISPLAYALLON_RESUME, // 22 display all on
    	NORMALDISPLAY, // 23 display normal (non-inverted)
        DISPLAYON // 24 disp on
	};
 
	i2cd = i2cOpen(port, i2c_address, 0);
	if(i2cd >= 0)
	{
		for(int k=0;k<sizeof(init_command)/sizeof(init_command[0]);k++)
			sendCommand(init_command[k]);
		gpioDelay(100);
		Clr();
	}
	return i2cd>=0;
}

void DisplayDriver::sendCommand(int command)
{
	i2cWriteByteData(i2cd, 0x00, command);
}


void DisplayDriver::drawHorizontalLine(int x, int y, int length, PixelColor color)
{
  if (x < 0) 
  {
    length += x;
    x = 0;
  }

  if ( (x + length) > width) 
    length = (width - x);
  

  if (length > 0) 
  {
	unsigned char *bufferPtr = frame;
	bufferPtr += (y >> 3) * width;
	bufferPtr += x;

	unsigned char drawBit = 1 << (y & 7);

	switch (color) {
		case WHITE:   while (length--) 
		{
			*bufferPtr++ |= drawBit;
		}; 
		break;
		case BLACK:  
		{ 
			drawBit = ~drawBit;   
			while (length--) 
				*bufferPtr++ &= drawBit;
		} 
		break;
		case INVERSE: 
		{
			while (length--)
				*bufferPtr++ ^= drawBit;
		}
		break;
	}
  }
}

void DisplayDriver::Render(unsigned char *from, int len)
{
	memcpy(frame, from, len);
	Render();
}

void DisplayDriver::Render()
{
	if(memcmp(frame, buffered_frame, frameSize) == 0)
		return; // nulla e' cambiato
		
	memcpy(buffered_frame, frame, frameSize),
	sendCommand(SETLOWCOLUMN | 0X00);
	sendCommand(SETSTARTLINE | 0X00);

    sendCommand(PAGEADDR);
    sendCommand(0x0);
    sendCommand((height==64)?0x7:0x03);
    sendCommand(SETSTARTLINE | 0x00);

	int rheight=height;  
	int rwidth=width+4; //ssh1106
	int m_row=0;
	int m_col=2;

	rheight >>=3;
	rwidth >>= 3;

	char *pfr=(char *)frame;

	for(int i=0; i< rheight;i++)
	{
		sendCommand(0xb0+i+m_row); //page address
		sendCommand(m_col & 0x0f); //lower col address
		sendCommand(0x10 | (m_col >>4)); //higher col address

		for(int j=0; j< 8; j++)
		{
			int len = rwidth;
			while(len > 0)
			{
				int blkLen = MIN(32, len);
				i2cWriteI2CBlockData(i2cd, 0x40, pfr, blkLen);
				pfr += blkLen;
				len -= blkLen;
			}	
			/*
			i2cWriteByte(i2cd, 0x40);
			for(int k=0; k<rwidth;k++,ptr++)
				i2cWriteByteData(i2cd, 0x40, frame[ptr]);*/
		}
	}
/*
	for(int i = 0; i < frameSize; i++)		
		i2cWriteByteData(i2cd, 0x40, frame[i]);*/
}


void DisplayDriver::Erase()
{
	memset(frame, 0, frameSize);
}

void DisplayDriver::Clr()
{
	sendCommand(NORMALDISPLAY);
	sendCommand(SETSTARTLINE);
	Erase();
	Render();
}

void DisplayDriver::SetPixel(int x, int y, PixelColor color)
{
	if(x<0  || x>=width || y <0 || y>=height)
		return;
	switch (color) 
	{
      case WHITE:   frame[x + (y / 8) * width] |=  (1 << (y & 7)); break;
      case BLACK:   frame[x + (y / 8) * width] &= ~(1 << (y & 7)); break;
      case INVERSE: frame[x + (y / 8) * width] ^=  (1 << (y & 7)); break;
    }
}

void DisplayDriver::InvertLine(int x0, int y0, int x1, int y1)
{
	Line(x0,y0,x1,y1,INVERSE);
}

void DisplayDriver::Line(int x0, int y0, int x1, int y1, PixelColor color)
{
	int dx =  ABS(x1 - x0);
	int sx = x0 < x1 ? 1 : -1;
  	int dy = -ABS(y1 - y0);
	int sy = y0 < y1 ? 1 : -1; 
  	int err = dx + dy;
 
	for(;;)
	{
		SetPixel(x0,y0,color);
		if(x0 == x1 && y0 == y1) 
			break;
		int e2 = 2 * err;
		if(e2 >= dy) 
		{ 
			err += dy; 
			x0 += sx; 
		}
		if(e2 <= dx) 
		{ 
			err += dx; 
			y0 += sy; 
		}
	}
}

void DisplayDriver::Box(int x0, int y0, int x1, int y1, PixelColor color)
{
	Line(x0,y0,x1,y0, color);
	Line(x1,y0,x1,y1, color);
	Line(x1,y1,x0,y1, color);
	Line(x0,y1,x0,y0, color);
}

void DisplayDriver::FillArea(int x0, int y0, int x1, int y1, PixelColor color)
{
	for(int k=y0;k<y1;k++)
		Line(x0,k,x1,k, color);
}

void DisplayDriver::Circle(int x, int y, int radius, PixelColor color)
{
  	int xr = 0;
	int yr = radius;
	int dp = 1 - radius;
	do 
	{
		if (dp < 0)
			dp = dp + (xr++) * 2 + 3;
		else
			dp = dp + (xr++) * 2 - (yr--) * 2 + 5;

		SetPixel(x + xr, y + yr, color);     //For the 8 octants
		SetPixel(x - xr, y + yr, color);
		SetPixel(x + xr, y - yr, color);
		SetPixel(x - xr, y - yr, color);
		SetPixel(x + yr, y + xr, color);
		SetPixel(x - yr, y + xr, color);
		SetPixel(x + yr, y - xr, color);
		SetPixel(x - yr, y - xr, color);

	} while(xr < yr);

  SetPixel(x + radius, y, color);
  SetPixel(x, y + radius, color);
  SetPixel(x - radius, y, color);
  SetPixel(x, y - radius, color);
}

void DisplayDriver::FillCircle(int x, int y, int radius, PixelColor color)
{
  	int xr = 0;
	int yr = radius;
	int dp = 1 - radius;
	do 
	{
		if (dp < 0)
      		dp = dp + (xr++) * 2 + 3;
    	else
      		dp = dp + (xr++) * 2 - (yr--) * 2 + 5;

		drawHorizontalLine(x - xr, y - yr, 2*xr, color);
		drawHorizontalLine(x - xr, y + yr, 2*xr, color);
		drawHorizontalLine(x - yr, y - xr, 2*yr, color);
		drawHorizontalLine(x - yr, y + xr, 2*yr, color);
	} while (xr < yr);
  	drawHorizontalLine(x - radius, y, 2 * radius, color);
}

void DisplayDriver::Flip()
{
	for(int i = 0; i < frameSize; i++)
		frame[i] ^= frame[i];
	// NO need to render
	sendCommand(INVERTDISPLAY);
}

void DisplayDriver::InvertArea(int x, int y, int w, int h)
{
	int x1=x+w-1;
	int y1=y+h-1;
	FillArea(x,y,x1,y1,INVERSE);
}

void DisplayDriver::Cleanln(int line) //line dovrebbe essere 1-based
{
	int y=(line-1)*12+8;
	FillArea(0, y, width,9, BLACK);
}

void DisplayDriver::DrawInfoBar(int inL, int inR, int outL, int outR)
{
    if(inR < 0)
        inR = 0;
    else if(inR > 11)
        inR = 11;

    if(inL < 0)
        inL = 0;
    else if(inL > 11)
        inL = 11;

 	if(outR < 0)
        outR = 0;
    else if(outR > 11)
        outR = 11;
		
	if(outL < 0)
        outL = 0;
    else if(outL > 11)
        outL = 11;		

    FillArea(0, 0, 128, 8, BLACK);

    Println(0, 0, 8, "I", WHITE);
    Println(64, 0, 8,"O", WHITE);

    for(int i=0; i<11;i++)
	{
        FillArea((i * 5) + 8, 1, 1, 2, WHITE);
        FillArea((i * 5) + 8, 5, 1, 2, WHITE);
	}

    for(int i=0; i<inR;i++)
        FillArea((i * 5) + 7, 0, 3, 4, WHITE);

    for(int i=0; i<inL;i++)
        FillArea((i * 5) + 7, 4, 3, 4, WHITE);

    for(int i=0; i<11;i++)
    {
        FillArea((i * 5) + 74, 1, 1, 2, WHITE);
        FillArea((i * 5) + 74, 5, 1, 2, WHITE);
	}

    for(int i=0; i<outR;i++)
        FillArea((i * 5) + 73, 0, 3, 4, WHITE);
    
	for(int i=0; i<outL;i++)
        FillArea((i * 5) + 73, 4, 3, 4, WHITE);
}

void DisplayDriver::Println(int col, int row, int fntsize, const char *txt, PixelColor color)
{
	SetFont((FontSize)fntsize);
	Print(col, row, txt, color);
}

char DisplayDriver::defaultFontTableLookup(const uint8_t ch) 
{
    // UTF-8 to font table index converter
    // Code form http://playground.arduino.cc/Main/Utf8ascii
	static uint8_t LASTCHAR;

	if (ch < 128) { // Standard ASCII-set 0..0x7F handling
		LASTCHAR = 0;
		return ch;
	}

	uint8_t last = LASTCHAR;   // get last char
	LASTCHAR = ch;

	switch (last) {    // conversion depnding on first UTF8-character
		case 0xC2: return (uint8_t) ch;
		case 0xC3: return (uint8_t) (ch | 0xC0);
		case 0x82: if (ch == 0xAC) return (uint8_t) 0x80;    // special case Euro-symbol
	}

	return (uint8_t) 0; // otherwise: return zero, if character has to be ignored
}

void DisplayDriver::Print(int col, int row, const char *text, PixelColor color) 
{
  int lineHeight = pgm_read_byte(fontData + HEIGHT_POS);
  int len = strlen(text);

  int yOffset = 0;
  if (textAlignment == TEXT_ALIGN_CENTER_BOTH) 
  {
    int lb = 0;
    // Find number of linebreaks in text
    for(int k=0; k < len;k++)
		if(text[k] == '\n')
      		lb++;
    yOffset = (lb * lineHeight) / 2;
  }

  	int line = 0;
	char *tmpAlloc = new char[1+len];
	int ptr = 0;
    for(int k=0; k < len;k++)
	{
	 	tmpAlloc[ptr] = text[k];
		if(text[k] == '\n')
		{
			tmpAlloc[ptr] = 0;
   			print_line(col, (row - yOffset + (line++)) * lineHeight, tmpAlloc, color);
			ptr=0;
		} else
			ptr++;
	}
	if(ptr > 0)
	{
		tmpAlloc[ptr] = 0;
   		print_line(col, (row - yOffset + (line++) )* lineHeight, tmpAlloc, color);
	}

	delete []tmpAlloc;
}

int DisplayDriver::getStringWidth(const char* text)
{
  int firstChar = pgm_read_byte(fontData + FIRST_CHAR_POS);
  int len = strlen(text);

  int stringWidth = 0;
  for(int k=0; k < len;k++)
    stringWidth += pgm_read_byte(fontData + JUMPTABLE_START + (text[k] - firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);

	return stringWidth;
}


void DisplayDriver::print_line(int col, int row, const char* text, PixelColor color) 
{
	int textLength = strlen(text);
	int textWidth = getStringWidth(text);

  int textHeight       = pgm_read_byte(fontData + HEIGHT_POS);
  int firstChar        = pgm_read_byte(fontData + FIRST_CHAR_POS);
  int sizeOfJumpTable = pgm_read_byte(fontData + CHAR_NUM_POS)  * JUMPTABLE_BYTES;

  int cursorX         = 0;
  int cursorY         = 0;

  switch (textAlignment) {
    case TEXT_ALIGN_CENTER_BOTH:
      row -= textHeight >> 1;
    // Fallthrough
    case TEXT_ALIGN_CENTER:
      col -= textWidth >> 1; // divide by 2
      break;
    case TEXT_ALIGN_RIGHT:
      col -= textWidth;
      break;
    case TEXT_ALIGN_LEFT:
      break;
  }

  // Don't draw anything if it is not on the screen.
  if (col + textWidth  < 0 || col > width || row + textHeight < 0 || row > width )
  		return;

  for (int j = 0; j < textLength; j++) 
  {
    int xPos = col + cursorX;
    int yPos = row + cursorY;

    uint8_t code = text[j];
    if (code >= firstChar) 
	{
      uint8_t charCode = code - firstChar;

      // 4 Bytes per char code
      uint8_t msbJumpToChar    = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES );                  // MSB  \ JumpAddress
      uint8_t lsbJumpToChar    = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_LSB);   // LSB /
      uint8_t charByteSize     = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_SIZE);  // Size
      uint8_t currentCharWidth = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_WIDTH); // Width

      // Test if the char is drawable
      if (!(msbJumpToChar == 255 && lsbJumpToChar == 255)) {
        // Get the position of the char data
        int charDataPosition = JUMPTABLE_START + sizeOfJumpTable + ((msbJumpToChar << 8) + lsbJumpToChar);
        drawInternal(xPos, yPos, currentCharWidth, textHeight, fontData, charDataPosition, charByteSize, color);
      }

      cursorX += currentCharWidth;
    }
  }
}

void DisplayDriver::drawInternal(int col, int row, int W, int H, const uint8_t *data, int offset, int bytesInData, PixelColor color) 
{
  if (W < 0 || H < 0) return;
  if (row + H < 0 || row > height)  return;
  if (col + W  < 0 || col > width)   return;

  uint8_t  rasterH = 1 + ((H - 1) >> 3); // fast ceil(H / 8.0)
  int8_t   yOffset      = row & 7;

  bytesInData = bytesInData == 0 ? W * rasterH : bytesInData;

  int initrow   = row;
  int8_t  initYOffset = yOffset;


  for (int i = 0; i < bytesInData; i++) {

    // Reset if next horizontal drawing phase is started.
    if ( i % rasterH == 0) {
      row   = initrow;
      yOffset = initYOffset;
    }

    uint8_t currentByte = pgm_read_byte(data + offset + i);

    int xPos = col + (i / rasterH);
    int yPos = ((row >> 3) + (i % rasterH)) * width;

//    int yScreenPos = row + yOffset;
    int dataPos    = xPos  + yPos;

    if (dataPos >=  0  && dataPos < frameSize &&
        xPos    >=  0  && xPos    < width ) {

      if (yOffset >= 0) {
        switch (color) {
          case WHITE:   frame[dataPos] |= currentByte << yOffset; break;
          case BLACK:   frame[dataPos] &= ~(currentByte << yOffset); break;
          case INVERSE: frame[dataPos] ^= currentByte << yOffset; break;
        }

        if (dataPos < (frameSize - width)) {
          switch (color) {
            case WHITE:   frame[dataPos + width] |= currentByte >> (8 - yOffset); break;
            case BLACK:   frame[dataPos + width] &= ~(currentByte >> (8 - yOffset)); break;
            case INVERSE: frame[dataPos + width] ^= currentByte >> (8 - yOffset); break;
          }
        }
      } else {
        // Make new offset position
        yOffset = -yOffset;

        switch (color) {
          case WHITE:   frame[dataPos] |= currentByte >> yOffset; break;
          case BLACK:   frame[dataPos] &= ~(currentByte >> yOffset); break;
          case INVERSE: frame[dataPos] ^= currentByte >> yOffset; break;
        }

        // Prepare for next iteration by moving one block up
        row -= 8;

        // and setting the new yOffset
        yOffset = 8 - yOffset;
      }
    }
  }
}