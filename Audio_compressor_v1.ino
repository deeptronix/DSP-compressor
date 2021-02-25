
#include "z_Config.h"

#ifdef DEBUG
volatile bool stop_rec = false;
volatile int16_t samples[SAMPLES];
#endif

#ifdef COMPRESSOR
volatile int16_t samples[SAMPLES];
#endif

void setup(){
  #ifdef DEBUG
  Serial.begin(115200);
  #endif
  
  PORT_dir = 0xFF;  // Set DAC port to OUTPUT direction
  PORT_out = 0x80;  // aka 0V with respect to VirtGND

  #ifdef BOARD_MEGA2560
  DDRE |= (1 << 4);
  #else
  DDRC |= (1 << 4);
  #endif
  
  ADCInit();
}

ISR(ADC_vect){
  #ifdef BOARD_MEGA2560
  PORTE |= (1 << 4);
  #else
  PORTC |= (1 << 4);
  #endif

  
  // Read "10 bit" value from ADC
  uint8_t x_low = ADCL;
  uint16_t x_high = ADCH;
  uint16_t x_raw = 0;
  x_raw = (x_high << 8) | (x_low & 0xFF);
  int16_t x = x_raw - (1 << 9);   // Normalize ADC value to +-(half ADC range)

  #ifdef DEBUG
  static uint16_t r = 0;
  if(!stop_rec){
    samples[r] = x_raw;
    r++;
    if(r >= SAMPLES)  stop_rec = true;
  }
  #endif

  #ifdef LOWPASS_FILTER
  static int16_t y_old = 0;
  int16_t y = (9*x + 55*y_old) >> 6;  // At 38KHz sample rate, this introduces a lowpass filter with cutoff frequency at ~1KHz
  y_old = y;
  PORT_out = (y + (1 << 9)) >> 2;
  
  #elif defined (HIGHPASS_FILTER)
  static int16_t y_old = 0, x_old = 0;
  int16_t y = (x - x_old);  // At 38KHz sample rate, this introduces a highpass filter with cutoff frequency at ~1KHz
  y += (55*y_old)>>6;   // The damn compiler won't let me perform this operation in conjunction with the above ones...
  y_old = y;
  x_old = x;
  PORT_out = (y + (1 << 9)) >> 2;


  #elif defined (NOTCH_FILTER)      // At 38KHz sample rate, this introduces a notch filter with bandstop frequency at ~1080Hz
  static int16_t y_1 = 0, y_2 = 0, x_1 = 0, x_2 = 0;
  int16_t x_part = (x_1*63) >> 5;
  x_part = x + x_2 - x_part;
  int16_t y_part = (y_1*57 - y_2*26) >> 5;
  int16_t out_val = x_part + y_part;

  y_2 = y_1;
  y_1 = out_val;
  
  x_2 = x_1;
  x_1 = x;
  
  out_val = (out_val + (1 << 9)) >> 2;
  out_val = _clamp(out_val, 0, 255);
  PORT_out = out_val;
  
  #elif defined (COMPRESSOR)
  static uint16_t s = 0;
  static uint16_t max_val = 0, old_max;
  
  if(s == 0){
    old_max = max_val;    // Save max value before updating it: this will be used to apply the attenuation
    max_val = _clamp((max_val - DECAY), 0, 512);  // decrease current max_val; if no loud noises keep coming through, this will eventually go to 0
  }
  uint16_t x_abs = abs(x);
  max_val = (x_abs > max_val)?  x_abs : max_val;
  
  int16_t out_val = adaptValue(old_max, samples[s]);  // inline function to apply the correct attenuation factor based on thresholds
  out_val = out_val + (1 << 7);   // make value compatible with 8-bit DAC
  out_val = _clamp(out_val, 0, 255);  // if anything goes wrong, clamp values within the safe interval

  PORT_out = out_val;
  samples[s] = x;   // save current value to the buffer
  s++;    // increase saved samples pointer
  if(s >= SAMPLES)  s = 0;


  #else   // when no algorithm is enabled, simply parse ADC data to the DAC
  
  PORT_out = x_raw >> 2;
  #endif
  



  #ifdef BOARD_MEGA2560
  PORTE &= ~(1 << 4);
  #else
  PORTC &= ~(1 << 4);
  #endif
}



void loop(){
  #ifdef DEBUG
    if(stop_rec){
      for(uint16_t i = 0; i < SAMPLES; i++){
        Serial.println(samples[i]);
      }
      while(1);
    }
  #endif
}


void ADCInit(){
  ADCSRA = 0;             // clear ADCSRA register
  ADCSRB = 0;             // clear ADCSRB register
  ADMUX |= (ANALOG_INPUT_A & 0x07);    // set analog input pin; page 218 datasheet
  ADMUX = ADMUX | ((1 << REFS1) | (1 << REFS0));  // set reference voltage to internal 1.1V bandgap reference
  //ADMUX |= (1 << ADLAR);  // left align ADC value to 8 bits from ADCH register - only for 8 bits of resolution

  // SAMPLING_RATE = 76
  //ADCSRA |= (1 << ADPS2);                     // 16 prescaler for 76.9 KHz (8.5 < ENOB < 9)
  // SAMPLING_RATE = 153
  //ADCSRA |= (1 << ADPS1) | (1 << ADPS0);      // 8 prescaler for 153.8 KHz (ENOB < 7.5)
  // SAMPLING_RATE = 38
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0);      // 32 prescaler for 38.5 KHz (9 < ENOB < 9.5)
  

  ADCSRA |= (1 << ADATE); // enable auto trigger
  ADCSRA |= (1 << ADIE);  // enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN);  // enable ADC
  ADCSRA |= (1 << ADSC);  // start ADC measurements
}


/*
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
*/

// EOF
