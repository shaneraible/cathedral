#include "Arduino.h"
#include <FastLED.h>

typedef int (*State) ();

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
void state0();
void state1();
void state2();
void state3();
void state4();
void state5();
void state6();
State states[] = { state2, state1, state2, state3, state4, state5, state6 };

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
  delay(3000);

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
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1);
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow

}

void setRange(int x1, int x2, CRGB col){
  for(int i = x1; i<=x2; i++){
    leds[i] = col;
  }
}

void explodeLED(CRGB col, unsigned int del, unsigned int s){
  for(int i=NUM_LEDS/2, k=NUM_LEDS/2+(NUM_LEDS%2==0?1:0); (i<NUM_LEDS || k>=0) && s==state; i++, k--){
    if(i<NUM_LEDS)
      leds[i] = col;
    if(k>=0)
      leds[k] = col;
    FastLED.show();
    delay(del);
  }
}

void state0(){
  FastLED.clear();
  FastLED.show();
  delay((state==0)*2000);
}
void state1(){
  explodeLED(CRGB::Blue, 20, 1);
  delay((state==1) * 2000);
  explodeLED(CRGB::Yellow, 20, 1);
}
void state2(){
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}
void state3(){
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}
void state4(){
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}
void state5(){
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}
void state6(){
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
