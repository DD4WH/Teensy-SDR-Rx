// code for the TFT display

#include "analyze_fft256iq.h"
#include <Metro.h>
#include "display.h"
#include <Audio.h>
#include <Adafruit_GFX.h>        // LCD Core graphics library
//#include <Adafruit_QDTech.h>     // 1.8" TFT Module using Samsung S6D02A1 chip
#include <Adafruit_ST7735.h> // Hardware-specific library
//extern Adafruit_QDTech tft;
extern Adafruit_ST7735 tft;
#define pos 50 // position of spectrum display, has to be < 124
#define pos_version 119 // position of version number printing
#define pos_x_tunestep 100
#define pos_y_tunestep 119 // = pos_y_menu 91
#define pos_x_frequency 21 //100
#define pos_y_frequency 61  //119
#define pos_x_smeter 5
#define pos_y_smeter 94
#define s_w 10
#define notchpos 2
#define notchL 15
#define notchColour YELLOW
//#define pos_centre_f 58 // Rx below IF, IF = 5515
int pos_centre_f = 98; // Rx above IF, IF = 5515 
#define spectrum_span 44.118
#define font_width 12
uint8_t sch = 0;
bool freq_flag = 0;

extern AudioAnalyzeFFT256IQ  myFFT;

float uv, uvold, dbuv, dbm, s;// microvolts, db-microvolts, s-units, volts, dbm, sold = old S-meter value, deltas = abs difference between sold and s
float dbmold = 0;
char string[80];  
int le, bwhelp, left, lsb; 
int pixelold[160] = {0};
int pixelnew[160];
int oldnotchF = 10000;
uint8_t digits_old [] = {9,9,9,9,9,9,9,9,9,9};

Metro Smetertimer = Metro(50);
  
//extern AudioAnalyzeFFT256  myFFT;      // FFT for Spectrum Display

void init_display(void) {
    tft.initR(INITR_BLACKTAB);
}

void setup_display(void) {
  
  // initialize the LCD display
//  tft.init();
//  tft.initR(INITR_BLACKTAB);   // initialize a S6D02A1S chip, black tab
  tft.setRotation(1);
  tft.fillScreen(BLACK); //BLACK);
  //tft.fillRect(0, pos, 160, 128-pos, BLACK); // erase old string
  tft.setCursor(0, 119);
  tft.setTextColor(WHITE);
  tft.setTextWrap(true);
//  tft.print("DD4WH SDR 5.00");
  
  // Show mid screen tune position
  tft.drawFastVLine(pos_centre_f, 0,pos+1, RED); //WHITE);
 // tft.drawFastHLine(0, pos,79, YELLOW);// WHITE);
  //tft.drawFastHLine(81, pos,79, YELLOW);

// draw S-Meter layout
  tft.drawFastHLine (pos_x_smeter, pos_y_smeter-1, 9*s_w, WHITE);
  tft.drawFastHLine (pos_x_smeter, pos_y_smeter+3, 9*s_w, WHITE);
  tft.fillRect(pos_x_smeter, pos_y_smeter-3, 2, 2, WHITE);
  tft.fillRect(pos_x_smeter+8*s_w, pos_y_smeter-3, 2, 2, WHITE);
  tft.fillRect(pos_x_smeter+2*s_w, pos_y_smeter-3, 2, 2, WHITE);
  tft.fillRect(pos_x_smeter+4*s_w, pos_y_smeter-3, 2, 2, WHITE);
  tft.fillRect(pos_x_smeter+6*s_w, pos_y_smeter-3, 2, 2, WHITE);
  tft.fillRect(pos_x_smeter+7*s_w, pos_y_smeter-4, 2, 3, WHITE);
  tft.fillRect(pos_x_smeter+3*s_w, pos_y_smeter-4, 2, 3, WHITE);
  tft.fillRect(pos_x_smeter+5*s_w, pos_y_smeter-4, 2, 3, WHITE);
  tft.fillRect(pos_x_smeter+s_w, pos_y_smeter-4, 2, 3, WHITE);
  tft.fillRect(pos_x_smeter+9*s_w, pos_y_smeter-4, 2, 3, WHITE);
  tft.drawFastHLine (pos_x_smeter+9*s_w, pos_y_smeter-1, 3*s_w*2+2, GREEN);
  tft.drawFastHLine (pos_x_smeter+9*s_w, pos_y_smeter+3, 3*s_w*2+2, GREEN);
  tft.fillRect(pos_x_smeter+11*s_w, pos_y_smeter-4, 2, 3, GREEN);
  tft.fillRect(pos_x_smeter+13*s_w, pos_y_smeter-4, 2, 3, GREEN);
  tft.fillRect(pos_x_smeter+15*s_w, pos_y_smeter-4, 2, 3, GREEN);
  tft.drawFastVLine (pos_x_smeter-1, pos_y_smeter-1, 5, WHITE); 
  tft.drawFastVLine (pos_x_smeter+15*s_w+2, pos_y_smeter-1, 5, GREEN);

  tft.setCursor(pos_x_smeter - 4, pos_y_smeter - 13);
  tft.setTextColor(WHITE);
  tft.setTextWrap(true);
  tft.print("S 1");
  tft.setCursor(pos_x_smeter + 28, pos_y_smeter - 13);
  tft.print("3");
  tft.setCursor(pos_x_smeter + 48, pos_y_smeter - 13);
  tft.print("5");
  tft.setCursor(pos_x_smeter + 68, pos_y_smeter - 13);
  tft.print("7");
  tft.setCursor(pos_x_smeter + 88, pos_y_smeter - 13);
  tft.print("9");
  tft.setCursor(pos_x_smeter + 120, pos_y_smeter - 13);
  tft.print("+20dB");

} // end void setupdisplay

