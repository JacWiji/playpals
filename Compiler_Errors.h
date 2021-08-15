#ifndef COMPILER_ERRORS_H
#define COMPILER_ERRORS_H

  // we only support Arduino 1.6.6 or greater
  #if ARDUINO < 10606
    #error Please upgrade your Arduino IDE to 1.6.6 or greater
  #else 
    // check that Bare Conductive Touch Board is selected in Tools -> Board
    #if !defined(ARDUINO_AVR_BARETOUCH) || defined(IPAD_COMPAT)
      #error Please select "Bare Conductive Touch Board" in the Tools -> Board menu.
    #endif
  #endif

#endif // COMPILER_ERRORS_H
