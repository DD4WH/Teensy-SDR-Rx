/* Teensy SDR
 * version 2016_02_22
 * on my new laptop
 * DD4WH mods included
 * 
 * Experiment: freq_conv einbauen --> ok!
 * AM switchoff (passthrough) in other modes --> ok! (sideband suppression superb)
 * now only 47% processor load in SSB modes !!!
 * 
 * ***************************************************
 * 
 * this script only works with modified mixer.h in Audio library
 * allow for negative gain !
 * 
 ***************************************************** 
 * processor load: 
 * SSB: 50% / 53 % (mem 12 / 19) 
 * AM: 63% / 66 % (mem 14 / 21)
 * after elimination of biquad 3 & 4
 * SSB: 45% / 47% (mem 14 / 19)
 * AM: 57% / 60% (mem 14 / 19)
 * 

 New features by DD4WH
 + spectrum display now with 44.1kHz bandwidth, uses complex FFT256IQ library by kpc
 + spectrum display with pixels instead of bars, LPF implemented for smooth appearance
 + added AM demodulation
 + added pseudo stereo AM 
 + added DSB demodulation
 + added passband tuning, meaning easy adjustment of different bandwidths for upper and lower sideband (in DSB and StereoAM mode)
 + added automatic snap mode, which automatically searches the carrier in DSB mode when in 100Hz tuning step Tuning mode --> uses three-point-quadratic interpolation (thanks vladn)
 + added quadrature oscillators in the I and Q paths in order to translate IQ to baseband for demodulation, so that all filters work in baseband now (thanks Clint KA7OEI)
 + Hilbert and FIR filter bandwidths 1.8kHz, 2.6kHz, 3.6kHz, 4.4kHz, 6.5kHz, 8kHz, 11kHz --> all combined with adjustable IIR
 + more IIR filter bandwidths 1.8kHz, 2.3kHz, 2.6kHz, 3.1kHz, 3.6kHz, 4.0kHz, 4.4kHz, 5.4kHz, 6.5kHz, 8kHz, 11kHz 
 + filter "switching" done automatically by smooth adjusting bandwidth with encoder (IIR and FIR/Hilberts automatically switched)
 + biquad 1: fixed bandwidths with postIIR filter (8th order IIR --> 4 cascaded biquad stages, inverted Chebychev)
 + (manually adjustable notch filter (8th order IIR biquad filter))
 + calibrated graphical S-meter implemented, S-value independent of analog gain ("RF gain") setting and digital volume control
 + tone control: bass & treble (postAudioprocessor) 
 + recorder included, records audio from the SDR radio to RAW-files on the SD-card - up to 255 files, ca. 8MB per minute, playback also possible 
 + tune step for LW & MW 9kHz, all other bands 5kHz
 + Real Time Clock integrated (backed up by battery)
 + date adjust integrated
 + frequency limits 12kHz - 36MHz implemented by using the new Si5351 library by Jason --> now SAQ on 17.1kHz is receivable ;-)
 + new 2015 version of Jason´s Si5351 library included: now tunes down to 12kHz, but below that Si5351 crashes, requires new boot
 + also new Si5351 library needed correction_factor for frequencies
 + Menu addition: adjust correction_constant for Si5351
 + Menu addition: adjust correction_factor for Si5351
 + Menu addition: Save settings to EEPROM
 + Menu addition: Load settings from EEPROM
 + bands array now also holds mode & USB bandwidth & LSB bandwidth for each band (and saved by EEPROM)
 + band is now global variable and is saved in EEPROM --> after load switches to last band saved
 + LPF automatic relay switching --> my hardware has 5 switchable elliptic Low Pass Filters: these are really needed for serious receive to supress 3rd, 5th, 7th and higher order harmonics
 + now five buttons (two additional ones)
 + added BAND down function, so buttons 1 and 2 now function as BAND DOWN and BAND UP
 + manual calibration of I + Q amplitude & phase --> phase adjustment mixes a little amount of I into Q or vice versa before passing the Hilbert Filters
 + button 5: Menu "RF gain" = manually adjust analog codec gain = "RF gain" in front of ADC (lineInLevel adjustable from 0-15 --> 0dB to 22.5dB in steps of 1.5dB)
 + Menu addition: choose among eleven different FFT Window Functions
 + added to EEPROM load and save: IQ amplitude & IQ phase calibrations, RF gain, mode, bandwidth USB, bandwidth LSB, FFT Window Function, LPF coeff for spectrum display
 + separate variables & voids: mode & audio bandwidth 
 + show voltage Vcc (Menu2 entry) and check every 5 secs and warn if < 11 volts
 + larger bandwiths for IIR filters in AM mode to allow for sideband-selected AM reception
 + Freq_conv.cpp mit receiveWritable
 + very narrow manually adjustable notch filter with graphical indicator
 + frequency display: update only those frequency digits that have changed during tuning
 + time display: update only digits that have changed

 Todo
 + sideband selected AM by shifting IF with an offset --> implemented in menu passband tuning
 + decimate and interpolate for biquad1 & biquad2 --> is it worth it? We already have reduced processor load to 47% . . .
 + LMS noise reduction, does it work even without decimation / interpolation ?
 + VFO A and B
 + switch between local time & UTC
 + Menu point: reset to factory settings
 + memories for stations with name & time read from txt.file on SD card


 * simple software define radio using the Softrock transceiver 
 * the Teensy audio shield is used to capture and generate 16 bit audio
 * audio processing is done by the Teensy 3.1
 * simple UI runs on a 160x128 color TFT display - AdaFruit or Banggood knockoff which has a different LCD controller
 * Copyright (C) 2014, 2015  Rich Heslip rheslip@hotmail.com
 * History:
 * 4/14 initial version by R Heslip VE3MKC
 * 6/14 Loftur E. Jónasson TF3LJ/VE2LJX - filter improvements, inclusion of Metro, software AGC module, optimized audio processing, UI changes
 * 1/15 RH - added encoder and SI5351 tuning library by Jason Milldrum <milldrum@gmail.com>
 *    - added HW AGC option which uses codec AGC module
 *    - added experimental waterfall display for CW
 * 3/15 RH - updated code to Teensyduino 1.21 and audio lib 1.02
 *    - added a lot of #defines to neaten up the code
 *    - added another summer at output - switches audio routing at runtime, provides a nice way to adjust I/Q balance and do AGC/ALC
 *    - added CW I/Q oscillators for CW transmit mode
 *    - added SSB and CW transmit
 *    - restructured the code to facilitate TX/RX switching
 * Todo:
 * clean up some more of the hard coded HW and UI stuff 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define ver "v 2016.02.22"

#include <Time.h>
#include <TimeLib.h>
#include <Metro.h>
#include <Audio.h>
#include <Wire.h>
#include <SD.h>
#include <Encoder.h>
#include <EEPROM.h>

#include <si5351.h>
#include <Bounce.h>
//#include <Bounce2.h>
#include <Adafruit_GFX.h>   // LCD Core graphics library
//#include <Adafruit_QDTech.h>// 1.8" TFT Module using Samsung S6D02A1 chip
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include "analyze_fft256iq.h" // new complex FFT library for full bandwidth spectrum from I & Q --> 44.1kHz
#include "AM_demod.h"
#include "freq_conv.h"
#include "filters.h"
#include "display.h"
#include "iir_18.h"
#include "iir_23.h"
#include "iir_26.h"
#include "iir_31.h"
#include "iir_36.h"
#include "iir_40.h"
#include "iir_44.h"
#include "iir_54.h"
#include "iir_65.h"
#include "iir_80.h"
#include "iir_110.h"


void sinewave(void);
extern void agc(void);      // RX agc function
extern void setup_display(void);
extern void init_display(void);
//extern void show_spectrum(void);  // spectrum display draw
extern void show_spectrum(float line_gain, float LPFcoeff);  // spectrum display draw
extern void show_waterfall(void); // waterfall display
//extern void show_bandwidth(int filterwidth);  // show filter bandwidth
extern void show_radiomode(String mode);  // show filter bandwidth
extern void show_band(String bandname);  // show band
extern void show_frequency(long int freq);  // show frequency
extern void show_tunestep (String S);
extern void show_bandwidth (int M, long int FU, long int FL);
extern void show_notch (int notchF, int MODE);

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

#define Schedule_length 3

//#define  DEBUG
//#define DEBUG_TIMING  // for timing execution - monitor HW pin

// SW configuration defines
// don't use more than one AGC!
//#define SW_AGC   // define for Loftur's SW AGC - this has to be tuned carefully for your particular implementation
// codec hardware AGC works but it looks at the entire input bandwidth 
// ie codec HW AGC works on the strongest signal, not necessarily what you are listening to
// it should work well for ALC (mic input) though
//#define HW_AGC // define for codec AGC 
// #define CW_WATERFALL // define for experimental CW waterfall - needs faster update rate
#define AUDIO_STATS    // shows audio library CPU utilization etc on serial console

// band selection stuff
struct band {
  long freq; // frequency in Hz
//  unsigned long freq; // frequency in Hz
  String name; // name of band
  int mode; // sideband & bandwidth
  int bandwidthU;
  int bandwidthL;
  int RFgain; 
};

// Estebans defs
//static int16_t IF_FREQ = 5515;     // IF Oscillator frequency
int16_t IF_FREQ = 5515;     // IF Oscillator frequency
float s_old = 0;
//static float32_t Osc_Q_buffer_i_f[AUDIO_BLOCK_SAMPLES];
//static float32_t Osc_I_buffer_i_f[AUDIO_BLOCK_SAMPLES];
float32_t Osc_Q_buffer_i_f[AUDIO_BLOCK_SAMPLES];
float32_t Osc_I_buffer_i_f[AUDIO_BLOCK_SAMPLES];
q15_t Osc_Q_buffer_i[AUDIO_BLOCK_SAMPLES];
q15_t Osc_I_buffer_i[AUDIO_BLOCK_SAMPLES];
//static bool flag = 0;
bool flag = 0;
bool freqconv_sw=1; // 1 = frequency conversion on; 0 = freqconv off
bool AM_pass = 1; // 1 = Am demodulation on; 0 = off
q15_t last_dc_level = 0;


//#define IF_FREQ 5515 //11029 //5515 // IF Oscillator frequency, already defined above!!
long calibration_factor = 10000000 ;// 10002285;
long calibration_constant = 3500;
unsigned long long hilfsf;
#define F_MAX 36000000 + IF_FREQ
#define F_MIN 12000 + IF_FREQ

#define BAND_LW  0   // these can be in any order but indexes need to be sequential for band switching code
#define BAND_MW   1
#define BAND_120M 2
#define BAND_90M  3
#define BAND_75M  4   // these can be in any order but indexes need to be sequential for band switching code
#define BAND_60M  5
#define BAND_49M  6
#define BAND_41M  7
#define BAND_31M  8
#define BAND_25M  9
#define BAND_22M  10
#define BAND_19M  11
#define BAND_16M  12
#define BAND_15M  13
#define BAND_13M  14
#define BAND_11M  15


#define FIRST_BAND BAND_LW
#define LAST_BAND  BAND_15M
#define NUM_BANDS  16

// radio operation mode defines used for filter selections etc
#define modeAM 0
#define modeUSB 1
#define modeLSB 2
#define modeDSB 3
#define modeStereoAM 4
#define modeSAM 5
#define firstmode modeAM
#define lastmode modeStereoAM
#define startmode modeAM

#define MAX_BANDWIDTH 11000

// f, band, mode, bandwidth, RFgain
struct band bands[NUM_BANDS] = {
  153000-IF_FREQ,"LW", modeAM, 3600,3600,0,
  531000-IF_FREQ,"MW",  modeAM, 3600,3600,0,
  2485000-IF_FREQ,"120M",  modeAM, 3600,3600,0,
  3500000-IF_FREQ,"90M",  modeLSB, 3600,3600,6,
  3905000-IF_FREQ,"75M",  modeAM, 3600,3600,4,
  4750000-IF_FREQ,"60M",  modeAM, 3600,3600,7,
  5910000-IF_FREQ,"49M",  modeAM, 3600,3600,0,
  7120000-IF_FREQ,"41M",  modeAM, 3600,3600,0,
  9420000-IF_FREQ,"31M",  modeAM, 3600,3600,0,
  11735000-IF_FREQ,"25M", modeAM, 3600,3600,0,
  13570000-IF_FREQ,"22M", modeAM, 3600,3600,0,
  15140000-IF_FREQ,"19M",  modeAM, 3600,3600,0,
  17480000-IF_FREQ,"16M", modeAM, 3600,3600,0,
  18900000-IF_FREQ,"15M", modeAM, 3600,3600,0,
  21450000-IF_FREQ,"13M", modeAM, 3600,3600,0,
  25670000-IF_FREQ,"11M", modeAM, 3600,3600,0
};
#define STARTUP_BAND BAND_LW    // 
int band = STARTUP_BAND;

//SPI connections for 1.8" display
const int8_t sclk   = 5;
const int8_t mosi   = 4;
const int8_t cs     = 2;
const int8_t dc     = 3;
const int8_t rst    = 1;

const int8_t MenuSW = 20;  // button 4 = menu switch: encoder tune or encoder BW adjust
const int8_t ModeSW = 26;    // button 3 = USB/LSB & bandwidth, ehemals pin 20
const int8_t BandDOWNSW = 21;    // button 1 = band selector down
const int8_t BandUPSW = 24; // button 2 = band selector up
const int8_t TuneSW = 8;    // Optical Encoder pushbutton
const int8_t Button5 = 25; // Menu2 switch

const int8_t Band1 = 29; // band selection pins for LPF relays, used with 2N7000: HIGH means LPF is activated
const int8_t Band2 = 30; // always use only one LPF with HIGH, all others have to be LOW
const int8_t Band3 = 31;
const int8_t Band4 = 32;
const int8_t Band5 = 33; // attention, special pin, only use as output pin!
const int8_t VoltCheck = 28; // DAC for voltage check

// definitions for cursor positions display setup

const int8_t pos_x_date = 100;
const int8_t pos_y_date = 119; //91;
const int8_t pos_x_time = 0;
const int8_t pos_y_time = 119;//91;
const int8_t pos_x_station = 0;
const int8_t pos_y_station = 0;
const int8_t pos_x_frequency  = 39; //100
const int8_t pos_y_frequency = 59;  //119
const int8_t pos_x_menu = 100;
const int8_t pos_y_menu = 119;

#define pos 50 // position of spectrum display, has to be < 124 --> also to be defined in display.cpp
 Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, mosi, sclk, rst); // soft SPI

#define  BACKLIGHT  0  // backlight control signal

// Optical Encoder connections
Encoder tune(16, 17);

// clock generator
Si5351 si5351;
#define MASTER_CLK_MULT  4  // QSD frontend requires 4x clock

// various timers
Metro five_sec=Metro(5000); // Set up a 5 second Metro
Metro one_sec = Metro (1000);
Metro ms_100 = Metro(100);  // Set up a 100ms Metro
Metro ms_50 = Metro(50);  // Set up a 50ms Metro for polling switches
Metro lcd_upd =Metro(4);  // Set up a Metro for spectrum display updates
Metro CW_sample =Metro(10);  // Set up a Metro for LCD updates
Metro MP3_Smetertimer = Metro(200);

#ifdef CW_WATERFALL
Metro waterfall_upd =Metro(25);  // Set up a Metro for waterfall updates
#endif

#define TUNE_STEP1   5000    // shortwave
#define TUNE_STEP2   1   // fine tuning
#define TUNE_STEP3   100    // 
#define first_tunehelp 1
#define last_tunehelp 3
int tunehelp = 1;
int tunestep = TUNE_STEP1;
String tune_text="Fast Tune";

// Menus !
#define TUNING 0
#define BWADJUST 1
#define NOTCH 2
#define RECORDING 3
#define CALIBRATIONCONSTANT 4
#define IQADJUST 5
#define IQPHASEADJUST 6
#define SAVETOEEPROM 7
#define LOADFROMEEPROM 8
#define TIMEADJUST 9
#define DATEADJUST 10
#define CALIBRATIONFACTOR 11
#define FFTWINDOW 12
#define LPFSPECTRUM 13

#define BW_step 200
#define first_menu TUNING
#define last_menu LPFSPECTRUM
#define start_menu TUNING
int Menu = TUNING;
String menu_text = "Tuning";

// Menu2 !
#define SNAP 1
#define PASSBAND 2
#define RFGAIN 3
#define BASS 4
#define TREBLE 5
//#define MP3 5
#define VOLTAGE 6
#define VERSION 7
//#define NOTCH 8
#define first_menu2 TUNING
#define last_menu2 VERSION //NOTCH
#define start_menu2 TUNING
int Menu2 = start_menu2;
float bass = 0.4;
float bass_help = bass * 100;
float treble = 0.0;
float treble_help = treble *100; 
int notchF = 200; // if 200, notch is off, if >= 400, notch is on
int notchQ = 10;
//bool notchswitch = 0; // 0 = notch filter off; 1 = notchfilter on
//bool notchside = 0; // 0 = LSB; 1 = USB
float LPFcoeff = 0.3; // used for low pass filtering the spectrum display
float LPFcoeffhelp = LPFcoeff * 100;
int passbandBW = 0;
int maximum = 0;
int maxbin = 0;
int Lbin = 0;
int Ubin = 0;
int links = 0;
int rechts = 0;
float delta = 0;
float binBW = AUDIO_SAMPLE_RATE_EXACT / 256.0;
float bin1 = 0;
float bin2 = 0;
float bin3 = 0;
int posbin = 159; // for Rx above IF
// int posbin = 95; // for Rx below IF



int helpmin; // definitions for time and date adjust - Menu
int helphour;
int helpday;
int helpmonth;
int helpyear;
int helpsec;
uint8_t hour10_old;
uint8_t hour1_old;
uint8_t minute10_old;
uint8_t minute1_old;
uint8_t second10_old;
uint8_t second1_old;
bool timeflag = 0;
float Q_gain=1000;
float I_gain=1000;
float I_help = 1.0;
float Q_help = 1.0;
float Q_in_I = 0.000;
float Q_in_I_help = 0;
float I_in_Q = 0.000;
float I_in_Q_help = 0;
float IQcounter = 0;
#define  INITIAL_VOLUME 0.8   // 0-1.0 output volume on startup
float vol = INITIAL_VOLUME;
//int line_in_gain = 0; // 0 --> 0 dB, 15 --> 22.3 dB of analog gain in front of the ADC
//int line_in_gain_help;
int Window_FFT = 0;
String FFT_STRING;
float Volt;
int RECORDFLAG = 0;

/* ############################################################
 *  definitions for MP3 Player & Recorder
 *  MP3-Player (Frank Bösing) did not work properly, memory issues ??
 *  it worked as standalone without SDR radio and with 1024FFT
 *  ###########################################################
 */