// draw the spectrum display
// this version draws 1/10 of the spectrum per call but we run it 10x the speed
// this allows other stuff to run without blocking for so long

void show_spectrum(float line_gain, float LPFcoeff) {
      static int startx=0, endx;
      endx=startx+16;
      int scale=3;
      float avg = 0.0;
      // Draw spectrum display
      for (int16_t x=startx; x < endx; x+=1) {

                if ((x > 1) && (x < 159)) 
                // moving window - weighted average of 5 points of the spectrum to smooth spectrum in the frequency domain
                // weights:  x: 50% , x-1/x+1: 36%, x+2/x-2: 14% 
                    avg = myFFT.output[(x)*16/10]*0.5 + myFFT.output[(x-1)*16/10]*0.18 + myFFT.output[(x-2)*16/10]*0.07 + myFFT.output[(x+1)*16/10]*0.18 + myFFT.output[(x+2)*16/10]*0.07;
                    else 
                    avg =  myFFT.output[(x)*16/10];
//                pixelnew[x] = LPFcoeff * 2 * sqrt (abs(myFFT.output[(x)*16/10])*scale) + (1 - LPFcoeff) * pixelold[x];
                // low pass filtering of the spectrum pixels to smooth/slow down spectrum in the time domain
                // experimental LPF for spectrum:  ynew = LPCoeff * x + (1-LPCoeff) * yprevious; here: A = 0.3 to 0.5 seems to be a good idea
                pixelnew[x] = LPFcoeff * 2 * sqrt (abs(avg)*scale) + (1 - LPFcoeff) * pixelold[x];
                
/*                for (int16_t i=0; x < 5; x+=1) {
                         
                pixelnew[x+i] = 2 * sqrt (abs(myFFT.output[(x+i)*16/10])*scale);
                if (pixelnew[x+i] > pos-1) pixelnew[x+i] = pos-1;
                }
  */              
//                int bar=2 * sqrt (abs(myFFT.output[x*16/10])*scale);
//                if (bar >pos-1) bar=pos-1;
                  
                              if(x != pos_centre_f) {
// common way: draw bars
//                                    tft.drawFastVLine(x, pos-1-bar,bar, BLUE); // GREEN);
//                                    tft.drawFastVLine(x, 0, pos-1-bar, WHITE); //BLACK);
// alternate way: draw pixels
// only plot pixel, if at a new position
                                      if (pixelold[x] != pixelnew[x]) { 
                                            tft.drawPixel(x, pos-1-pixelold[x], BLACK); // delete old pixel
                                            tft.drawPixel(x, pos-1-pixelnew[x], WHITE); // write new pixel
                                            pixelold[x] = pixelnew[x]; }
/*                                      if (pixelnew[x] > 5 * (pixelnew[x+1] + pixelnew[x-1])) {
                                               tft.drawFastVLine(x, pos-1-pixelnew[x], pixelnew[x], BLUE);
                                               tft.drawFastVLine(x, 0, pos-1- pixelnew[x], BLACK);
                                    }
  */                              }
      }


 
     // Calculate S units. 50uV = S9
    //if (0) {
     if (Smetertimer.check()==1) {
      uv = myFFT.output[159]+myFFT.output[160]+myFFT.output[161]+myFFT.output[162]+myFFT.output[158]+myFFT.output[157]+myFFT.output[156];   // measure signal strength of carrier of AM Transmission at exact frequency
      // low pass filtering for Smeter values 
      uv = 0.1 * uv + 0.9 * uvold;
      
      if (uv == 0) dbm = -130;
      else {
      dbm = 20*log10(uv)-83.5-25.7-1.5*line_gain;} //dbm standardized to 15.26Hz Receiver Bandwidth
      
      // constant 83.5dB determined empirically by measuring a carrier with a Perseus SDR
      // and comparing the output of the Teensy FFT
      // 25.7dB is INA163 gain in frontend 
      //dbm measurement on the Perseus standardized to RBW of 15.26Hz 
     // float vol = analogRead(15);
     // vol = vol / 1023.0;
    // now calculate S-value from dbm
      s = 9.0 + ((dbm + 73.0) / 6.0);
      if (s <0.0) s=0.0;
      if ( s > 9.0)
      {
        dbuv = dbm + 73.0;
        s = 9.0;
      }
      else dbuv = 0.0;
     
      // Print S units
      //s=roundf(s);
/*      tft.fillRect(0,105, 50, 7,ST7735_BLACK);
      tft.fillRect(0,105, 160, 7,ST7735_BLACK);
      tft.setCursor(100,105);
   //   sprintf(string,"%04.0f FFT",uv);
   //     sprintf(string,"%04.0f dbm",dbm);
        sprintf(string,"%02.0f",s);
      tft.print(string);
      tft.setCursor(0,105);
   //   sprintf(string,"%04.0f FFT",uv);
        sprintf(string,"%04.0f dbm",dbm);
      tft.print(string);
*/   
      tft.drawFastHLine(pos_x_smeter, pos_y_smeter, s*s_w+1, BLUE);
      tft.drawFastHLine(pos_x_smeter+s*s_w+1, pos_y_smeter, (9*s_w+1)-s*s_w+1, BLACK);

      tft.drawFastHLine(pos_x_smeter, pos_y_smeter+1, s*s_w+1, WHITE);
      tft.drawFastHLine(pos_x_smeter+s*s_w+1, pos_y_smeter+1, (9*s_w+1)-s*s_w+1, BLACK);
      tft.drawFastHLine(pos_x_smeter, pos_y_smeter+2, s*s_w+1, BLUE);
      tft.drawFastHLine(pos_x_smeter+s*s_w+1, pos_y_smeter+2, (9*s_w+1)-s*s_w+1, BLACK);

   //   tft.drawFastHLine(pos_x_smeter, pos_y_smeter+3, s*s_w+1, BLUE);
   //   tft.drawFastHLine(pos_x_smeter+s*s_w+1, pos_y_smeter+3, (9*s_w+1)-s*s_w+1, BLACK);

      if(dbuv>30) dbuv=30;
      tft.drawFastHLine(pos_x_smeter+9*s_w+1, pos_y_smeter, (dbuv/5)*s_w+1, RED);
      tft.drawFastHLine(pos_x_smeter+9*s_w+(dbuv/5)*s_w+1, pos_y_smeter, (6*s_w+1)-(dbuv/5)*s_w, BLACK);
      tft.drawFastHLine(pos_x_smeter+9*s_w+1, pos_y_smeter+1, (dbuv/5)*s_w+1, RED);
      tft.drawFastHLine(pos_x_smeter+9*s_w+(dbuv/5)*s_w+1, pos_y_smeter+1, (6*s_w+1)-(dbuv/5)*s_w, BLACK);
      tft.drawFastHLine(pos_x_smeter+9*s_w+1, pos_y_smeter+2, (dbuv/5)*s_w+1, RED);
      tft.drawFastHLine(pos_x_smeter+9*s_w+(dbuv/5)*s_w+1, pos_y_smeter+2, (6*s_w+1)-(dbuv/5)*s_w, BLACK);

   //   tft.drawFastHLine(pos_x_smeter+9*s_w+1, pos_y_smeter+3, (dbuv/5)*s_w+1, RED);
   //   tft.drawFastHLine(pos_x_smeter+9*s_w+(dbuv/5)*s_w+1, pos_y_smeter+3, (6*s_w+1)-(dbuv/5)*s_w, BLACK);

               
//      tft.fillRect(0, 105, 70, 7,ST7735_BLACK);
//      tft.setCursor(0, 105);
//      if (dbuv == 0) sprintf(string,"S:%1.0f",s);
//      else {
//        sprintf(string,"S:9+%02.0f dB",dbuv);
        
//        }
//      tft.print(string);
//      tft.fillRect(100,105, 50, 7,ST7735_BLACK);
//      tft.setCursor(100,105);
//      sprintf(string,"%04.0f dBm",dbm);
//      tft.print(string);
      uvold = uv;
      } // end if (Smeter Timer)   
  
  startx+=16;
  if(startx >=160) startx=0;


//digitalWrite(DEBUG_PIN,0); // 
}

