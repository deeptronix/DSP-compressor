
//#define BOARD_MEGA2560
#define ANALOG_INPUT_A 3


// Compiler options
//#define DEBUG
//#define LOWPASS_FILTER
//#define HIGHPASS_FILTER
//#define NOTCH_FILTER
#define COMPRESSOR

// Constants:
#define SAMPLES 800
#define DECAY 5

#define THRESH_1 128
#define THRESH_2 220
#define THRESH_3 300
#define THRESH_4 380

#define FIXED_POINT_SHIFT 4



// ____ SELF CONFIGURATION ____

#ifdef BOARD_MEGA2560
  #define PORT_out PORTA
  #define PORT_dir DDRA
#else                       // Pro-mini or other boards
  #define PORT_out PORTD
  #define PORT_dir DDRD
#endif





// EOF