int audiomem = 0;

int Rec_Mode = 0; // Recorder: 0 = stopped, 1 = recording, 2 = playing
File frec; // audio is recorded to this file first
int playfile = 0;
//int recfile = -1; 

// buttons in RECORD Mode 
Bounce bouncer1 = Bounce(BandUPSW, 50); //play
Bounce bouncer3= Bounce(BandDOWNSW, 50); //record
Bounce bouncer2 = Bounce(ModeSW, 50);//stop & switch to radio
Bounce bouncer4 = Bounce(MenuSW, 50);//previous track
Bounce bouncer5 = Bounce(Button5, 50); //next track 

/* ############################################################
 *  end of definitions for MP3 Player
 *  
 *  ###########################################################
 */

// audio definitions 

// Create the Audio components.  These should be created in the
// order data flows, inputs/sources -> processing -> outputs
//		
AudioInputI2S       audioinput;           // Audio Shield: mic or line-in
AudioMixer4         audio_in_I;
AudioMixer4         audio_in_Q;

AudioEffectFreqConv   FreqConv;

AMDemod             AM;

// FIR filters
AudioFilterFIR      FIR_I;
AudioFilterFIR      FIR_Q;

// IIR filters (= biquads)
AudioFilterBiquad   biquad1;
AudioFilterBiquad   biquad2;
AudioFilterBiquad   biquad3;
AudioFilterBiquad   biquad4;

AudioMixer4         Summer;        // Summer (add inputs)
AudioAnalyzeFFT256IQ  myFFT;      // Spectrum Display complex FFT

/*
AudioSynthWaveform  cosIF;           // Local Oscillator
AudioSynthWaveform  sinIF;           // Local Oscillator

//AudioAnalyzePeak    MP3_peak;   // does not seem to work: bug internally in AudioAnalyzePeak ???
AudioEffectMultiply    multiply_I_cos;         // Mixer (multiply inputs)
AudioEffectMultiply    multiply_Q_sin;       // Mixer (multiply inputs)
AudioEffectMultiply    multiply_I_sin;         // Mixer (multiply inputs)
AudioEffectMultiply    multiply_Q_cos;         // Mixer (multiply inputs)
*/
AudioRecordQueue    recorder;       // to record audio from the radio to SD card
AudioMixer4         recmix;

//AudioMixer4            add_substract_I; // belongs to the complex NCO
//AudioMixer4            add_substract_Q; // belongs toe the complex NCO
AudioMixer4            USB;
AudioMixer4            LSB;

AudioMixer4         mixleft;
AudioMixer4         mixright;

AudioOutputI2S      audioOutput;   // Audio Shield: headphones & line-out

//AudioPlaySdWav           playWAV1;
//AudioPlaySdMp3           playMp31; 
//AudioPlaySdAac           playAac1;  
AudioPlaySdRaw           playSd;      // playback recorded files in RAW format

AudioControlSGTL5000 audioShield;  // Create an object to control the audio shield.

//---------------------------------------------------------------------------------------------------------
// Create Audio connections to build a software defined Radio Receiver
//

// audio input and IQ amplitude and phase correction
AudioConnection c5(audioinput, 0, audio_in_I, 0);
AudioConnection c6(audioinput, 1, audio_in_Q, 0);
AudioConnection c7(audio_in_I, 0, audio_in_Q, 1);
AudioConnection c8(audio_in_Q, 0, audio_in_I, 1); 

// complex FFT for spectrum display 
AudioConnection c23(audioinput, 0, myFFT, 0);
AudioConnection c24(audioinput, 1, myFFT, 1);

AudioConnection c3(audio_in_I, 0, FreqConv, 0);
AudioConnection c4(audio_in_Q, 0, FreqConv, 1);

/*
// Quadrature oscillator for frequency translation of the I and Q into baseband
AudioConnection c3(audio_in_I, 0, multiply_I_cos, 0);
AudioConnection c4(audio_in_Q, 0, multiply_Q_cos, 0);
AudioConnection c9(audio_in_I, 0, multiply_I_sin, 0);
AudioConnection c10(audio_in_Q, 0, multiply_Q_sin, 0);
AudioConnection c11(cosIF, 0, multiply_I_cos, 1);
AudioConnection c12(cosIF, 0, multiply_Q_cos, 1);
AudioConnection c13(sinIF, 0, multiply_Q_sin, 1);
AudioConnection c14(sinIF, 0, multiply_I_sin, 1);
AudioConnection c15(multiply_I_cos, 0, add_substract_I, 0);
AudioConnection c16(multiply_Q_sin, 0, add_substract_I, 1);
AudioConnection c17(multiply_I_sin, 0, add_substract_Q, 0);
AudioConnection c18(multiply_Q_cos, 0, add_substract_Q, 1);

// Main filters: either Hilberts with +45/-45 degrees or standard FIR filters
AudioConnection c19(add_substract_I, 0, FIR_I,0);
AudioConnection c20(add_substract_Q, 0, FIR_Q,0);
//AudioConnection c19(add_substract_Q, 0, FIR_I,0);
//AudioConnection c20(add_substract_I, 0, FIR_Q,0);
*/

AudioConnection c19(FreqConv, 0, FIR_I,0);
AudioConnection c20(FreqConv, 1, FIR_Q,0);

// Standard AM demodulator
AudioConnection c21(FIR_I, 0, AM, 0);
AudioConnection c22(FIR_Q, 0, AM, 1);
AudioConnection c25a(AM, 0, USB, 2);
AudioConnection c25b(AM, 0, LSB, 2);

// connections between I and Q signals and the AM/LSB/USB/DSB switches (mixers)
AudioConnection c26a(FIR_I ,0, USB, 0);
AudioConnection c26b(FIR_I ,0, LSB, 0);
AudioConnection c27a(FIR_Q ,0, USB, 1);
AudioConnection c27b(FIR_Q ,0, LSB, 1);

// IIR filters separately for left and right channel (to enable "stereo AM" = pseudostereo DSB reception)
AudioConnection c28a(USB, 0, biquad1, 0); // right channel
AudioConnection c29a(biquad1, 0, biquad3, 0);
AudioConnection c28b(LSB, 0, biquad2, 0); // left channel
AudioConnection c29b(biquad2, 0, biquad4, 0);

//mp3
//AudioConnection          patch1(playMp31,0,mixleft,0);
//AudioConnection          patch2(playMp31,1,mixright,0);
//aac
//AudioConnection          patch1a(playAac1, 0, mixleft, 1);
//AudioConnection          patch2a(playAac1, 1, mixright, 1);

// play RAW files from SD (in mono)
AudioConnection          patch3(playSd, 0, mixleft, 3);
AudioConnection          patch4(playSd, 0, mixright, 3);

// outputs from the IIR filters go to both mixers (left & right)
AudioConnection c30a(biquad3,0, mixright, 0);
AudioConnection c30b(biquad3,0, mixleft, 0);
AudioConnection c31a(biquad4,0, mixright, 1);
AudioConnection c31b(biquad4,0, mixleft, 1);

// mix left & right channel for recorder
AudioConnection c101 (mixleft, 0, recmix, 1);
AudioConnection c102a (mixright, 0, recmix, 0);
// connect recorder to recmixer
AudioConnection c102b (recmix, 0, recorder, 0);

// stereo output connected to mixright & mixleft
AudioConnection c32(mixright,0, audioOutput, 1);
AudioConnection c33(mixleft,0, audioOutput, 0);


//---------------------------------------------------------------------------------------------------------


void setup() 
{
  Serial.begin(9600); // debug console
#ifdef DEBUG
  while (!Serial) ; // wait for connection
  Serial.println("initializing");
#endif 

  setSyncProvider(getTeensy3Time);
  //setTime(19,55,0,3,5,2015); 
 
  pinMode(BACKLIGHT, INPUT_PULLUP); // yanks up display BackLight signal
  pinMode(ModeSW, INPUT_PULLUP);  // USB/LSB switch
  pinMode(BandUPSW, INPUT_PULLUP);  // 
  pinMode(BandDOWNSW, INPUT_PULLUP);  // 
  pinMode(TuneSW, INPUT_PULLUP);  // tuning rate = high
  pinMode(MenuSW, INPUT_PULLUP);  // 
  pinMode(Button5, INPUT_PULLUP);  // 

  pinMode(Band1, OUTPUT);  // LPF switches
  pinMode(Band2, OUTPUT);  // 
  pinMode(Band3, OUTPUT);  // 
  pinMode(Band4, OUTPUT);  // 
  pinMode(Band5, OUTPUT);  // 

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
    if (!digitalRead(BandDOWNSW)) {AudioMemory(21); audiomem = 21;} // this seems to be the figure that is the cause of the sometimes arbitrary loss of mirror rejection!? 32 does not work
      else {AudioMemory(90); audiomem = 100;}
  // SDR mode needs a max of 21, M4A playback needs a max of 11, recording needs > 70

  // Enable the audio shield and set the output volume.
  audioShield.enable();
  audioShield.volume(INITIAL_VOLUME/2);
  audioShield.audioPostProcessorEnable(); // enables the DAP chain of the codec post audio processing before the headphone out

#ifdef HW_AGC
  /* COMMENTS FROM Teensy Audio library:
    Valid values for dap_avc parameters
	maxGain; Maximum gain that can be applied
	0 - 0 dB
	1 - 6.0 dB
	2 - 12 dB
	lbiResponse; Integrator Response
	0 - 0 mS
	1 - 25 mS
	2 - 50 mS
	3 - 100 mS
	hardLimit
	0 - Hard limit disabled. AVC Compressor/Expander enabled.
	1 - Hard limit enabled. The signal is limited to the programmed threshold (signal saturates at the threshold)
	threshold
	floating point in range 0 to -96 dB
	attack
	floating point figure is dB/s rate at which gain is increased
	decay
	floating point figure is dB/s rate at which gain is reduced
*/
  audioShield.autoVolumeControl(2,1,0,-30.0,3.0,70.0); // see comments above
  audioShield.autoVolumeEnable();
#endif

// initialize the TFT and show signon message etc
  SPI.setMOSI(7); // set up HW SPI for use with the audio card - alternate pins
  SPI.setSCK(14);	
  init_display();
  setup_display();
/*      tft.fillRect(0,pos_y_frequency, 160,16,BLACK);
      tft.setCursor(0,pos_y_frequency);
      tft.setTextSize(2);
      tft.setTextColor(WHITE);
      tft.print("memory "); 
      tft.print(audiomem); 
      delay(1000);
      tft.setTextSize(1);
  setup_display();
*/              
  // If red button switch is pressed on boot, factory settings are loaded as start settings
  //delay (1000);
  //if (digitalRead(TuneSW)) 
  EEPROMLOAD(); // get saved frequencies, modes and calibration from EEPROM

// set up initial band and frequency
  show_band(bands[band].name);
  show_tunestep(tune_text);
  
// here startup message
    tft.fillRect(pos_x_frequency,pos_y_frequency, 160-pos_x_frequency,16,BLACK);
    tft.setCursor(pos_x_frequency,pos_y_frequency);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print("DD4WH SDR"); 
    tft.setCursor(pos_x_time,pos_y_time);
    tft.setTextSize(1);
    tft.setTextColor(WHITE);
    tft.print (ver); 
  delay (2000);
  tft.fillRect(pos_x_frequency,pos_y_frequency, 160-pos_x_frequency,16,BLACK); // erase for frequency display
    tft.fillRect(pos_x_time, pos_y_time, 80, 8, BLACK); // erase for time display
  audioShield.eqSelect (2); // Tone Control
  audioShield.eqBands (bass, treble); // (float bass, float treble) in % -100 to +100

  audioShield.inputSelect(AUDIO_INPUT_LINEIN); // RX mode uses line ins

  setup_gain();
  setup_mode(bands[band].mode);
  setup_RX(bands[band].mode, bands[band].bandwidthU, bands[band].bandwidthL);  // set up the audio chain for reception
    FFT_WINDOW_SET (Window_FFT);
    FFT_STRING = FFT_WINDOW_STRING (Window_FFT);
  audioShield.lineInLevel((float)bands[band].RFgain/10, (float)bands[band].RFgain/10);  

  if (!(SD.begin(10))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }


  si5351.init(SI5351_CRYSTAL_LOAD_10PF, 27000000);
//  si5351.drive_strength(SI5351_CLK0, SI5351_CLK_DRIVE_8MA); // does this work ???
  si5351.set_correction(calibration_constant);  
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  setfreq();
  delay(3);

  sinewave();          

} // end void SETUP