/* old draw routine
// draw the spectrum display

void show_spectrum(void) {

  int scale=1;
  for (int16_t x=0; x < 160; x+=2) 
  {
    int bar=abs(myFFT.output[x*8/10])/scale;
    if (bar >60) bar=60;
    if(x!=80)
    {
       tft.drawFastVLine(x, 60-bar,bar, GREEN);
       tft.drawFastVLine(x, 0, 60-bar, BLACK);    
    }
  }
}
*/

void show_waterfall(void) {
  // experimental waterfall display for CW -
  // this should probably be on a faster timer since it needs to run as fast as possible to catch CW edges
  //  FFT bins are 22khz/128=171hz wide 
  // cw peak should be around 11.6khz - 
  static uint16_t waterfall[80];  // array for simple waterfall display
  static uint8_t w_index=0,w_avg;
  waterfall[w_index]=0;
  for (uint8_t y=66;y<67;++y)  // sum of bin powers near cursor - usb only
      waterfall[w_index]+=(uint8_t)(abs(myFFT.output[y])); // store bin power readings in circular buffer
  waterfall[w_index]|= (waterfall[w_index]<<5 |waterfall[w_index]<<11); // make it colorful
  int8_t p=w_index;
  for (uint8_t x=158;x>0;x-=2) {
    tft.fillRect(x,65,2,4,waterfall[p]);
    if (--p<0 ) p=79;
  }
  if (++w_index >=80) w_index=0; 
}



