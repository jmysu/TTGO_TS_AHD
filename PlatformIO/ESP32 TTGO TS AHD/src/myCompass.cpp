#include <Arduino.h>

#include <TFT_eSPI.h>                 // Include the graphics library (this includes the sprite functions)
extern TFT_eSPI tft;
//TFT_eSPI    tft = TFT_eSPI();         // Create object "tft"
TFT_eSprite img = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object
                                      // the pointer is used by pushSprite() to push it onto the TFT

#define BACKGROUND TFT_BLACK
// Pause in milliseconds to set refresh speed
#define WAIT 10

int angle = 0; //Compass starting angle
int number = 0;

  int lx1 = 0;
  int ly1 = 0;
  int lx2 = 0;
  int ly2 = 0;
  int lx3 = 0;
  int ly3 = 0;
  int lx4 = 0;
  int ly4 = 0;

// Test only
uint16_t  n = 0;
uint32_t dt = 0;

#define RAD2DEG 0.0174532925
// Get coordinates of end of a vector, centre at x,y, length r, angle a
// Coordinates are returned to caller via the xp and yp pointers
void getCoord(int x, int y, int *xp, int *yp, int r, int a)
{
  float sx1 = cos( (a-90) * RAD2DEG );    
  float sy1 = sin( (a-90) * RAD2DEG );
  *xp =  sx1 * r + x;
  *yp =  sy1 * r + y;
}

// Test code to measure runtimes, executes code 100x and shows time taken
#define TSTART //n=100;dt=millis();while(n--){
#define TPRINT //};Serial.println((millis()-dt)/100.0);

// #########################################################################
// Draw compass using the defined transparent colour (takes ~6ms)
// #########################################################################
void drawCompass(int x, int y, int angle)
{
  TSTART
  img.setColorDepth(8);
  img.createSprite(100, 100);

  // TFT_TRANSPARENT is a special colour with reversible 8/16 bit coding
  // this allows it to be used in both 8 and 16 bit colour sprites.
  img.fillSprite(TFT_TRANSPARENT);

  #define NEEDLE_L 84/2  // Needle length is 84, we want radius which is 42
  #define NEEDLE_W 12/2  // Needle width is 12, radius is then 6

  // Draw the old needle position in the screen background colour so
  // it gets erased on the TFT when the sprite is drawn
  img.fillTriangle(lx1,ly1,lx3,ly3,lx4,ly4,BACKGROUND);
  img.fillTriangle(lx2,ly2,lx3,ly3,lx4,ly4,BACKGROUND);

  // Set text coordinate datum to middle centre
  img.setTextDatum(MC_DATUM);
  img.setTextColor(TFT_WHITE);
  
  img.drawString("N",50,50-42,4);
  img.drawString("E",50+36,50,4); 
  img.drawString("S",50,50+42,4);
  img.drawString("W",50-48,50,4);

  img.drawCircle(50,50,30,TFT_DARKGREY);

  getCoord(50, 50, &lx1, &ly1, NEEDLE_L, angle);
  getCoord(50, 50, &lx2, &ly2, NEEDLE_L, angle+180);
  getCoord(50, 50, &lx3, &ly3, NEEDLE_W, angle+90);
  getCoord(50, 50, &lx4, &ly4, NEEDLE_W, angle-90);
  
  img.fillTriangle(lx1,ly1,lx3,ly3,lx4,ly4,TFT_RED);
  img.fillTriangle(lx2,ly2,lx3,ly3,lx4,ly4,TFT_LIGHTGREY);

  img.fillCircle(50,50,4,TFT_DARKGREY);
  img.fillCircle(50,50,2,TFT_LIGHTGREY);
  
  img.pushSprite(x-50, y-50, TFT_TRANSPARENT);

  // Delete sprite to free up the RAM
  img.deleteSprite();
  TPRINT
}

// -------------------------------------------------------------------------
// Main loop
// -------------------------------------------------------------------------
void loopCompass(int heading) {


  if (angle < heading) {
    drawCompass(64,64,angle); // Draw centre of compass at 64,64
    angle+=3; //Increment angle for testing
    }
  //if (angle > 359) angle = 0; // Limit angle to 360
  delay(WAIT); //don't spin too fast!
}