const char* filename (int pointer) {
      
      String NAME = "REC"+String(pointer)+".RAW";
      char str[15];
      NAME.toCharArray(str, sizeof(NAME));
      return str;
}

void controls() {
  // print time every second
      if (ms_100.check() == 1) { displayClock();
      AudioProcessorUsageMaxReset();
      AudioMemoryUsageMaxReset();}
      bouncer1.update();
      bouncer2.update();
      bouncer3.update();
      bouncer4.update();
      bouncer5.update();
      if ( bouncer1.fallingEdge()) { 
              Serial.print("Play Button Press");
              if (Rec_Mode == 1) stopRecording();
              if (Rec_Mode == 0){
                showtrack();
                Rec_Mode = 2;
                startPlaying();
              }
            
      } 
    
      if ( bouncer2.fallingEdge()) {  
              Serial.print("Stop Button Press");
              if (Rec_Mode == 1) stopRecording();
              if (Rec_Mode == 2) stopPlaying();
              Rec_Mode = 0;
              Switch_to_SDR();
              Menu = TUNING; Menu2 = TUNING;
              RECORDFLAG = 0;
      }
    
      if ( bouncer3.fallingEdge()) { 
                Serial.print("Record Button Press");
                if (Rec_Mode == 2) {
                      stopPlaying();
                      Switch_to_SDR();
                }
                if (Rec_Mode == 0) startRecording();       
            }
              
      if ( bouncer4.fallingEdge()) { 
                Serial.print("previous track Button Press ");
                prevtrack();
            }
      if ( bouncer5.fallingEdge()) { 
                Serial.print("next track Button Press");
                nexttrack();
            }
                        
            // If we're playing or recording, carry on...
            if (Rec_Mode == 1) {
                continueRecording();
            }
            if (Rec_Mode == 2) {
                continuePlaying();
            }
      }

   
void startRecording() {
  Serial.print("startRecording");
/*  if (SD.exists("RECORD.RAW")) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove("RECORD.RAW");
  } */
//  recfile = recfile + 1;
  String NAME = "REC"+String(playfile)+".RAW";
  char fi[15];
  NAME.toCharArray(fi, sizeof(NAME));
   if (SD.exists(fi)) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove(fi);
  }    
  frec = SD.open(fi, FILE_WRITE);
  if (frec) {
    recorder.begin();
    Rec_Mode = 1;
    tft.fillRect(0, 104, 160, 10, ST7735_RED);   //ST7735_BLACK);
    tft.setCursor(0, 105);
    tft.print (" Recording !");
    showtrack();
  }
}

void continueRecording() {
  if (recorder.available() >= 2) {
    byte buffer[512];
    // Fetch 2 blocks from the audio library and copy
    // into a 512 byte buffer.  The Arduino SD library
    // is most efficient when full 512 byte sector size
    // writes are used.
    memcpy(buffer, recorder.readBuffer(), 256);
    recorder.freeBuffer();
    memcpy(buffer+256, recorder.readBuffer(), 256);
    recorder.freeBuffer();
    // write all 512 bytes to the SD card
//    elapsedMicros usec = 0;
    frec.write(buffer, 512);
    // Uncomment these lines to see how long SD writes
    // are taking.  A pair of audio blocks arrives every
    // 5802 microseconds, so hopefully most of the writes
    // take well under 5802 us.  Some will take more, as
    // the SD library also must write to the FAT tables
    // and the SD card controller manages media erase and
    // wear leveling.  The recorder object can buffer
    // approximately 301700 us of audio, to allow time
    // for occasional high SD card latency, as long as
    // the average write time is under 5802 us.
    //    if (one_sec.check() == 1) {
    //      Serial.print("SD write, us=");
    //    }
    //    Serial.println(usec);
  }
}

void stopRecording() {
  Serial.print("stopRecording");
  recorder.end();
  if (Rec_Mode == 1) {
    while (recorder.available() > 0) {
      frec.write((byte*)recorder.readBuffer(), 256);
      recorder.freeBuffer();
    }
    frec.close();
//    playfile = recfile;
  }
  Rec_Mode = 0;
  clearname();
  tft.print (" Recording stopped!");

}


void startPlaying() {
      String NAME = "REC"+String(playfile)+".RAW";
      char fi[15];
      NAME.toCharArray(fi, sizeof(NAME));
       
  Serial.print("startPlaying ");
//  const char* fi = filename(playfile);
//  const char* fi = "REC13.RAW";
//  Serial.print ("File: ");Serial.print(playfile);
  Serial.println ("Playfile: "); Serial.print (playfile);
  Serial.println ("Name: "); Serial.print (filename(playfile));
  delay(100);
  Switch_to_MP3();
//  playSd.play(filename(playfile));
//  playSd.play("REC13.RAW");
    playSd.play(fi);

  Rec_Mode = 2;
  tft.fillRect(0, 104, 160, 10, ST7735_GREEN);   //ST7735_BLACK);
  tft.setCursor(0, 105);
  tft.setTextColor(ST7735_BLACK);
  tft.print (" Playing !");
  tft.setTextColor(ST7735_WHITE);
  showtrack();
}

void continuePlaying() {
  if (!playSd.isPlaying()) {
    playSd.stop();
    Rec_Mode = 0;
    clearname();
    tft.print("End of recording");
  }
}

void stopPlaying() {
  Serial.print("stopPlaying");
  if (Rec_Mode == 2) playSd.stop();
  Rec_Mode = 0;
  clearname();
  tft.print (" Playing stopped");

}

void prevtrack() {

    playfile = playfile - 1;
    if (playfile < 0) playfile = 0;
    if (playfile > 255) playfile = 255;
    showtrack();
    if (Rec_Mode == 2) {
        stopPlaying(); // old track
        startPlaying();// new track = old track -1  
    }
//    if (Rec_Mode == 0) startPlaying();       

}

void nexttrack() {
    playfile = playfile + 1;
    if (playfile < 0) playfile = 0;
    if (playfile > 255) playfile = 255;
    showtrack();
    if (Rec_Mode == 2) {
        stopPlaying();
        startPlaying();  
    }
}


void showtrack() {
           if (Rec_Mode == 1) {
            tft.fillRect(pos_x_menu, pos_y_menu - 14, 60, 9, ST7735_RED); // REC
            tft.setTextColor(WHITE);
           }
           if (Rec_Mode == 2) {
            tft.fillRect(pos_x_menu, pos_y_menu - 14, 60, 9, ST7735_GREEN); // PLAY
            tft.setTextColor(BLACK);
           }
           if (Rec_Mode == 0) {
            tft.fillRect(pos_x_menu, pos_y_menu - 14, 60, 9, BLACK); // STOP
            tft.setTextColor(WHITE);
           }
           tft.setCursor(pos_x_menu, pos_y_menu - 14);
           tft.print("Track: "); tft.print(playfile);
           // hier printen den track
           Serial.print(playfile);
           Serial.print("  ");
//           Serial.print(recfile);  Serial.print("  ");
}

void Switch_to_MP3() {
  // set gain values of SDR chain to 0
  audioShield.eqBands (0.7, 0.0); // (float bass, float treble) in % -100 to +100
  audio_in_I.gain(0, 0); // take values from EEPROM
  audio_in_I.gain(1, 0);  // take values from EEPROM
  audio_in_I.gain(2,0); //not used
  audio_in_I.gain(3,0); //not used
  audio_in_Q.gain(0, 0); // always 1, only I gain is calibrated
  audio_in_Q.gain(1, 0); // take values from EEPROM
  audio_in_Q.gain(2,0); //not used
  audio_in_Q.gain(3,0); //not used

/*  add_substract_I.gain(0, 0);
  add_substract_I.gain(1,0); // substract
  add_substract_Q.gain(0,0);
  add_substract_Q.gain(0,0); // add
*/
// set audio chain to MP3/M4A 
  mixleft.gain(1, 0); //AAC on
  mixright.gain(1,0);
  mixleft.gain(0,0); // MP3 on
  mixright.gain(0,0);
  mixleft.gain(3, 1.0); // RAW on
  mixright.gain(3, 1.0);
  mixleft.gain(2,0); // radio off
  mixright.gain(2,0);

// switch of IF generators
//  cosIF.amplitude(0);
//  sinIF.amplitude(0);

// switch of FIR filters
  FIR_I.end();
  FIR_Q.end();
  } // end void Switch_to_MP3

void Switch_to_SDR() {
    setup_display();
    show_tunestep(tune_text);
    setfreq(); // just to show right frequency !
    setup_gain();
    setup_mode (bands[band].mode);
    setup_RX(bands[band].mode, bands[band].bandwidthU, bands[band].bandwidthL);
    // set audio chain to SDR 
//    bass = 0.5; bass_help = 50;
//    treble = -0.8; treble_help = -80;
    audioShield.eqBands (bass, treble); // (float bass, float treble) in % -100 to +100
    clearname();
    tft.setTextColor(ST7735_WHITE);
    tft.setTextWrap(true);
    tft.print("Switching to SDR Radio");
    delay (1000); // prevent unwanted switching of mode button
    clearname();
    } // end void Switch_to_SDR


void setfreq () {
   hilfsf = bands[band].freq*10000000*MASTER_CLK_MULT*SI5351_FREQ_MULT;
//   hilfsf = bands[band].freq*10000000UL*MASTER_CLK_MULT*SI5351_FREQ_MULT;
   hilfsf = hilfsf / calibration_factor;
   si5351.set_freq(hilfsf, SI5351_PLL_FIXED, SI5351_CLK2);
//   show_frequency(bands[band].freq - IF_FREQ);  // frequency we are listening to, Rx below IF
   show_frequency(bands[band].freq + IF_FREQ);  // frequency we are listening to, Rx above IF

// print Station names 
// printstationname();

if (Menu == TUNING) {
switch (bands[band].freq + IF_FREQ) {
    case 225000:
        clearname();
        tft.print ("Polskie Radio Jedynka");
      break;
      case 216000:
        clearname();
        tft.print ("RMC, Roumoules");
      break;
      case 183000:
        clearname();
        tft.print ("Europe 1, Saarlouis");
      break;
      case 207000:
        clearname();
        tft.print ("Rikisutvarpid, ISL");
      break;
    case 252000:
        clearname();
        tft.print ("Chaine 3, ALG & RTE 1, IRL");
      break;
    case 234000:
        clearname();
        tft.print ("RTL, LUX");
      break;
    case 270000:
        clearname();
        tft.print ("CRo Radiozurnal, CZ");
      break;
    case 279000:
        clearname();
        tft.print ("BR Pershy Kanal, BLR");
      break;
    case 162000:
        clearname();
        tft.print ("France Inter");
      break;
    case 171000:
        clearname();
        tft.print ("Medi 1, ALG");
      break;
      case 531000:
        clearname();
        tft.print ("JIL FM, Algeria");
      break;
    case 549000:
        clearname();
        tft.print ("Deutschlandfunk, D");
      break;
    case 639000:
        clearname();
        tft.print ("CRo Dvojka, Praha, CZ");
        break;
    case 1494000:
        clearname();
        tft.print ("France Info & Bleu Corse");
      break;
    case 11735000:
        clearname();
        tft.print ("Zanzibar BC, Tanzania");
      break;
    case 1215000:
        clearname();
        tft.print ("Absolute Radio, UK");
      break;
    default:
        tft.fillRect(0, 104, 160, 10,ST7735_BLACK);
      break;
  }}


// LPF switching follows here
// Only five filter banks there:
// longwave LPF 295kHz, mediumwave I LPF 955kHz, mediumwave II LPF 2MHz, tropical bands LPF 5.4MHz, others LPF LPF 30MHz
///*
   if (((bands[band].freq + IF_FREQ) < 955001) && ((bands[band].freq + IF_FREQ) > 300001)) { 
     digitalWrite (Band3, HIGH);
     digitalWrite (Band1, LOW); digitalWrite (Band2, LOW); digitalWrite (Band4, LOW); digitalWrite (Band5, LOW);
     } // end if
   if (((bands[band].freq + IF_FREQ) > 955000) && ((bands[band].freq + IF_FREQ) < 1996001)) {
     digitalWrite (Band1, HIGH);
     digitalWrite (Band5, LOW); digitalWrite (Band3, LOW); digitalWrite (Band4, LOW); digitalWrite (Band2, LOW);
     } // end if
   if (((bands[band].freq + IF_FREQ) > 1996000) && ((bands[band].freq + IF_FREQ) < 5400001)) {
     digitalWrite (Band2, HIGH);
     digitalWrite (Band4, LOW); digitalWrite (Band3, LOW); digitalWrite (Band1, LOW); digitalWrite (Band5, LOW);
     } // end if
   if ((bands[band].freq + IF_FREQ) > 5400000) { 
    // && ((bands[band].freq + IF_FREQ) < 12500001)) {
     digitalWrite (Band4, HIGH);
     digitalWrite (Band1, LOW); digitalWrite (Band3, LOW); digitalWrite (Band2, LOW); digitalWrite (Band5, LOW);
     } // end if
      // I took out the 12.5MHz lowpass and inserted the 30MHz instead - I have to live with 3rd harmonic images in the range 5.4 - 12Mhz now
      // maybe this is more important than the 5.4 - 2Mhz filter ?? Maybe swap them sometime, because I only got five filter relays . . .
      /*   if ((bands[band].freq + IF_FREQ) > 12500000) {
     digitalWrite (Band5, HIGH);
     digitalWrite (Band2, LOW); digitalWrite (Band3, LOW); digitalWrite (Band4, LOW); digitalWrite (Band1, LOW);
     } // end if
     */
    // this is the brandnew longwave LPF (cutoff ca. 295kHz)
   if ((bands[band].freq + IF_FREQ) < 300000) {
     digitalWrite (Band5, HIGH);
     digitalWrite (Band2, LOW); digitalWrite (Band3, LOW); digitalWrite (Band4, LOW); digitalWrite (Band1, LOW);
     } // end if
   
//*/
  

} //end void setfreq

