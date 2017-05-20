/* 
  LEDParabolaMatrix V1 class by Dan Somrack
  date: 4/16/2017
  
  LEDMatrix V5 class by Aaron Liddiment (c) 2016
  modified:  Juergen Skrotzky (JorgenVikingGod@gmail.com)
  date:      2016/04/27
*/

#ifndef LEDParabolaMatrix_h
#define LEDParabolaMatrix_h
#include <FastLED_GFX.h>
#include <FastLED.h>


class cLEDMatrixBase : public FastLED_GFX //I think this is the base class and everything should be overriden, for example the tiled matrix
{
  friend class cSprite;

  protected:
    int16_t m_Width, m_MaxHeight, m_Size;
	  int16_t *m_Heights; 

    struct CRGB m_OutOfBounds;

  public:
    cLEDMatrixBase(int16_t tMWidth, int16_t tMMaxHeight);
    virtual uint16_t mXY(uint16_t x, uint16_t y)=0;
    struct CRGB *operator[](int n);
    struct CRGB *m_LED;
    struct CRGB &operator()(int16_t x, int16_t y);
    struct CRGB &operator()(int16_t i);

    int Size()  { return(m_Size); }	
    int Width() { return(m_Width);  } 
    int Height()  { return(m_MaxHeight); } //TODO Replace?

	void drawPixel(int16_t x0, int16_t y0, CRGB Col);
    void DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, CRGB Col);
    void DrawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, CRGB Col);
    void DrawCircle(int16_t xc, int16_t yc, uint16_t r, CRGB Col);
    void DrawFilledRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, CRGB Col);
    void DrawFilledCircle(int16_t xc, int16_t yc, uint16_t r, CRGB Col);
};

template<int16_t tMWidth, int16_t tMMaxHeight, int16_t *tMHeights, int16_t tLength> class LEDParabolaMatrix : public cLEDMatrixBase
{
  private:
	struct CRGB p_LED[tLength]; //TODO add up all heights	
	int16_t m_ColumnStart[tMWidth];
	
  public:
    LEDParabolaMatrix():cLEDMatrixBase(tMWidth,tMMaxHeight)
    {	  
      m_Width = tMWidth;
      m_MaxHeight = tMMaxHeight;
	  m_Size = tLength;
	  m_Heights = tMHeights;
      m_LED = p_LED;
	  
	  //Initialize the m_ColumnStart array
	  //m_ColumnStart array value will be the smallest index in that x line  
	  int runningTotal = 0;
	  for(int i = m_Width-1; i>=0; i--)
	  {
		m_ColumnStart[i] = runningTotal;
		runningTotal += m_Heights[i];  
	  }	  
    }	
    
	virtual uint16_t mXY(uint16_t x, uint16_t y)
	{
	//     XY(x,y) takes x and y coordinates and returns an LED index number,
	//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;
	//             No error checking is performed on the ranges of x and y.
	//
		uint16_t i;
	  
		//if even row and there are even number of rows, since 
		if( x % 2 != 0) {
		  // even rows run down. Start will be in the middle or top.
		  i = m_ColumnStart[x] - (m_MaxHeight - m_Heights[x]) + y;
		}
		else {
		  // odd rows run up. start will be at the bottom
		  i = m_ColumnStart[x] + (m_MaxHeight - 1) - y;    
		}
		return i;			
	}	
};

#endif