void show_bandwidth (int M, long int FU, long int FL) {

   tft.drawFastHLine(0,pos+1,160, BLACK); // erase old indicator
   tft.drawFastHLine(0,pos+2,160, BLACK); // erase old indicator 
   tft.drawFastHLine(0,pos+3,160, BLACK); // erase old indicator
   tft.drawFastHLine(0,pos,160, BLACK); // erase old indicator

      bwhelp = FU /100;
      int leU = bwhelp*16/spectrum_span;
      bwhelp = FL /100;
      int leL = bwhelp*16/spectrum_span;
      float kHz = (FU + FL) / 1000.0;
      
  switch (M) {
  case 0: //AM
      tft.fillRect(4, pos_y_frequency-3, 32, 8, BLACK); // erase old string
      tft.setTextColor(GREEN);
      tft.setCursor(4, pos_y_frequency-3);
      tft.print("AM"); 
      break;
  case 3: //DSB
      tft.fillRect(4, pos_y_frequency-3, 32, 8, BLACK); // erase old string
      tft.setTextColor(GREEN);
      tft.setCursor(4, pos_y_frequency-3);
      tft.print("DSB"); 
      break;
  case 4: //StereoAM
      tft.fillRect(4, pos_y_frequency-3, 32, 8, BLACK); // erase old string
      tft.setTextColor(GREEN);
      tft.setCursor(4, pos_y_frequency-3);
      tft.print("SteAM"); 
      break;
  
  case 2: //LSB
      tft.fillRect(4, pos_y_frequency-3, 32, 8, BLACK); // erase old string
      tft.setTextColor(GREEN);
      tft.setCursor(4, pos_y_frequency-3);
      tft.print("LSB"); 
      break;
  case 1:  //USB
      tft.fillRect(4, pos_y_frequency-3, 32, 8, BLACK); // erase old string
      tft.setTextColor(GREEN);
      tft.setCursor(4, pos_y_frequency-3);
      tft.print("USB"); 
      break;
} // end switch
      //print bandwidth !
        tft.fillRect(4, pos_y_frequency+7, 32, 8, BLACK); // erase old string
        tft.setCursor(4, pos_y_frequency+7);
        sprintf(string,"%02.1fk",kHz);
        tft.print(string);
        tft.setTextColor(WHITE); // set text color to white for other print routines not to get confused ;-)
      // draw upper sideband indicator
      tft.drawFastHLine(pos_centre_f, pos+1, leU, RED);
      tft.drawFastHLine(pos_centre_f, pos+2, leU, RED);
      tft.drawFastHLine(pos_centre_f, pos+3, leU, RED);
      tft.drawFastHLine(pos_centre_f, pos, leU, RED);
      // draw lower sideband indicator   
      left = pos_centre_f - leL; 
      tft.drawFastHLine(left+1, pos+1, leL, RED);
      tft.drawFastHLine(left+1, pos+2, leL, RED);
      tft.drawFastHLine(left+1,pos+3, leL, RED);
      tft.drawFastHLine(left+1,pos, leL, RED);

  tft.fillRect(pos_centre_f  + 160/spectrum_span * 5, pos, 2, 3, YELLOW); // erase old string
  tft.fillRect(pos_centre_f  + 160/spectrum_span * 10, pos, 2, 4, YELLOW); // erase old string
  tft.fillRect(pos_centre_f  - 160/spectrum_span * 5, pos, 2, 3, YELLOW); // erase old string
  tft.fillRect(pos_centre_f - 160/spectrum_span * 10, pos, 2, 4, YELLOW); // erase old string
  tft.fillRect(pos_centre_f  - 160/spectrum_span * 15, pos, 2, 3, YELLOW); // erase old string
  tft.fillRect(pos_centre_f  + 160/spectrum_span * 15, pos, 2, 3, YELLOW); // erase old string
  tft.fillRect(pos_centre_f  - 160/spectrum_span * 20, pos, 2, 4, YELLOW); // erase old string
  tft.fillRect(pos_centre_f  - 160/spectrum_span * 25, pos, 2, 3, YELLOW); // erase old string
  tft.fillRect(pos_centre_f  - 160/spectrum_span * 30, pos, 2, 4, YELLOW); // erase old string
}  