void clearname() {
        
        tft.fillRect(0, 104, 160, 10,152);  
        tft.setCursor(0, 105);
} // end void clearname


void loop() 
{
  static uint8_t modesw_state=0;
  static uint8_t button5sw_state=0;
  static uint8_t Bandupsw_state=0;
  static uint8_t Banddownsw_state=0;
  static uint8_t tunesw_state = 0, menusw_state=0; 
  static uint8_t turns = 64;
  static long encoder_pos=0, last_encoder_pos=0;
  long encoder_change;

    if (RECORDFLAG == 1) controls();
// tune radio and adjust settings in menus using encoder switch  
  encoder_pos=tune.read();
  
  if (encoder_pos != last_encoder_pos){
      encoder_change=encoder_pos-last_encoder_pos;
      last_encoder_pos=encoder_pos;
   
   if (Menu == TUNING && Menu2 == TUNING) {
    if (((band == BAND_LW) || (band == BAND_MW)) && (tunestep == TUNE_STEP1)) 
        tunestep = 9000;
    
    if (((band != BAND_LW) && (band != BAND_MW)) && (tunestep == 9000))
      tunestep = TUNE_STEP1;

    bands[band].freq+=encoder_change*tunestep;  // tune the master vfo 
    if (bands[band].freq > F_MAX) bands[band].freq = F_MAX;
    if (bands[band].freq < F_MIN) bands[band].freq = F_MIN;
    setfreq();

        // this is SAM mode, when you tune in 5kHz steps and frequency is automatically tuned to carrier
        if (tunestep == TUNE_STEP1 && bands[band].mode == modeSAM) { // temporarily switched off! 
        // define boundaries in which the maximum output bin is being searched for        

        // for this mode, we will search in +-3.4kHz
        
//        Lbin = posbin - (bands[band].bandwidthL / binBW);
//        Ubin = posbin + (bands[band].bandwidthU / binBW);
        Lbin = posbin - 22;
        Ubin = posbin + 22;
        
        Serial.print (" Lbin = "); Serial.print (Lbin);
        Serial.print (" Ubin = "); Serial.print (Ubin);
//        for (int x = 0; x < 2; x++) {
        for (int c = Lbin; c <= Ubin; c++) { // search for FFT bin with highest value = carrier and save the no. of the bin in maxbin 
        while (!myFFT.available());
        if (maximum < myFFT.output[c]) {
            maximum = myFFT.output[c];
            maxbin = c;
        }}
        if (maximum == 0) return;
            // ok, we have found the maximum, now set frequency to that bin
            delta = (maxbin - posbin) * binBW;
            bands[band].freq = bands[band].freq + delta;
            setfreq();
            Serial.print (" maxbin = "); Serial.print (maxbin);
            Serial.print (" Maximum = "); Serial.print (myFFT.output[maxbin]);
            Serial.print (" delta = "); Serial.print (delta);
            show_spectrum(bands[band].RFgain/10, LPFcoeff);        

        // estimate frequ of carrier by three-point-interpolation of bins 94, 95, 96
        bin1 = myFFT.output[posbin-1];
        bin2 = myFFT.output[posbin];
        bin3 = myFFT.output[posbin+1];
        if (bin1+bin2+bin3==0) bin1=1; // prevent divide by 0
        // formula by (Jacobsen & Kootsookos 2007) equation (4) P=1.36 for Hanning window FFT function
        delta = binBW * (1.36 * (bin3 - bin1)) / (bin1 + bin2 + bin3);
        bands[band].freq = bands[band].freq + delta;
        setfreq();
        Serial.print (" bin 94 = "); Serial.print (bin1);
        Serial.print (" bin 95 = "); Serial.print (bin2);
        Serial.print (" bin 96 = "); Serial.print (bin3);
        Serial.print (" delta = "); Serial.print (delta);
        }
  //        } // for (5 times)
    if (tunestep == TUNE_STEP1 || tunestep == TUNE_STEP3) show_spectrum(bands[band].RFgain/10, LPFcoeff);

  } // end TUNING
  
  if (Menu == BWADJUST) {
            bands[band].bandwidthL = bands[band].bandwidthL + encoder_change*BW_step; 
            bands[band].bandwidthU = bands[band].bandwidthU + encoder_change*BW_step; 
      if (bands[band].mode == modeLSB)
            bands[band].bandwidthU = 0;
      if (bands[band].mode == modeUSB)
            bands[band].bandwidthL = 0;

    if(bands[band].bandwidthU < 0) bands[band].bandwidthU = 0;
    if (bands[band].bandwidthU > MAX_BANDWIDTH) bands[band].bandwidthU = MAX_BANDWIDTH;
    if(bands[band].bandwidthL < 0) bands[band].bandwidthL = 0;
    if (bands[band].bandwidthL > MAX_BANDWIDTH) bands[band].bandwidthL = MAX_BANDWIDTH;
    show_bandwidth(bands[band].mode, bands[band].bandwidthU, bands[band].bandwidthL); // draw bandwidth indicator under spectrum display
    setup_RX(bands[band].mode, bands[band].bandwidthU, bands[band].bandwidthL); 
    } // END BWadjust
  
 //experimental NOTCH FILTER 
//  if (Menu2 == NOTCH) { 
  if (Menu == NOTCH) { 
    notchF = notchF + encoder_change*10;
    if(notchF < -11000) notchF = -11000;
    if (notchF > 11000) notchF = 11000;
    if (bands[band].mode == modeLSB && notchF >= -200) notchF = -200;     
    if (bands[band].mode == modeUSB && notchF < 200) notchF = 200;     
    show_notch(notchF, bands[band].mode); // draw notch indicator into spectrum display
    if (notchF < 400 && notchF > -400) { // notch indicator is in centre position, which means OFF
            tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
            tft.setCursor(100, 105);
            tft.setTextColor(GREEN);
            tft.print ("Notch OFF");
            tft.setTextColor(WHITE);  
    }
    else {
      // here real notch 
            tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
            tft.setCursor(100, 105);
            tft.setTextColor(GREEN);
            tft.print (notchF); tft.print ("Hz");     // Linkwitz-Riley filter, 48 dB/octave
            tft.setTextColor(WHITE);
            setup_RX(bands[band].mode, bands[band].bandwidthU, bands[band].bandwidthL);
    } // end ELSE
   } // END NOTCH

  if (Menu == TIMEADJUST) {
      helpmin = minute(); helphour = hour();
      helpmin = helpmin + encoder_change;
      if (helpmin > 59) { 
        helpmin = 0; helphour = helphour +1;}
       if (helpmin < 0) {
        helpmin = 59; helphour = helphour -1; }
       if (helphour < 0) helphour = 23; 
      if (helphour > 23) helphour = 0;
      helpmonth = month(); helpyear = year(); helpday = day();
      setTime (helphour, helpmin, 0, helpday, helpmonth, helpyear);      
      Teensy3Clock.set(now()); // set the RTC  
    } // end TIMEADJUST

    if (Menu == DATEADJUST) {
      helpyear = year(); helpmonth = month();
      helpday = day();
      helpday = helpday + encoder_change;
      if (helpday < 1) {helpday=31; helpmonth=helpmonth-1;}
      if (helpday > 31) {helpmonth = helpmonth +1; helpday=1;}
      if (helpmonth < 1) {helpmonth = 12; helpyear = helpyear-1;}
      if (helpmonth > 12) {helpmonth = 1; helpyear = helpyear+1;}
      helphour=hour(); helpmin=minute(); helpsec=second(); 
      setTime (helphour, helpmin, helpsec, helpday, helpmonth, helpyear);      
      Teensy3Clock.set(now()); // set the RTC
      displayDate();
          } // end DATEADJUST
 
    if (Menu == CALIBRATIONFACTOR) {
      calibration_factor = calibration_factor + encoder_change;
        tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
        tft.setCursor(100, 105);
         //  sprintf(string,"S:9+%02.0f dB",dbuv);           
       tft.print (calibration_factor);
       si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
       setfreq();
        } // end CALIBRATIONFACTOR
      
    if (Menu == CALIBRATIONCONSTANT) {
      calibration_constant = calibration_constant + encoder_change*10;
        tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
        tft.setCursor(100, 105);
         //  sprintf(string,"S:9+%02.0f dB",dbuv);           
       tft.print (calibration_constant);
        si5351.set_correction(calibration_constant); 
        si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
        setfreq();
      } // end CALIBRATIONCONSTANT
   
   if (Menu == LOADFROMEEPROM) {
        tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
            tft.setCursor(0, 105);
            tft.print ("Turn till 0 to load:   ");
        turns = turns - encoder_change;
        tft.print (turns); //, " steps till load!"); 
        if (turns == 0) {
        turns = 64;
        tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
        tft.setCursor(0, 105);
        tft.print ("Loaded from EEPROM!"); 
        delay (2000);
        last_encoder_pos = encoder_pos;
        encoder_change = 0;
        tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
        Menu = TUNING;
         show_tunestep(tune_text);
         EEPROMLOAD(); // load from EEPROM
         setfreq();
         setup_gain();
         audioShield.lineInLevel((float)bands[band].RFgain/10, (float)bands[band].RFgain/10);         
         setup_mode(bands[band].mode);
         setup_RX(bands[band].mode, bands[band].bandwidthU, bands[band].bandwidthL);  // set up the audio chain for new mode
         }
              } // end Load from EEPROM

   if (Menu == SAVETOEEPROM) {
          tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
            tft.setCursor(0, 105);
            tft.print ("Turn till 0 to save:   ");
            turns = turns - encoder_change;
        //tft.setCursor(0, 105);
          tft.print (turns); //, " steps till save!"); 
        if (turns == 0) {
        turns = 64;
        
        tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
        //tft.fillRect(0, 119, 160, 7,ST7735_BLACK);
        tft.setCursor(0, 105);
        tft.print ("Saved to EEPROM!"); 
          delay (2000);
          last_encoder_pos = encoder_pos;
        encoder_change = 0;
        tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
        Menu = TUNING;
         show_tunestep(tune_text);
         EEPROMSAVE();  // save to EEPROM
                  }
              } // end Save to EEPROM

  if (Menu == IQADJUST) {
    I_gain = I_help * 1000;
    I_gain=I_gain+encoder_change;
    if (I_gain > 1500) I_gain=1500;
    if (I_gain <1) I_gain=1;
    I_help = I_gain/1000;
          tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
            tft.setCursor(0, 105);
            tft.print (I_gain);
      AudioNoInterrupts();   // Disable Audio while reconfiguring filters
        //   audio_in_I.gain(0,I_help*vol); // sets I gain on input I
           audio_in_I.gain(0,I_help); // sets I gain on input I
      AudioInterrupts();
    } // END IQadjust

  if (Menu == LPFSPECTRUM) {
    LPFcoeffhelp = LPFcoeff * 100;
    LPFcoeffhelp = LPFcoeffhelp + encoder_change;
    if (LPFcoeffhelp > 100) LPFcoeffhelp = 100;
    if (LPFcoeffhelp < 1) LPFcoeffhelp = 1;
    LPFcoeff = LPFcoeffhelp / 100;
          tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
          tft.setCursor(0, 105);
          tft.print (LPFcoeffhelp);
    } // END LPFSPECTRUM

  
  if (Menu == IQPHASEADJUST) {
    // get values of I_in_Q / Q_in_I
    if (I_in_Q != 0) {
      IQcounter = - I_in_Q_help * 1000;
    } else IQcounter = Q_in_I_help * 1000;
    IQcounter = IQcounter + encoder_change;
    if (IQcounter < 0) {
    I_in_Q = - IQcounter;
    Q_in_I = 0;
    }
    else {
      Q_in_I = IQcounter;
      I_in_Q = 0;
      }  
    Q_in_I_help = Q_in_I / 1000;
    I_in_Q_help = I_in_Q / 1000;
          tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
            tft.setCursor(0, 105);
            if (I_in_Q != 0)
            tft.print (-I_in_Q);
            else tft.print (Q_in_I);
          AudioNoInterrupts();   // Disable Audio while reconfiguring filters
         audio_in_I.gain(1,Q_in_I_help); // sets I gain on input I
         audio_in_Q.gain(1,I_in_Q_help);
          AudioInterrupts();
    } // END IQadjust

   if (Menu == FFTWINDOW) {
      //wind = Window_FFT * 4;
      Window_FFT = Window_FFT + encoder_change;
      //Window_FFT = wind / 4;
      if (Window_FFT < 0) Window_FFT = 0;
      if (Window_FFT > 10) Window_FFT = 10;

     FFT_WINDOW_SET (Window_FFT);
     FFT_STRING = FFT_WINDOW_STRING (Window_FFT);
      tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
      tft.setCursor(100, 105);
     tft.print (FFT_STRING);
       } // end FFTWINDOW

   if (Menu == RECORDING && RECORDFLAG != 1) {
        if (turns == 64) turns = 16;
        tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
        tft.setCursor(0, 105);
        tft.print ("Turn till 0 to REC:   ");
        turns = turns - encoder_change;
        //tft.setCursor(0, 105);
          tft.print (turns); //, " steps till save!"); 
        if (turns == 0) {
        turns = 64;
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("REC Mode"); 
        tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
        //tft.fillRect(0, 119, 160, 7,ST7735_BLACK);
        tft.setCursor(0, 105);
        tft.print ("Record Menu!"); 
          delay (1000);
          last_encoder_pos = encoder_pos;
        encoder_change = 0;
        tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
        RECORDFLAG = 1;
   }
   } // end if Menu == RECORDING

   if (Menu2 == SNAP) {
        if (turns == 64) turns = 5;
        tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
        tft.setCursor(0, 105);
        tft.print ("Turn till 0 to SNAP:   ");
        turns = turns - encoder_change;
        //tft.setCursor(0, 105);
          tft.print (turns); //, " steps till snap!"); 
        if (turns == 0) {
        turns = 64;
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("Carrier !"); 
           tft.fillRect(0, 105, 160, 7,ST7735_BLACK);
           last_encoder_pos = encoder_pos;
           encoder_change = 0;
        }

        // define boundaries in which the maximum output bin is being searched for        
        Lbin = posbin - (bands[band].bandwidthL / binBW);
        Ubin = posbin + (bands[band].bandwidthU / binBW);
        Serial.print (" Lbin = "); Serial.print (Lbin);
        Serial.print (" Ubin = "); Serial.print (Ubin);

        for (int c = Lbin; c <= Ubin; c++) { // search for FFT bin with highest value = carrier and save the no. of the bin in maxbin 
        if (maximum < myFFT.output[c]) {
            maximum = myFFT.output[c];
            maxbin = c;
        }}
        maximum = 0; // reset maximum for next time ;-)

        // ok, we have found the maximum, now set frequency to that bin
        delta = (maxbin - posbin) * binBW;
        bands[band].freq = bands[band].freq + delta;
        setfreq();
        Serial.print (" maxbin = "); Serial.print (maxbin);
        Serial.print (" Maximum = "); Serial.print (myFFT.output[maxbin]);
        Serial.print (" delta = "); Serial.print (delta);
        delay(1);
        
        // estimate frequ of carrier by three-point-interpolation of bins around posbin (which is the Rx frequency bin either 95 or 95+64)
        bin1 = myFFT.output[posbin-1];
        bin2 = myFFT.output[posbin];
        bin3 = myFFT.output[posbin+1];

        // formula by (Jacobsen & Kootsookos 2007) equation (4) P=1.36 for Hanning window FFT function
        delta = binBW * (1.36 * (bin3 - bin1)) / (bin1 + bin2 + bin3);
        bands[band].freq = bands[band].freq + delta;
        setfreq();
        Serial.print (" bin 94 = "); Serial.print (bin1);
        Serial.print (" bin 95 = "); Serial.print (bin2);
        Serial.print (" bin 96 = "); Serial.print (bin3);
        Serial.print (" delta = "); Serial.print (delta);
        delay(1);

   } // end if Menu == SNAP

   if (Menu2 == RFGAIN) {
//      line_in_gain_help = bands[band].RFgain *10;
      bands[band].RFgain = bands[band].RFgain + encoder_change;
      if (bands[band].RFgain < 0) bands[band].RFgain = 0;
      if (bands[band].RFgain > 150) bands[band].RFgain = 150;
//      bands[band].RFgain = line_in_gain_help / 10;
        tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
        tft.setCursor(100, 105);
        tft.print (bands[band].RFgain/10);
      audioShield.lineInLevel((float)bands[band].RFgain/10, (float)bands[band].RFgain/10);  
              } // end RFGAIN


// add Menu2 here!
    if (Menu2 == PASSBAND) {
                    passbandBW = bands[band].bandwidthL + bands[band].bandwidthU;
                        // increase/decrease LSB bandwidth
                    bands[band].bandwidthL = bands[band].bandwidthL - encoder_change * BW_step;
                        // decrease/increase USB bandwidth
                    bands[band].bandwidthU = bands[band].bandwidthU + encoder_change * BW_step;

                if (bands[band].bandwidthL < 0) bands[band].bandwidthL = 0;
                if (bands[band].bandwidthU < 0) bands[band].bandwidthU = 0;
                if (bands[band].bandwidthU > passbandBW) bands[band].bandwidthU = passbandBW;
                if (bands[band].bandwidthL > passbandBW) bands[band].bandwidthL = passbandBW;
                if (bands[band].bandwidthU > MAX_BANDWIDTH) bands[band].bandwidthU = MAX_BANDWIDTH;
                if (bands[band].bandwidthL > MAX_BANDWIDTH) bands[band].bandwidthL = MAX_BANDWIDTH;
                setup_RX(bands[band].mode, bands[band].bandwidthU, bands[band].bandwidthL);  
                show_bandwidth(bands[band].mode, bands[band].bandwidthU, bands[band].bandwidthL); // draw bandwidth indicator under spectrum display
    } // end Menu2 == PASSBAND

    if (Menu2 == BASS) {     
                 bass_help = bass_help + encoder_change;
                  if (bass_help < -100) bass_help = -100;
                  if (bass_help > 100) bass_help = 100;
                  bass = bass_help / 100;
              tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
              tft.setCursor(100, 105);
              tft.print (bass_help);
              audioShield.eqBands (bass, treble); // (float bass, float treble) in % -100 to +100
    } // end Menu2 == BASS

    if (Menu2 == TREBLE) {     
                 treble_help = treble_help + encoder_change;
                  if (treble_help < -100) treble_help = -100;
                  if (treble_help > 100) treble_help = 100;
                  treble = treble_help / 100;
              tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
              tft.setCursor(100, 105);
              tft.print (treble_help);
              audioShield.eqBands (bass, treble); // (float bass, float treble) in % -100 to +100
    } // end Menu2 == TREBLE
      
    } // end Encoder !
  
    
 // print time every 0.1 second
   if (ms_100.check() == 1) displayClock();
  
// every 50 ms, adjust the volume and check the switches
  if (ms_50.check() == 1) {
    vol = analogRead(15);
    vol = vol / 1023.0;
        // OLD
        //    audio_in_I.gain(0, I_help*vol); // Volume pot does adjust digital gain of the I and Q input lines
        //    audio_in_Q.gain(0, vol);//
    
    audioShield.volume(vol); // Volume pot does adjust analog gain of headphone amp

    float Lout =  (1.0 - vol) * 18 + 13;
    int lineout = (int) Lout;
//    audioShield.lineOutLevel (31,31);
    audioShield.lineOutLevel((1.0 - vol) * 18 + 13, (1.0 - vol) * 18 + 13); // 13 loudest, 31 lowest
//    Serial.print(" LOut: "); Serial.print(Lout);    Serial.print(" lineout: "); Serial.print(lineout);

 //###################################
    if (!digitalRead(MenuSW) && (RECORDFLAG != 1)) {
       if (menusw_state == 0) { // switch was pressed - falling edge
          tft.fillRect(0, 104, 160, 10,ST7735_BLACK);
         if (Menu2 != TUNING) {// escape button from menu2
         Menu2 = TUNING; Menu = TUNING-1;
         }
         if(++Menu > last_menu) Menu = first_menu;
         if (Menu == BWADJUST){
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("BW adjust");
            } 
            
          if (Menu == TUNING && Menu2 == TUNING) {
            show_tunestep(tune_text);
          }
          
          if (Menu == TIMEADJUST) {
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("Time Set"); 
           } 

          if (Menu == DATEADJUST) {
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("Date Set");
           //displayDate();
         } 
           if (Menu == NOTCH) {
           tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("Notch"); 
            if (notchF < 400 && notchF > -400) { // notch indicator is in centre position, which means OFF
            tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
            tft.setCursor(100, 105);
            tft.setTextColor(GREEN);
            tft.print ("Notch OFF");
            tft.setTextColor(WHITE);  
              }
            else {
            // here real notch 
            tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
            tft.setCursor(100, 105);
            tft.setTextColor(GREEN);
            tft.print (notchF); tft.print ("Hz");     // Linkwitz-Riley filter, 48 dB/octave
            tft.setTextColor(WHITE);
                } // end ELSE
           }         
           if (Menu == CALIBRATIONFACTOR) {
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("CaliFactor"); 
           tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
           tft.setCursor(100, 105);
           tft.print (calibration_factor); 
           } 
           
           if (Menu == CALIBRATIONCONSTANT)  {
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("CaliConst"); 
           tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
           tft.setCursor(100, 105);
           tft.print (calibration_constant); 
           } 
                      
           if (Menu == LOADFROMEEPROM) {
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("Load ?"); 
           } 
                                 
           if (Menu == RECORDING) {
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           if (RECORDFLAG == 1) tft.print ("REC Mode"); 
           else tft.print("Recording"); 
           } 
           
           if (Menu == SAVETOEEPROM) {
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("Save ?"); 
           } 
           if (Menu == IQADJUST) {
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("IQ Ampl.");
           tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
           tft.setCursor(100, 105);
           tft.print (I_gain);
           }
           if (Menu == LPFSPECTRUM) {
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("LPF Spectr.");
           tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
           tft.setCursor(100, 105);
           tft.print (LPFcoeffhelp); 
           }
           if (Menu == IQPHASEADJUST) {
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("IQ Phase");
           tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
           tft.setCursor(100, 105);
           if (I_in_Q != 0)
           tft.print (-I_in_Q);
           else tft.print (Q_in_I);
           }
          if (Menu == FFTWINDOW) {
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("FFT Window"); 
           FFT_STRING = FFT_WINDOW_STRING (Window_FFT);
           tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
           tft.setCursor(100, 105);
           tft.print (FFT_STRING);
           }

        menusw_state=1; // flag switch is pressed
       }
    }
    else menusw_state=0; // flag switch not pressed

  
// #######################################
         
     if (!digitalRead(TuneSW)) {
       if (tunesw_state == 0) { // switch was pressed - falling edge
         tunehelp = tunehelp +1;
         if(tunehelp > last_tunehelp) tunehelp = first_tunehelp;
           if (tunehelp == 1)  {tunestep=TUNE_STEP1; tune_text="Fast Tune";}
           if (tunehelp == 2)  {tunestep=TUNE_STEP2; tune_text="Precision";}
     //      if (tunehelp == 3)   {tunestep=TUNE_STEP3; tune_text="9kHz Tune";}
    //       if (tunehelp == 4)  {tunestep=TUNE_STEP4; tune_text="Precision";}
           if (tunehelp == 3)   {tunestep=TUNE_STEP3; tune_text="100Hz Tune";}
         show_tunestep(tune_text);
        Menu = TUNING; 
        tunesw_state=1; // flag switch is pressed
       }
    }
    else tunesw_state=0; // flag switch not pressed
   
    if (!digitalRead(ModeSW) && RECORDFLAG != 1) {
       if (modesw_state==0) { // switch was pressed - falling edge
       if(++bands[band].mode > lastmode) bands[band].mode=firstmode; // cycle thru radio modes 
         if (bands[band].mode == modeUSB && notchF <=-400) notchF = notchF *-1; // this flips the notch filter round, when you go from LSB --> USB and vice versa
         if (bands[band].mode == modeLSB && notchF >=400) notchF = notchF *-1;
         setup_mode(bands[band].mode);
         setup_RX(bands[band].mode, bands[band].bandwidthU, bands[band].bandwidthL);  // set up the audio chain for new mode                            
         show_bandwidth(bands[band].mode, bands[band].bandwidthU, bands[band].bandwidthL);
         modesw_state=1; // flag switch is pressed
       }
    }
    else modesw_state=0; // flag switch not pressed; end ModeSW

    if (!digitalRead(Button5) && (RECORDFLAG != 1)) {
       if (button5sw_state==0) { // switch was pressed - falling edge
          tft.fillRect(0, 104, 160, 10,ST7735_BLACK);
         if (Menu == TUNING) {
         if(++Menu2 > last_menu2) Menu2 = first_menu2;
         // PASSBAND can only be meaningfully adjusted in DSB or StereoAM mode, otherwise skip Menu2 PASSBAND
         if (Menu2 == PASSBAND && ((bands[band].mode != modeDSB) && (bands[band].mode != modeStereoAM))) Menu2 = Menu2+1;
         if (Menu2 == RFGAIN){
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("RF Gain");
           tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
           tft.setCursor(100, 105);
           tft.print (bands[band].RFgain/10); 
         }
        if (Menu2 == PASSBAND) {
           tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("Passband"); 
           } 
        if (Menu2 == SNAP) {
           tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("Snap ?"); 
           } 
       if (Menu2 == BASS) {
           tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("Bass"); 
           tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
           tft.setCursor(100, 105);
           tft.print (bass_help); 
           } 
        if (Menu2 == VOLTAGE) {
           tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("Battery"); 
           clearname();
           tft.print ("Voltage is ");
           tft.print(analogRead(VoltCheck)/54.95);
           tft.print(" Volts");
           }

        if (Menu2 == VERSION) {
           tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("Software"); 
           clearname();
           tft.print ("Version is ");
           tft.print(ver);
           }
            
        if (Menu2 == TREBLE) {
           tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
           tft.fillRect(pos_x_menu, pos_y_menu, 60, 8, BLACK); // erase old string
           tft.setTextColor(WHITE);
           tft.setCursor(pos_x_menu, pos_y_menu);
           tft.print("Treble"); 
           tft.fillRect(100, 105, 70, 7,ST7735_BLACK);
           tft.setCursor(100, 105);
           tft.print (treble_help); 
           }
        if (Menu2 == TUNING) {
           tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
           show_tunestep(tune_text);
           }
         } else {
            Menu = TUNING; Menu2 = TUNING;
            tft.fillRect(100, 105, 70, 8,ST7735_BLACK);
            show_tunestep(tune_text);
           }
         button5sw_state=1; // flag switch is pressed
       }
    }
    else button5sw_state=0; // flag switch not pressed
    
    
    if (!digitalRead(BandUPSW) && RECORDFLAG != 1) {
       if (Bandupsw_state==0) { // switch was pressed - falling edge
         if(++band > LAST_BAND) band=FIRST_BAND; // cycle thru radio bands 
         show_band(bands[band].name); // show new band
         setup_mode(bands[band].mode);
         audioShield.lineInLevel((float)bands[band].RFgain/10, (float) bands[band].RFgain/10);
         setup_RX(bands[band].mode, bands[band].bandwidthU, bands[band].bandwidthL);  // set up the audio chain for new mode
         setfreq();
         Bandupsw_state=1; // flag switch is pressed
       }
    }
    else Bandupsw_state=0; // flag switch not pressed  
    
    
    if (!digitalRead(BandDOWNSW) && RECORDFLAG != 1) {
       if (Banddownsw_state==0) { // switch was pressed - falling edge
         if(--band < FIRST_BAND) band=LAST_BAND; // cycle thru radio bands 
         show_band(bands[band].name); // show new band
         setup_mode(bands[band].mode);
         audioShield.lineInLevel((float)bands[band].RFgain/10, (float) bands[band].RFgain/10);
         setup_RX(bands[band].mode, bands[band].bandwidthU, bands[band].bandwidthL);  // set up the audio chain for new mode
         setfreq();
         Banddownsw_state=1; // flag switch is pressed
       }
    }
    else Banddownsw_state=0; // flag switch not pressed

  
  }


#ifdef SW_AGC
 if (Menu == TUNING)  agc();  // Automatic Gain Control function
#endif  




  //
  // Draw Spectrum Display
  //
//  if ((lcd_upd.check() == 1) && myFFT.available()) show_spectrum();
  if ((lcd_upd.check() == 1) && RECORDFLAG != 1) show_spectrum(bands[band].RFgain/10, LPFcoeff);

#ifdef CW_WATERFALL
  if ((waterfall_upd.check() == 1) && myFFT.available()) show_waterfall();
#endif

#ifdef AUDIO_STATS
  //
  // DEBUG - Microcontroller Load Check
  //
  // Change this to if(1) to monitor load

  /*
  For PlaySynthMusic this produces:
  Proc = 20 (21),  Mem = 2 (8)
  */  
    if (five_sec.check() == 1)
    {
      Serial.print("Proc = ");
      Serial.print(AudioProcessorUsage());
      Serial.print(" (");    
      Serial.print(AudioProcessorUsageMax());
      Serial.print("),  Mem = ");
      Serial.print(AudioMemoryUsage());
      Serial.print(" (");    
      Serial.print(AudioMemoryUsageMax());
      Serial.println(")");
      AudioProcessorUsageMaxReset();
      AudioMemoryUsageMaxReset();
      Serial.print(analogRead(VoltCheck)/54.95);
      Serial.println(" Volt");
      Volt = analogRead(VoltCheck)/54.95;
      if (Volt < 11.0) {
      clearname();
      tft.print ("Low voltage! ");
      tft.print(Volt);
      tft.print(" Volts");
      }
    
    }
#endif
} // end void loop ()


