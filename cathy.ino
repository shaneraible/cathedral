#include "Arduino.h"
#include <FastLED.h>

typedef void (*State) ();

#define DATA_PIN 3
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 10
#define BUTTON_PIN 2

CRGB leds[NUM_LEDS];

//interrupt values
volatile unsigned int state;
volatile const unsigned int num_states = 7;
volatile unsigned long t=0;

//function prototypes
void makeBusy();
void explodeLED(CRGB col);
void setRange(int x1, int x2);
void explodeLED(CRGB col);
void pittLighting();

//states
void turnOn();
void offState();
void pittColors1();
void rainbowState();
void twinkleState();
void victoryState();
void warmLightState();
void rotateColors();
State states[] = { offState, pittColors1, rainbowState, twinkleState, victoryState, warmLightState, rotateColors };


static void nextState() {
  //debounce
  if(millis()-t>200){
    state = (state+1)%num_states;
    Serial.println(state);
    t = millis();
  }
}

void setup() {
  state = 0;
  Serial.begin(9600);
  delay(500);

  //configure the leds
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.clear();
  FastLED.show();

  //configure the interrupt
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), nextState, RISING);
}

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop() {
  states[state]();
  FastLED.show();  
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_MILLISECONDS( 6000 ) { nextState(); }
}

void setRange(int x1, int x2, CRGB col){
  for(int i = x1; i<=x2; i++){
    leds[i] = col;
  }
}

void fadeRange(int srt, int nd, CRGB col, int t){
  CRGB leds_start[NUM_LEDS];
  for(int i=0; i<NUM_LEDS; i++) leds_start[i] = leds[i];
  
  for(int i=0; i<t; i++){
    for(int l=srt; l<=nd; l++){
      leds[l].r += ((double)col.r-leds_start[l].r)/(t);
      leds[l].g += ((double)col.g-leds_start[l].g)/(t);
      leds[l].b += ((double)col.b-leds_start[l].b)/(t);
    }
    FastLED.show();
    delay(1);
  }
}

void fadeColorUp(CRGB col, int del){
  fadeRange(0,3, col, del);
  fadeRange(4,5, col, del);
  fadeRange(6,7, col, del);
  fadeRange(8,8, col, del);
  fadeRange(9,9, col, del);
}

void offState(){
  FastLED.clear();
  FastLED.show();
  delay((state==0)*2000);
}

void pittColors1(){
  if(state==1) fadeColorUp(CRGB::Blue, 100);
  delay((state==1) * 1000);
  if(state==1) fadeColorUp(CRGB::Yellow, 100);
  delay((state==1) * 1000);
}

void rainbowState(){
  // good as is
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void twinkleState(){
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
  delay(40);
}

void victoryState(){
  FastLED.clear();
  fill_solid(leds, NUM_LEDS, CRGB::Orange);
  fadeToBlackBy( leds, NUM_LEDS, 230);
  leds[9] = CRGB::Orange;
  FastLED.show();
}

void warmLightState(){
  fill_solid(leds, NUM_LEDS, CRGB::Orange);
  FastLED.show();
}

void rotateColors(){
  int del = 100;
  for(int i = 0; i<255&&state==6; i+=10){
    fadeColorUp(CHSV(i, 255, 255), del);
    delay((state==6)*200);
  }
}