void show_tunestep(String S) {  // show band
  char string[80]; 
  tft.fillRect(pos_x_tunestep, pos_y_tunestep, 60, 8, BLACK); // erase old string
  tft.setTextColor(WHITE);
  tft.setCursor(pos_x_tunestep, pos_y_tunestep);
  tft.print(S);
}

// show radio mode
void show_radiomode(String mode) { 
//  tft.fillRect(125, 85, 30, 7, BLACK); // erase old string
//  tft.setTextColor(WHITE);
//  tft.setCursor(125, 85);
//  tft.print(mode);
}  

void show_band(String bandname) {  // show band
//  tft.fillRect(100, 85, 19, 7, BLACK); // erase old string
//  tft.setTextColor(WHITE);
//  tft.setCursor(100, 85);
//  tft.print(bandname);
}

void show_notch(int notchF, int MODE) {
  // pos_centre_f is the x position of the Rx centre
  // pos is the y position of the spectrum display 
  // notch display should be at x = pos_centre_f +- notch frequency and y = 20 
  //  LSB: 
  pos_centre_f+=1; // = pos_centre_f + 1;
          // delete old indicator
          tft.drawFastVLine(pos_centre_f + 1 + 160/spectrum_span * oldnotchF / 1000, notchpos, notchL, BLACK);
          tft.drawFastVLine(pos_centre_f + 160/spectrum_span * oldnotchF / 1000, notchpos, notchL, BLACK);
          tft.drawFastVLine(pos_centre_f -1 + 160/spectrum_span * oldnotchF / 1000, notchpos, notchL, BLACK);
          tft.drawFastHLine(pos_centre_f -4 + 160/spectrum_span * oldnotchF / 1000, notchpos+notchL, 9, BLACK);
          tft.drawFastHLine(pos_centre_f -3 + 160/spectrum_span * oldnotchF / 1000, notchpos+notchL + 1, 7, BLACK);
          tft.drawFastHLine(pos_centre_f -2 + 160/spectrum_span * oldnotchF / 1000, notchpos+notchL + 2, 5, BLACK);
          tft.drawFastHLine(pos_centre_f -1 + 160/spectrum_span * oldnotchF / 1000, notchpos+notchL + 3, 3, BLACK);
          tft.drawFastVLine(pos_centre_f + 160/spectrum_span * oldnotchF / 1000, notchpos+notchL + 4, 2, BLACK);

          tft.drawFastVLine(pos_centre_f +1 - 160/spectrum_span * oldnotchF / 1000, notchpos, notchL, BLACK);
          tft.drawFastVLine(pos_centre_f - 160/spectrum_span * oldnotchF / 1000, notchpos, notchL, BLACK);
          tft.drawFastVLine(pos_centre_f -1 - 160/spectrum_span * oldnotchF / 1000, notchpos, notchL, BLACK);
          tft.drawFastHLine(pos_centre_f -4 - 160/spectrum_span * oldnotchF / 1000, notchpos+notchL, 9, BLACK);
          tft.drawFastHLine(pos_centre_f -3 - 160/spectrum_span * oldnotchF / 1000, notchpos+notchL + 1, 7, BLACK);
          tft.drawFastHLine(pos_centre_f -2 - 160/spectrum_span * oldnotchF / 1000, notchpos+notchL + 2, 5, BLACK);
          tft.drawFastHLine(pos_centre_f -1 - 160/spectrum_span * oldnotchF / 1000, notchpos+notchL + 3, 3, BLACK);
          tft.drawFastVLine(pos_centre_f - 160/spectrum_span * oldnotchF / 1000, notchpos+notchL + 4, 2, BLACK);
          // Show mid screen tune position
          tft.drawFastVLine(pos_centre_f - 1, 0,pos+1, RED); //WHITE);

      if (notchF >= 400 || notchF <= -400) {
          // draw new indicator according to mode
      switch (MODE)  {
          case 2: //modeLSB:
          tft.drawFastVLine(pos_centre_f + 1 - 160/spectrum_span * notchF / -1000, notchpos, notchL, notchColour);
          tft.drawFastVLine(pos_centre_f - 160/spectrum_span * notchF / -1000, notchpos, notchL, notchColour);
          tft.drawFastVLine(pos_centre_f -1 - 160/spectrum_span * notchF / -1000, notchpos, notchL, notchColour);
          tft.drawFastHLine(pos_centre_f -4 - 160/spectrum_span * notchF / -1000, notchpos+notchL, 9, notchColour);
          tft.drawFastHLine(pos_centre_f -3 - 160/spectrum_span * notchF / -1000, notchpos+notchL + 1, 7, notchColour);
          tft.drawFastHLine(pos_centre_f -2 - 160/spectrum_span * notchF / -1000, notchpos+notchL + 2, 5, notchColour);
          tft.drawFastHLine(pos_centre_f -1 - 160/spectrum_span * notchF / -1000, notchpos+notchL + 3, 3, notchColour);
          tft.drawFastVLine(pos_centre_f - 160/spectrum_span * notchF / -1000, notchpos+notchL + 4, 2, notchColour);
          break;
          case 1: //modeUSB:
          tft.drawFastVLine(pos_centre_f +1 + 160/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
          tft.drawFastVLine(pos_centre_f + 160/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
          tft.drawFastVLine(pos_centre_f -1 + 160/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
          tft.drawFastHLine(pos_centre_f -4 + 160/spectrum_span * notchF / 1000, notchpos+notchL, 9, notchColour);
          tft.drawFastHLine(pos_centre_f -3 + 160/spectrum_span * notchF / 1000, notchpos+notchL + 1, 7, notchColour);
          tft.drawFastHLine(pos_centre_f -2 + 160/spectrum_span * notchF / 1000, notchpos+notchL + 2, 5, notchColour);
          tft.drawFastHLine(pos_centre_f -1 + 160/spectrum_span * notchF / 1000, notchpos+notchL + 3, 3, notchColour);
          tft.drawFastVLine(pos_centre_f + 160/spectrum_span * notchF / 1000, notchpos+notchL + 4, 2, notchColour);
          break;
          case 0: // modeAM:
          tft.drawFastVLine(pos_centre_f + 1 + 160/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
          tft.drawFastVLine(pos_centre_f + 160/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
          tft.drawFastVLine(pos_centre_f - 1 + 160/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
          tft.drawFastHLine(pos_centre_f -4 + 160/spectrum_span * notchF / 1000, notchpos+notchL, 9, notchColour);
          tft.drawFastHLine(pos_centre_f -3 + 160/spectrum_span * notchF / 1000, notchpos+notchL + 1, 7, notchColour);
          tft.drawFastHLine(pos_centre_f -2 + 160/spectrum_span * notchF / 1000, notchpos+notchL + 2, 5, notchColour);
          tft.drawFastHLine(pos_centre_f -1 + 160/spectrum_span * notchF / 1000, notchpos+notchL + 3, 3, notchColour);
          tft.drawFastVLine(pos_centre_f + 160/spectrum_span * notchF / 1000, notchpos+notchL + 4, 2, notchColour);

          tft.drawFastVLine(pos_centre_f + 1 - 160/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
          tft.drawFastVLine(pos_centre_f - 160/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
          tft.drawFastVLine(pos_centre_f - 1 - 160/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
          tft.drawFastHLine(pos_centre_f -4 - 160/spectrum_span * notchF / 1000, notchpos+notchL, 9, notchColour);
          tft.drawFastHLine(pos_centre_f -3 - 160/spectrum_span * notchF / 1000, notchpos+notchL + 1, 7, notchColour);
          tft.drawFastHLine(pos_centre_f -2 - 160/spectrum_span * notchF / 1000, notchpos+notchL + 2, 5, notchColour);
          tft.drawFastHLine(pos_centre_f -1 - 160/spectrum_span * notchF / 1000, notchpos+notchL + 3, 3, notchColour);
          tft.drawFastVLine(pos_centre_f - 160/spectrum_span * notchF / 1000, notchpos+notchL + 4, 2, notchColour);
          break;
          case 3: //modeDSB:
          case 4: //modeStereoAM:
          if (notchF <=-400) {
          tft.drawFastVLine(pos_centre_f + 1 - 160/spectrum_span * notchF / -1000, notchpos, notchL, notchColour);
          tft.drawFastVLine(pos_centre_f - 160/spectrum_span * notchF / -1000, notchpos, notchL, notchColour);
          tft.drawFastVLine(pos_centre_f - 1 - 160/spectrum_span * notchF / -1000, notchpos, notchL, notchColour);
          tft.drawFastHLine(pos_centre_f -4 - 160/spectrum_span * notchF / -1000, notchpos+notchL, 9, notchColour);
          tft.drawFastHLine(pos_centre_f -3 - 160/spectrum_span * notchF / -1000, notchpos+notchL + 1, 7, notchColour);
          tft.drawFastHLine(pos_centre_f -2 - 160/spectrum_span * notchF / -1000, notchpos+notchL + 2, 5, notchColour);
          tft.drawFastHLine(pos_centre_f -1 - 160/spectrum_span * notchF / -1000, notchpos+notchL + 3, 3, notchColour);
          tft.drawFastVLine(pos_centre_f - 160/spectrum_span * notchF / -1000, notchpos+notchL + 4, 2, notchColour);
          }
          if (notchF >=400) {
          tft.drawFastVLine(pos_centre_f + 1 + 160/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
          tft.drawFastVLine(pos_centre_f + 160/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
          tft.drawFastVLine(pos_centre_f - 1 + 160/spectrum_span * notchF / 1000, notchpos, notchL, notchColour);
          tft.drawFastHLine(pos_centre_f -4 + 160/spectrum_span * notchF / 1000, notchpos+notchL, 9, notchColour);
          tft.drawFastHLine(pos_centre_f -3 + 160/spectrum_span * notchF / 1000, notchpos+notchL + 1, 7, notchColour);
          tft.drawFastHLine(pos_centre_f -2 + 160/spectrum_span * notchF / 1000, notchpos+notchL + 2, 5, notchColour);
          tft.drawFastHLine(pos_centre_f -1 + 160/spectrum_span * notchF / 1000, notchpos+notchL + 3, 3, notchColour);
          tft.drawFastVLine(pos_centre_f + 160/spectrum_span * notchF / 1000, notchpos+notchL + 4, 2, notchColour);
          }
          break;
      }
      }
      oldnotchF = notchF;
      pos_centre_f-=1; // = pos_centre_f - 1;
  } // end void show_notch


int ExtractDigit(long int n, int k) {
        switch (k) {
              case 0: return n%10;
              case 1: return n/10%10;
              case 2: return n/100%10;
              case 3: return n/1000%10;
              case 4: return n/10000%10;
              case 5: return n/100000%10;
              case 6: return n/1000000%10;
              case 7: return n/10000000%10;
              case 8: return n/100000000%10;
        }
}



// show frequency
void show_frequency(long int freq) { 
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    uint8_t zaehler;
    uint8_t digits[10];
    zaehler = 8;

          while (zaehler--) {
              digits[zaehler] = ExtractDigit (freq, zaehler);
//              Serial.print(digits[zaehler]);
//              Serial.print(".");
// 7: 10Mhz, 6: 1Mhz, 5: 100khz, 4: 10khz, 3: 1khz, 2: 100Hz, 1: 10Hz, 0: 1Hz
        }
            //  Serial.print("xxxxxxxxxxxxx");

    zaehler = 8;
        while (zaehler--) { // counts from 7 to 0
              if (zaehler < 6) sch = 7; // (khz)
              if (zaehler < 3) sch = 14; // (Hz)
          if (digits[zaehler] != digits_old[zaehler] || !freq_flag) { // digit has changed (or frequency is displayed for the first time after power on)
              if (zaehler == 7) {
                     sch = 0;
                     tft.setCursor(pos_x_frequency + font_width * (8-zaehler) + sch,pos_y_frequency); // set print position
                     tft.fillRect(pos_x_frequency + font_width * (8-zaehler) + sch,pos_y_frequency, font_width,16,BLACK); // delete old digit
                     if (digits[7] != 0) tft.print(digits[zaehler]); // write new digit in white
              }
              if (zaehler == 6) {
                            sch = 0;
                            tft.setCursor(pos_x_frequency + font_width * (8-zaehler) + sch,pos_y_frequency); // set print position
                            tft.fillRect(pos_x_frequency + font_width * (8-zaehler) + sch,pos_y_frequency, font_width,16,BLACK); // delete old digit
                            if (digits[6]!=0 || digits[7] != 0) tft.print(digits[zaehler]); // write new digit in white
              }
               if (zaehler == 5) {
                            sch = 7;
                            tft.setCursor(pos_x_frequency + font_width * (8-zaehler) + sch,pos_y_frequency); // set print position
                            tft.fillRect(pos_x_frequency + font_width * (8-zaehler) + sch,pos_y_frequency, font_width,16,BLACK); // delete old digit
                            if (digits[5] != 0 || digits[6]!=0 || digits[7] != 0) tft.print(digits[zaehler]); // write new digit in white
              }
              
              if (zaehler < 5) { 
              // print the digit
              tft.setCursor(pos_x_frequency + font_width * (8-zaehler) + sch,pos_y_frequency); // set print position
              tft.fillRect(pos_x_frequency + font_width * (8-zaehler) + sch,pos_y_frequency, font_width,16,BLACK); // delete old digit
              tft.print(digits[zaehler]); // write new digit in white
              }
              digits_old[zaehler] = digits[zaehler]; 
        }
        }
  
      tft.setTextSize(1);
      if (digits[7] == 0 && digits[6] == 0)
              tft.fillRect(pos_x_frequency + font_width * 3,pos_y_frequency + 11, 3, 3, BLACK);
      else    tft.fillRect(pos_x_frequency + font_width * 3,pos_y_frequency + 11, 3, 3, YELLOW);
      tft.fillRect(pos_x_frequency + font_width * 7 -4, pos_y_frequency + 11, 3, 3, YELLOW);
      if (!freq_flag) {
            tft.setCursor(pos_x_frequency + font_width * 9 + 16,pos_y_frequency + 7); // set print position
            tft.setTextColor(GREEN);
            tft.print("Hz");
      }
      freq_flag = 1;
      tft.setTextColor(WHITE);

} // END VOID SHOW-FREQUENCY

    