// #############################################

void FFT_WINDOW_SET (int FFT) {
  
  switch (FFT) {
        case 0: myFFT.windowFunction(AudioWindowHanning256);
                break; 
        case 1: myFFT.windowFunction(AudioWindowHamming256);
                
                break;
        case 2: myFFT.windowFunction(AudioWindowBartlett256);
               
                break;
        case 3: myFFT.windowFunction(AudioWindowBlackman256);
               
                break;
        case 4: myFFT.windowFunction(AudioWindowNuttall256);
                
                break;
        case 5: myFFT.windowFunction(AudioWindowBlackmanHarris256);
              
                break;
        case 6: myFFT.windowFunction(AudioWindowBlackmanNuttall256);
               
                break;
        case 7: myFFT.windowFunction(AudioWindowFlattop256);
               
                break;
        case 8: myFFT.windowFunction(AudioWindowWelch256);
               
                break;
        case 9: myFFT.windowFunction(AudioWindowCosine256);
               
                break;
        case 10:myFFT.windowFunction(AudioWindowTukey256);
               
                break;
         default: break;       
      } //end switch
} // end FFT WINDOW SET

String FFT_WINDOW_STRING (int FFT) {
  switch (FFT) {
        case 0: return "Hann"; break; 
        case 1: return "Hamming";
                break;
        case 2: return "Bartlett";
                break;
        case 3: return "Blackman";
                break;
        case 4: return "Nuttal";
                break;
        case 5: return "BlackHarri";
                break;
        case 6: return "BlackNutt";
                break;
        case 7: return "BlackNutt";
                break;
        case 8: return "Welch";
                break;
        case 9: return "Cosine";
                break;
        case 10: return "Tukey";
                break;
      
      } //end switch
    } // end FFT WINDOW SET

void EEPROMSAVE() {

   struct config_t {
 long calibration_factor;
  long calibration_constant;
 long freq[NUM_BANDS];
 int mode[NUM_BANDS];
 int bwu[NUM_BANDS];
 int bwl[NUM_BANDS];
 int rfg[NUM_BANDS];
 int band;
 float I_ampl;
 float Q_in_I;
 float I_in_Q;
 int Window_FFT;
 float LPFcoeff;
 } E; 

E.calibration_factor = calibration_factor;
E.band = band;
E.calibration_constant = calibration_constant;
for (int i=0; i< (NUM_BANDS); i++) 
E.freq[i] = bands[i].freq;
for (int i=0; i< (NUM_BANDS); i++)
E.mode[i] = bands[i].mode;
for (int i=0; i< (NUM_BANDS); i++)
E.bwu[i] = bands[i].bandwidthU;
for (int i=0; i< (NUM_BANDS); i++)
E.bwl[i] = bands[i].bandwidthL;
for (int i=0; i< (NUM_BANDS); i++)
E.rfg[i] = bands[i].RFgain;
E.I_ampl = I_help;
E.Q_in_I = Q_in_I_help;
E.I_in_Q = I_in_Q_help;
E.Window_FFT = Window_FFT;
E.LPFcoeff = LPFcoeff;
eeprom_write_block (&E,0,sizeof(E));


} // end void eeProm SAVE 

void EEPROMLOAD() {

   struct config_t {
 long calibration_factor;
  long calibration_constant;
 long freq[NUM_BANDS];
 int mode[NUM_BANDS];
 int bwu[NUM_BANDS];
 int bwl[NUM_BANDS];
 int rfg[NUM_BANDS];
 int band;
 float I_ampl;
 float Q_in_I;
 float I_in_Q;
  int Window_FFT;
  float LPFcoeff;
 } E; 

eeprom_read_block(&E,0,sizeof(E));
calibration_factor = E.calibration_factor;
calibration_constant = E.calibration_constant;
for (int i=0; i< (NUM_BANDS); i++) 
bands[i].freq = E.freq[i];
for (int i=0; i< (NUM_BANDS); i++)
bands[i].mode = E.mode[i];
for (int i=0; i< (NUM_BANDS); i++)
bands[i].bandwidthU = E.bwu[i];
for (int i=0; i< (NUM_BANDS); i++)
bands[i].bandwidthL = E.bwl[i];
for (int i=0; i< (NUM_BANDS); i++)
bands[i].RFgain = E.rfg[i];
band = E.band;
I_help = E.I_ampl;
Q_in_I_help = E.Q_in_I;
I_in_Q_help = E.I_in_Q;
Window_FFT = E.Window_FFT;
 LPFcoeff = E.LPFcoeff;

} // end void eeProm LOAD 

/* #########################################################################
 *  
 * void setup_gain
 *  
 *  
 * ######################################################################### 
 */

void setup_gain() {
  AudioNoInterrupts();
  // set gain values for audio_in_I and audio_in_Q
  audio_in_I.gain(0, I_help); // take values from EEPROM
  audio_in_I.gain(1, Q_in_I_help);  // take values from EEPROM
  audio_in_I.gain(2,0); //not used
  audio_in_I.gain(3,0); //not used
  audio_in_Q.gain(0, 1); // always 1, only I gain is calibrated
  audio_in_Q.gain(1, I_in_Q_help); // take values from EEPROM
  audio_in_Q.gain(2,0); //not used
  audio_in_Q.gain(3,0); //not used

/*
  add_substract_I.gain(0, 1);
  add_substract_Q.gain(0,1);

//  add_substract_I.gain(1,+1); // Rx below IF = add 
  add_substract_I.gain(1,-1); // Rx above IF = substract 
                                                             //  add_substract_Q.gain(0,-1); // add // falsch!!!!
//  add_substract_Q.gain(1,-1); // Rx below IF = substract 
  add_substract_Q.gain(1, +1); // Rx above IF = add 
  
   
  // set IF oscillator for RX
 cosIF.begin(1,IF_FREQ, TONE_TYPE_SINE);
 cosIF.phase(90.0);
// cosIF.phase(-90.0);
 sinIF.begin(1,IF_FREQ, TONE_TYPE_SINE);
 sinIF.phase(0);
// sinIF.phase(180); // 
 */

AudioInterrupts();
} // end void setup_gain();


/* #########################################################################
 *  
 *  void setup_mode
 *  
 *  set up radio for RX modes - USB, LSB
 * ######################################################################### 
 */

void setup_mode(int MO) {
  switch (MO)  {

    case modeLSB:
        if (bands[band].bandwidthU != 0) bands[band].bandwidthL = bands[band].bandwidthU;
        bands[band].bandwidthU = 0;
        USB.gain(0, 0);
        USB.gain(1, 0); // 
        USB.gain(2, 0); // AM off
        LSB.gain(0, 1);
        LSB.gain(1, -1); // substract I & Q
        LSB.gain(2, 0); // AM off
        mixright.gain(0, 0); // USB off
        mixright.gain(1, 1); // LSB on
        mixright.gain(3, 0); // SD card off
        mixleft.gain(0, 0); // USB off
        mixleft.gain(1, 1); // LSB on
        mixleft.gain(3, 0); // SD card off
    break;
    case modeUSB:
        if (bands[band].bandwidthL != 0) bands[band].bandwidthU = bands[band].bandwidthL;
        bands[band].bandwidthL = 0;
        USB.gain(0, 1);
        USB.gain(1, 1); // add I+Q
        USB.gain(2, 0); // AM off
        LSB.gain(0, 0);
        LSB.gain(1, 0);
        LSB.gain(2, 0);
        mixright.gain(0, 1); // USB on
        mixright.gain(1, 0); // LSB off
        mixright.gain(3, 0); // SD card off
        mixleft.gain(0, 1); // USB on
        mixleft.gain(1, 0); // LSB off
        mixleft.gain(3, 0); // SD card off
    break;
    case modeAM:
        if (bands[band].bandwidthU >= bands[band].bandwidthL) 
            bands[band].bandwidthL = bands[band].bandwidthU;
            else bands[band].bandwidthU = bands[band].bandwidthL;
        USB.gain(0, 0);
        USB.gain(1, 0); // off
        USB.gain(2, 1); // AM !
        LSB.gain(0, 0);
        LSB.gain(1, 0);
        LSB.gain(2, 0); // we only need one channel for mono AM
        mixright.gain(0, 1); // AM on
        mixright.gain(1, 0); // LSB off
        mixright.gain(3, 0); // SD card off
        mixleft.gain(0, 1); // AM on
        mixleft.gain(1, 0); // LSB off
        mixleft.gain(3, 0); // SD card off
    break;
    case modeDSB:
        if (bands[band].bandwidthU >= bands[band].bandwidthL) 
            bands[band].bandwidthL = bands[band].bandwidthU;
            else bands[band].bandwidthU = bands[band].bandwidthL;
          
        //if (bands[band].bandwidthU == 0)     
        USB.gain(0, 1);
        USB.gain(1, 1); // I + Q = USB
        USB.gain(2, 0); // AM off
        LSB.gain(0, 1);
        LSB.gain(1, -1); // I - Q = LSB
        LSB.gain(2, 0); // AM off
        mixright.gain(0, 0.5); // mono mix of USB
        mixright.gain(1, 0.5); // and LSB
        mixright.gain(3, 0); // SD card off
        mixleft.gain(0, 0.5); // mono mix of USB
        mixleft.gain(1, 0.5); // and LSB
        mixleft.gain(3, 0); // SD card off
    break;
    case modeStereoAM:
        if (bands[band].bandwidthU >= bands[band].bandwidthL) 
            bands[band].bandwidthL = bands[band].bandwidthU;
            else bands[band].bandwidthU = bands[band].bandwidthL;
        USB.gain(0, 1);
        USB.gain(1, 1); // I + Q = USB
        USB.gain(2, 0); // AM off
        LSB.gain(0, 1);
        LSB.gain(1, -1); // I - Q = LSB
        LSB.gain(2, 0); // AM off
        mixright.gain(0, 1); // right channel =  USB
        mixright.gain(1, 0); // 
        mixright.gain(3, 0); // SD card off
        mixleft.gain(0, 0); // 
        mixleft.gain(1, 1); // left channel = LSB
        mixleft.gain(3, 0); // SD card off
    break;
  }
} // end void setup_mode


/* #########################################################################
 *  
 *  void setup_RX
 *  
 *  set up filters: bandwidths etc
 * ######################################################################### 
 */


// void setup_RX(int MO, int BW)
 void setup_RX(int MO, int bwu, int bwl)
{
  show_notch(notchF, MO);
  FreqConv.direction(1); // receive freq is higher than zeroband
  FreqConv.passthrough(freqconv_sw); // do not passthrough audio, but convert by IF
   

//    audioShield.volume(0);
//    audioShield.muteHeadphone();
      delay(10);
      AudioNoInterrupts();   // Disable Audio while reconfiguring filters
      int BW = bwl;
  if (MO == modeAM) {
    AM.passthrough(AM_pass); // switches on the AM demodulator (AM_pass = bool (1))
        // hier if-Abfragen für AM
            if (BW < 1301) {
                FIR_I.begin(fir18,BPF_COEFFS);
                FIR_Q.begin(fir18,BPF_COEFFS);
                biquad1.setCoefficients(0, IIR_26_Coeffs_0);
                biquad1.setCoefficients(1, IIR_26_Coeffs_1);
                biquad1.setCoefficients(2, IIR_26_Coeffs_2);
                biquad1.setCoefficients(3, IIR_26_Coeffs_3);
                biquad2.setCoefficients(0, IIR_26_Coeffs_0);
                biquad2.setCoefficients(1, IIR_26_Coeffs_1);
                biquad2.setCoefficients(2, IIR_26_Coeffs_2);
                biquad2.setCoefficients(3, IIR_26_Coeffs_3);
            } else
            if (BW < 1801) {
                FIR_I.begin(fir18,BPF_COEFFS);
                FIR_Q.begin(fir18,BPF_COEFFS);
                biquad1.setCoefficients(0, IIR_31_Coeffs_0);
                biquad1.setCoefficients(1, IIR_31_Coeffs_1);
                biquad1.setCoefficients(2, IIR_31_Coeffs_2);
                biquad1.setCoefficients(3, IIR_31_Coeffs_3);
                biquad2.setCoefficients(0, IIR_31_Coeffs_0);
                biquad2.setCoefficients(1, IIR_31_Coeffs_1);
                biquad2.setCoefficients(2, IIR_31_Coeffs_2);
                biquad2.setCoefficients(3, IIR_31_Coeffs_3);
            } else
            if (BW < 2301) {
                FIR_I.begin(fir26,BPF_COEFFS);
                FIR_Q.begin(fir26,BPF_COEFFS);
                biquad1.setCoefficients(0, IIR_44_Coeffs_0);
                biquad1.setCoefficients(1, IIR_44_Coeffs_1);
                biquad1.setCoefficients(2, IIR_44_Coeffs_2);
                biquad1.setCoefficients(3, IIR_44_Coeffs_3);
                biquad2.setCoefficients(0, IIR_44_Coeffs_0);
                biquad2.setCoefficients(1, IIR_44_Coeffs_1);
                biquad2.setCoefficients(2, IIR_44_Coeffs_2);
                biquad2.setCoefficients(3, IIR_44_Coeffs_3);
            } else
            if (BW < 2601) {
                FIR_I.begin(fir26,BPF_COEFFS);
                FIR_Q.begin(fir26,BPF_COEFFS);
                biquad1.setCoefficients(0, IIR_54_Coeffs_0);
                biquad1.setCoefficients(1, IIR_54_Coeffs_1);
                biquad1.setCoefficients(2, IIR_54_Coeffs_2);
                biquad1.setCoefficients(3, IIR_54_Coeffs_3);
                biquad2.setCoefficients(0, IIR_54_Coeffs_0);
                biquad2.setCoefficients(1, IIR_54_Coeffs_1);
                biquad2.setCoefficients(2, IIR_54_Coeffs_2);
                biquad2.setCoefficients(3, IIR_54_Coeffs_3);
            }
            else        
            if (BW < 3601) {
                FIR_I.begin(fir36,BPF_COEFFS);
                FIR_Q.begin(fir36,BPF_COEFFS);
                biquad1.setCoefficients(0, IIR_65_Coeffs_0);
                biquad1.setCoefficients(1, IIR_65_Coeffs_1);
                biquad1.setCoefficients(2, IIR_65_Coeffs_2);
                biquad1.setCoefficients(3, IIR_65_Coeffs_3);
                biquad2.setCoefficients(0, IIR_65_Coeffs_0);
                biquad2.setCoefficients(1, IIR_65_Coeffs_1);
                biquad2.setCoefficients(2, IIR_65_Coeffs_2);
                biquad2.setCoefficients(3, IIR_65_Coeffs_3);
            }
            else
            if (BW < 4401) {
                FIR_I.begin(fir44,BPF_COEFFS);
                FIR_Q.begin(fir44,BPF_COEFFS);
                biquad1.setCoefficients(0, IIR_80_Coeffs_0);
                biquad1.setCoefficients(1, IIR_80_Coeffs_1);
                biquad1.setCoefficients(2, IIR_80_Coeffs_2);
                biquad1.setCoefficients(3, IIR_80_Coeffs_3);
                biquad2.setCoefficients(0, IIR_80_Coeffs_0);
                biquad2.setCoefficients(1, IIR_80_Coeffs_1);
                biquad2.setCoefficients(2, IIR_80_Coeffs_2);
                biquad2.setCoefficients(3, IIR_80_Coeffs_3);
            }
            else
            if (BW < 6500) {
                FIR_I.begin(fir65,BPF_COEFFS);
                FIR_Q.begin(fir65,BPF_COEFFS);
                biquad1.setCoefficients(0, IIR_110_Coeffs_0);
                biquad1.setCoefficients(1, IIR_110_Coeffs_1);
                biquad1.setCoefficients(2, IIR_110_Coeffs_2);
                biquad1.setCoefficients(3, IIR_110_Coeffs_3);
                biquad2.setCoefficients(0, IIR_110_Coeffs_0);
                biquad2.setCoefficients(1, IIR_110_Coeffs_1);
                biquad2.setCoefficients(2, IIR_110_Coeffs_2);
                biquad2.setCoefficients(3, IIR_110_Coeffs_3);
            }
            else {
                FIR_I.begin(fir80,BPF_COEFFS);
                FIR_Q.begin(fir80,BPF_COEFFS);
/*                biquad1.setCoefficients(0, IIR_80_Coeffs_0);
                biquad1.setCoefficients(1, IIR_80_Coeffs_1);
                biquad1.setCoefficients(2, IIR_80_Coeffs_2);
                biquad1.setCoefficients(3, IIR_80_Coeffs_3);
                biquad2.setCoefficients(0, IIR_80_Coeffs_0);
                biquad2.setCoefficients(1, IIR_80_Coeffs_1);
                biquad2.setCoefficients(2, IIR_80_Coeffs_2);
                biquad2.setCoefficients(3, IIR_80_Coeffs_3);
*/
                biquad1.setCoefficients(0, IIR_110_Coeffs_0);
                biquad1.setCoefficients(1, IIR_110_Coeffs_1);
                biquad1.setCoefficients(2, IIR_110_Coeffs_2);
                biquad1.setCoefficients(3, IIR_110_Coeffs_3);
                biquad2.setCoefficients(0, IIR_110_Coeffs_0);
                biquad2.setCoefficients(1, IIR_110_Coeffs_1);
                biquad2.setCoefficients(2, IIR_110_Coeffs_2);
                biquad2.setCoefficients(3, IIR_110_Coeffs_3);

            }

  }
  else { //hier if-Abfragen für LSB/USB/DSB/StereoAM/SAM
    
          AM.passthrough(0); // switches off AM demodulator for the single sideband demodulation modes
    
          // set Hilbert filters to the highest of bwl & bwu
          if (bwu >= bwl) BW = bwu;
            else BW = bwl;    
            if (BW < 1801) {
                FIR_I.begin(H_m45_1_8kHz,HILBERT_COEFFS);
                FIR_Q.begin(H_45_1_8kHz,HILBERT_COEFFS);
            } else
            if (BW < 2601) {
                FIR_I.begin(H_m45_2_6kHz,HILBERT_COEFFS);
                FIR_Q.begin(H_45_2_6kHz,HILBERT_COEFFS);
            } else
            if (BW < 3601) {
                FIR_I.begin(H_m45_3_6kHz,HILBERT_COEFFS);
                FIR_Q.begin(H_45_3_6kHz,HILBERT_COEFFS);
            } else
            if (BW < 4401) {
                FIR_I.begin(H_m45_4_4kHz,HILBERT_COEFFS);
                FIR_Q.begin(H_45_4_4kHz,HILBERT_COEFFS);
            } else
            if (BW < 6501) {
                FIR_I.begin(H_m45_6_5kHz,HILBERT_COEFFS);
                FIR_Q.begin(H_45_6_5kHz,HILBERT_COEFFS);
            } else
            if (BW < 8001) {
                FIR_I.begin(H_m45_8kHz,HILBERT_COEFFS);
                FIR_Q.begin(H_45_8kHz,HILBERT_COEFFS);
            } else
             {
                FIR_I.begin(H_m45_11kHz,HILBERT_COEFFS);
                FIR_Q.begin(H_45_11kHz,HILBERT_COEFFS);
             }
      // now set separately lower and upper sideband path (for StereoAM & DSB)
      // for USB and LSB only one filter is necessary      
      // first: lower sideband path with biquad2
            if (bwl < 1801) {
                biquad2.setCoefficients(0, IIR_18_Coeffs_0);
                biquad2.setCoefficients(1, IIR_18_Coeffs_1);
                biquad2.setCoefficients(2, IIR_18_Coeffs_2);
                biquad2.setCoefficients(3, IIR_18_Coeffs_3);
            } else
            if (bwl < 2301) {
                biquad2.setCoefficients(0, IIR_23_Coeffs_0);
                biquad2.setCoefficients(1, IIR_23_Coeffs_1);
                biquad2.setCoefficients(2, IIR_23_Coeffs_2);
                biquad2.setCoefficients(3, IIR_23_Coeffs_3);
            } else
            if (bwl < 2601) {
                biquad2.setCoefficients(0, IIR_26_Coeffs_0);
                biquad2.setCoefficients(1, IIR_26_Coeffs_1);
                biquad2.setCoefficients(2, IIR_26_Coeffs_2);
                biquad2.setCoefficients(3, IIR_26_Coeffs_3);
            } else
            if (bwl < 3101) {
                biquad2.setCoefficients(0, IIR_31_Coeffs_0);
                biquad2.setCoefficients(1, IIR_31_Coeffs_1);
                biquad2.setCoefficients(2, IIR_31_Coeffs_2);
                biquad2.setCoefficients(3, IIR_31_Coeffs_3);
            } else
            if (bwl < 3601) {
                biquad2.setCoefficients(0, IIR_36_Coeffs_0);
                biquad2.setCoefficients(1, IIR_36_Coeffs_1);
                biquad2.setCoefficients(2, IIR_36_Coeffs_2);
                biquad2.setCoefficients(3, IIR_36_Coeffs_3);
            } else
            if (bwl < 4001) {
                biquad2.setCoefficients(0, IIR_40_Coeffs_0);
                biquad2.setCoefficients(1, IIR_40_Coeffs_1);
                biquad2.setCoefficients(2, IIR_40_Coeffs_2);
                biquad2.setCoefficients(3, IIR_40_Coeffs_3);
            } else
            if (bwl < 4401) {
                biquad2.setCoefficients(0, IIR_44_Coeffs_0);
                biquad2.setCoefficients(1, IIR_44_Coeffs_1);
                biquad2.setCoefficients(2, IIR_44_Coeffs_2);
                biquad2.setCoefficients(3, IIR_44_Coeffs_3);
            } else
            if (bwl < 5401) {
                biquad2.setCoefficients(0, IIR_54_Coeffs_0);
                biquad2.setCoefficients(1, IIR_54_Coeffs_1);
                biquad2.setCoefficients(2, IIR_54_Coeffs_2);
                biquad2.setCoefficients(3, IIR_54_Coeffs_3);
            } else
            if (bwl < 6500) {
                biquad2.setCoefficients(0, IIR_65_Coeffs_0);
                biquad2.setCoefficients(1, IIR_65_Coeffs_1);
                biquad2.setCoefficients(2, IIR_65_Coeffs_2);
                biquad2.setCoefficients(3, IIR_65_Coeffs_3);
            } else
            if (bwl < 8000) {
                biquad2.setCoefficients(0, IIR_80_Coeffs_0);
                biquad2.setCoefficients(1, IIR_80_Coeffs_1);
                biquad2.setCoefficients(2, IIR_80_Coeffs_2);
                biquad2.setCoefficients(3, IIR_80_Coeffs_3);
            } else
             {
                biquad2.setCoefficients(0, IIR_110_Coeffs_0);
                biquad2.setCoefficients(1, IIR_110_Coeffs_1);
                biquad2.setCoefficients(2, IIR_110_Coeffs_2);
                biquad2.setCoefficients(3, IIR_110_Coeffs_3);              }
      // second: upper sideband path with biquad1
            if (bwu < 1801) {
                biquad1.setCoefficients(0, IIR_18_Coeffs_0);
                biquad1.setCoefficients(1, IIR_18_Coeffs_1);
                biquad1.setCoefficients(2, IIR_18_Coeffs_2);
                biquad1.setCoefficients(3, IIR_18_Coeffs_3);
            } else
            if (bwu < 2301) {
                biquad1.setCoefficients(0, IIR_23_Coeffs_0);
                biquad1.setCoefficients(1, IIR_23_Coeffs_1);
                biquad1.setCoefficients(2, IIR_23_Coeffs_2);
                biquad1.setCoefficients(3, IIR_23_Coeffs_3);
            } else
            if (bwu < 2601) {
                biquad1.setCoefficients(0, IIR_26_Coeffs_0);
                biquad1.setCoefficients(1, IIR_26_Coeffs_1);
                biquad1.setCoefficients(2, IIR_26_Coeffs_2);
                biquad1.setCoefficients(3, IIR_26_Coeffs_3);
            } else
            if (bwu < 3101) {
                biquad1.setCoefficients(0, IIR_31_Coeffs_0);
                biquad1.setCoefficients(1, IIR_31_Coeffs_1);
                biquad1.setCoefficients(2, IIR_31_Coeffs_2);
                biquad1.setCoefficients(3, IIR_31_Coeffs_3);
            } else
            if (bwu < 3601) {
                biquad1.setCoefficients(0, IIR_36_Coeffs_0);
                biquad1.setCoefficients(1, IIR_36_Coeffs_1);
                biquad1.setCoefficients(2, IIR_36_Coeffs_2);
                biquad1.setCoefficients(3, IIR_36_Coeffs_3);
            } else
            if (bwu < 4001) {
                biquad1.setCoefficients(0, IIR_40_Coeffs_0);
                biquad1.setCoefficients(1, IIR_40_Coeffs_1);
                biquad1.setCoefficients(2, IIR_40_Coeffs_2);
                biquad1.setCoefficients(3, IIR_40_Coeffs_3);
            } else
            if (bwu < 4401) {
                biquad1.setCoefficients(0, IIR_44_Coeffs_0);
                biquad1.setCoefficients(1, IIR_44_Coeffs_1);
                biquad1.setCoefficients(2, IIR_44_Coeffs_2);
                biquad1.setCoefficients(3, IIR_44_Coeffs_3);
            } else
            if (bwu < 5401) {
                biquad1.setCoefficients(0, IIR_54_Coeffs_0);
                biquad1.setCoefficients(1, IIR_54_Coeffs_1);
                biquad1.setCoefficients(2, IIR_54_Coeffs_2);
                biquad1.setCoefficients(3, IIR_54_Coeffs_3);
            } else
            if (bwu < 6500) {
                biquad1.setCoefficients(0, IIR_65_Coeffs_0);
                biquad1.setCoefficients(1, IIR_65_Coeffs_1);
                biquad1.setCoefficients(2, IIR_65_Coeffs_2);
                biquad1.setCoefficients(3, IIR_65_Coeffs_3);
            } else
            if (bwu < 8000) {
                biquad1.setCoefficients(0, IIR_80_Coeffs_0);
                biquad1.setCoefficients(1, IIR_80_Coeffs_1);
                biquad1.setCoefficients(2, IIR_80_Coeffs_2);
                biquad1.setCoefficients(3, IIR_80_Coeffs_3);
            } else
             {
                biquad1.setCoefficients(0, IIR_110_Coeffs_0); 
                biquad1.setCoefficients(1, IIR_110_Coeffs_1);
                biquad1.setCoefficients(2, IIR_110_Coeffs_2);
                biquad1.setCoefficients(3, IIR_110_Coeffs_3);              }
                
        } // end else (mode = AM)

   show_bandwidth(MO, bwu, bwl); // draw bandwidth indicator under spectrum display

/*        biquad1.setCoefficients(0, IIR_44_Coeffs_0);
        biquad1.setCoefficients(1, IIR_44_Coeffs_1);
        biquad1.setCoefficients(2, IIR_44_Coeffs_2);
        biquad1.setCoefficients(3, IIR_44_Coeffs_3);
//        biquad1.setCoefficients(4, IIR_44_Coeffs_4);
//        biquad1.setCoefficients(5, IIR_44_Coeffs_5); 
  */ 

  
    if (bwu < 400) bwu = 400;
    if (bwl < 400) bwl = 400;
    if (MO == modeAM) {
      float fac = 2.0;
      biquad3.setLowpass(0, bwu*fac, 0.54);
      biquad3.setLowpass(1, bwu*fac, 1.3);
      biquad3.setLowpass(2, bwu*fac, 0.54);
      biquad3.setLowpass(3, bwu*fac, 1.3);
  // LSB path
      biquad4.setLowpass(0, bwl*fac, 0.54);
      biquad4.setLowpass(1, bwl*fac, 1.3);
      biquad4.setLowpass(2, bwl*fac, 0.54);
      biquad4.setLowpass(3, bwl*fac, 1.3);
   }
   // Linkwitz-Riley filter, 48 dB/octave
   // USB path
     else {
      biquad3.setLowpass(0, bwu, 0.54);
      biquad3.setLowpass(1, bwu, 1.3);
      biquad3.setLowpass(2, bwu, 0.54);
      biquad3.setLowpass(3, bwu, 1.3);
  // LSB path
      biquad4.setLowpass(0, bwl, 0.54);
      biquad4.setLowpass(1, bwl, 1.3);
      biquad4.setLowpass(2, bwl, 0.54);
      biquad4.setLowpass(3, bwl, 1.3);
     }
     
// notchfilter switching is implemented only HERE
      if (notchF >= 400 || notchF <=-400) {
      switch (MO) {
          case modeUSB:
              if (notchF >= 400) { 
              biquad3.setNotch(0, notchF, notchQ);
              biquad3.setNotch(1, notchF, notchQ);
              biquad3.setNotch(2, notchF, notchQ);
              biquad3.setNotch(3, notchF, notchQ);
              } else
              {
              biquad3.setNotch(0, notchF * -1, notchQ);
              biquad3.setNotch(1, notchF * -1, notchQ);
              biquad3.setNotch(2, notchF * -1, notchQ);
              biquad3.setNotch(3, notchF * -1, notchQ);
              }
          break;
          case modeLSB:
              if (notchF <= -400) { 
              biquad4.setNotch(0, notchF * -1, notchQ);
              biquad4.setNotch(1, notchF * -1, notchQ);
              biquad4.setNotch(2, notchF * -1, notchQ);
              biquad4.setNotch(3, notchF * -1, notchQ);
              }
              else {
              biquad4.setNotch(0, notchF, notchQ);
              biquad4.setNotch(1, notchF, notchQ);
              biquad4.setNotch(2, notchF, notchQ);
              biquad4.setNotch(3, notchF, notchQ);
              }
          break;
          case modeAM:
                if (notchF <= -400) {
                     biquad3.setNotch(0, notchF * -1, notchQ);
                     biquad3.setNotch(1, notchF * -1, notchQ);
                     biquad3.setNotch(2, notchF * -1, notchQ);
                     biquad3.setNotch(3, notchF * -1, notchQ);
                }
                else {
                     biquad3.setNotch(0, notchF, notchQ);
                     biquad3.setNotch(1, notchF, notchQ);
                     biquad3.setNotch(2, notchF, notchQ);
                     biquad3.setNotch(3, notchF, notchQ);
                }
          break;
          case modeDSB:
          case modeStereoAM:
                if (notchF <= -400) {
                      biquad4.setNotch(0, notchF * -1, notchQ); // LSB path
                      biquad4.setNotch(1, notchF * -1, notchQ); // LSB path
                      biquad4.setNotch(2, notchF * -1, notchQ); // LSB path
                      biquad4.setNotch(3, notchF * -1, notchQ); // LSB path
                      biquad3.setLowpass(0, bwu, 0.54);
                      biquad3.setLowpass(1, bwu, 1.3);
                      biquad3.setLowpass(2, bwu, 0.54);
                      biquad3.setLowpass(3, bwu, 1.3);
                }
                else if (notchF >= 400){     
                      biquad3.setNotch(0, notchF, notchQ);
                      biquad3.setNotch(1, notchF, notchQ);
                      biquad3.setNotch(2, notchF, notchQ);
                      biquad3.setNotch(3, notchF, notchQ);
                      biquad4.setLowpass(0, bwl, 0.54);
                      biquad4.setLowpass(1, bwl, 1.3);
                      biquad4.setLowpass(2, bwl, 0.54);
                      biquad4.setLowpass(3, bwl, 1.3);
                }
          break;
      }
      }


 AudioInterrupts();
//     audioShield.unmuteHeadphone(); 
//    audioShield.volume(vol);
    }

void displayClock() {

      uint8_t hour10 = hour ()/10%10;
      uint8_t hour1 = hour()%10;
      uint8_t minute10 = minute()/10%10;
      uint8_t minute1 = minute()%10;
      uint8_t second10 = second()/10%10;
      uint8_t second1 = second()%10;
      uint8_t time_pos_shift = 6;
      uint8_t dp = 5;

/*       Serial.print(hour10);
       Serial.print(hour1);
       Serial.print(":");
       Serial.print(minute10);
       Serial.print(minute1);
       Serial.print(":");
       Serial.print(second10);
       Serial.print(second1); */

// set up ":" for time display        
      if (!timeflag) {
      tft.setCursor(pos_x_time + 2 * time_pos_shift, pos_y_time);
      tft.print(":");
      tft.setCursor(pos_x_time + 4 * time_pos_shift + dp, pos_y_time);
      tft.print(":");
      tft.setCursor(pos_x_time + 7 * time_pos_shift + 2*dp, pos_y_time);
      tft.print("UTC");
      }

      if (hour10 != hour10_old || !timeflag) {
             tft.setCursor(pos_x_time, pos_y_time);
             tft.fillRect(pos_x_time, pos_y_time, time_pos_shift, 8, BLACK);             
             if (hour10) tft.print(hour10);  // do not display, if zero   
        }
      if (hour1 != hour1_old || !timeflag) {
             tft.setCursor(pos_x_time + time_pos_shift, pos_y_time);
             tft.fillRect(pos_x_time  + time_pos_shift, pos_y_time, time_pos_shift, 8, BLACK);             
             tft.print(hour1);  // always display   
        }
      if (minute1 != minute1_old || !timeflag) {
             tft.setCursor(pos_x_time + 3 * time_pos_shift + dp, pos_y_time);
             tft.fillRect(pos_x_time  + 3 * time_pos_shift + dp, pos_y_time, time_pos_shift, 8, BLACK);             
             tft.print(minute1);  // always display   
        }
      if (minute10 != minute10_old || !timeflag) {
             tft.setCursor(pos_x_time + 2 * time_pos_shift + dp, pos_y_time);
             tft.fillRect(pos_x_time  + 2 * time_pos_shift + dp, pos_y_time, time_pos_shift, 8, BLACK);             
             tft.print(minute10);  // always display   
        }
      if (second10 != second10_old || !timeflag) {
             tft.setCursor(pos_x_time + 4 * time_pos_shift + 2*dp, pos_y_time);
             tft.fillRect(pos_x_time  + 4 * time_pos_shift + 2*dp, pos_y_time, time_pos_shift, 8, BLACK);             
             tft.print(second10);  // always display   
        }
      if (second1 != second1_old || !timeflag) {
             tft.setCursor(pos_x_time + 5 * time_pos_shift + 2*dp, pos_y_time);
             tft.fillRect(pos_x_time  + 5 * time_pos_shift + 2*dp, pos_y_time, time_pos_shift, 8, BLACK);             
             tft.print(second1);  // always display   
        }

      hour1_old = hour1;
      hour10_old = hour10;
      minute1_old = minute1;
      minute10_old = minute10;
      second1_old = second1;
      second10_old = second10;
      timeflag = 1;

// old time display routine
/*  char string99 [14]; 
  tft.fillRect(pos_x_time, pos_y_time, 80, 8, BLACK); // erase old string
  tft.setTextColor(WHITE);
  tft.setCursor(pos_x_time, pos_y_time);
  sprintf(string99,"%02d:%02d:%02d UTC", hour(),minute(),second());
  tft.print(string99);
*/
} // end void displayTime

void displayDate() {
  char string99 [14]; 
  tft.fillRect(pos_x_date, pos_y_date, 160-pos_x_date, 8, BLACK); // erase old string
  tft.setTextColor(WHITE);
  tft.setCursor(pos_x_date, pos_y_date);
  sprintf(string99,"%02d.%02d.%04d", day(),month(),year());
  tft.print(string99);
}


/////////////////////////////////////////////////////////////////////
// Sine/cosine generation function
// Adjust rad_calc *= 32;  p.e. ( 44100 / 128 * 32 = 11025 khz)
/////////////////////////////////////////////////////////////////////

void sinewave()
{
  uint     i;
  float32_t rad_calc;
  char string[10];   // print format stuff

  if (!flag)  {  // have we already calculated the sine wave?
    for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {  // No, let's do it!
      rad_calc = (float32_t)i;    // convert to float the current position within the buffer
      rad_calc /= (AUDIO_BLOCK_SAMPLES);     // make this a fraction
      rad_calc *= (PI * 2);     // convert to radians
      rad_calc *= IF_FREQ * 128 / AUDIO_SAMPLE_RATE_EXACT;      // multiply by number of cycles that we want within this block ( 44100 / 128 * 32 = 11025 khz)
      //
      Osc_Q_buffer_i_f[i] = arm_cos_f32(rad_calc);  // get sine and cosine values and store in pre-calculated array
      Osc_I_buffer_i_f[i] = arm_sin_f32(rad_calc);
    }

    arm_float_to_q15(Osc_Q_buffer_i_f, Osc_Q_buffer_i, AUDIO_BLOCK_SAMPLES);
    arm_float_to_q15(Osc_I_buffer_i_f, Osc_I_buffer_i, AUDIO_BLOCK_SAMPLES);

#ifdef DEBUG

    for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {

      sprintf(string, "%d ; %d",  Osc_I_buffer_i[i], Osc_Q_buffer_i[i] );
      Serial.println(string);
    }

#endif

    flag = 1; // signal that once we have generated the quadrature sine waves, we shall not do it again
  }


}

/*##############################
// experimental Schedule
//##############################

short Schedule[Schedule_length] = {
#include "Schedule" 
};

   struct config_t {
 long f[Schedule_length];
  char country[Schedule_length];
 char language[Schedule_length];
 char name[Schedule_length];
 int timestart[Schedule_length];
 int timeend[Schedule_length]
 } sched; 

for (i=0, i<Schedule_length, i++) {
  sched[i].f = Schedule[i*6+1]*1000; // frequency in Hz
  sched[i].country = Schedule[i*6+2];
  sched[i].language = Schedule[i*6+3];
  sched[i].name = Schedule[i*6+4];
  sched[i].timestart = Schedule[i*6+5];
  sched[i].timeend = Schedule[i*6+6];
  } // endif
// Jetzt haben wir auf alle Variablen aus Schedule.h Zugriff

for (i=0, i<Schedule_length, i++) {
  
  if (bands[band].freq+IF_FREQ < (sched[i].f+500) | bands[band].freq+IF_FREQ > (sched[i].f-500)) { //frequency is within 1kHz of f in schedule
      // ask for time
      if (hour()+1 > sched[i].timestart | hour() < sched[i].timeend+1) { // actual time is within broadcast time 
            // fill an array with all possible stations on that frequency at that time
      

*/
/*
void MP3_Smeter(void){
    int pos_x_smeter = 5;
    int  pos_y_smeter = 94;
    int  s_w = 10;
    float redlength = 0;
    if (MP3_peak.available()) {
    float s = MP3_peak.read();
      s = s * 80.0;
//      if (s <0.0) s=0.0;
//      if ( s > 255.0) s = 255.0;
      
      if ((s*s_w+1) > 9.0*s_w+1) {
        redlength = (s*s_w+1) - (9.0*s_w+1);
        s = 9.0;
      }
      if (redlength > 50) redlength = 50; 
            
      tft.drawFastHLine(pos_x_smeter, pos_y_smeter, s*s_w+1, BLUE);
      tft.drawFastHLine(pos_x_smeter+s*s_w+1, pos_y_smeter, (9*s_w+1)-s*s_w+1, BLACK);

      tft.drawFastHLine(pos_x_smeter, pos_y_smeter+1, s*s_w+1, WHITE);
      tft.drawFastHLine(pos_x_smeter+s*s_w+1, pos_y_smeter+1, (9*s_w+1)-s*s_w+1, BLACK);
      tft.drawFastHLine(pos_x_smeter, pos_y_smeter+2, s*s_w+1, BLUE);
      tft.drawFastHLine(pos_x_smeter+s*s_w+1, pos_y_smeter+2, (9*s_w+1)-s*s_w+1, BLACK);

   //   tft.drawFastHLine(pos_x_smeter, pos_y_smeter+3, s*s_w+1, BLUE);
   //   tft.drawFastHLine(pos_x_smeter+s*s_w+1, pos_y_smeter+3, (9*s_w+1)-s*s_w+1, BLACK);

      tft.drawFastHLine(pos_x_smeter+9*s_w+1, pos_y_smeter,  redlength, RED);
      tft.drawFastHLine(pos_x_smeter+9*s_w+ redlength+1, pos_y_smeter, 30 - redlength, BLACK);
      tft.drawFastHLine(pos_x_smeter+9*s_w+1, pos_y_smeter+1, redlength, RED);
      tft.drawFastHLine(pos_x_smeter+9*s_w+ redlength+1, pos_y_smeter+1,  30 - redlength, BLACK);
      tft.drawFastHLine(pos_x_smeter+9*s_w+1, pos_y_smeter+2, redlength, RED);
      tft.drawFastHLine(pos_x_smeter+9*s_w+  redlength+1, pos_y_smeter+2,  30 - redlength, BLACK);
      }
      
} // end void MP3_Smeter
*/
/* void MP3_spectrum() {

  static int startx=0, endx;
  endx=startx+16;
  if (MP3_FFT.available()) {
    int scale;
    scale = 100;

  for (int16_t x=startx; x < endx; x+=2) {
     int bar=abs(MP3_FFT.output[x*10/32])/scale;
     if (bar >110) bar=110;
     if (bar > peak[x]) peak[x]=bar;
     tft.drawFastVLine(x, 110-bar,bar, ST7735_MAGENTA);
     tft.drawFastVLine(x, 0, 110-bar, ST7735_BLACK);    
     tft.drawPixel(x,110-peak[x], ST7735_YELLOW);
     if(peak[x]>0) peak[x]-=5;
  }
      startx+=16;
  if(startx >=160) startx=0;
  } //end if
   } // end void spectrum
*/

