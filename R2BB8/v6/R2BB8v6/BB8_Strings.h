/****
  * Flash Storage of All User-Facing Strings, across Multiple Languages.
  *
  * The purpose of the following structure is to maximize the ability to
  * localize strings on the fly while minimizing MCU memory consumption,
  * namely RAM but also program space.
****/
#ifndef _BB8_STRINGS_H
#define _BB8_STRINGS_H


#include <WString.h>

//Number of Languages:
#define NUM_LANG 2
//Language Indices for the Subsequent String Arrays.
#define LID_ENGLISH 0
#define LID_SPANISH 1

byte CURR_LANG = 0;// Variable for Current Language

/*
 Safely get a String from the desired Array in the Given Language, lang,
 returning the English result if no string is defined for the desired language.
 Example Call: GetString(STR__LANGUAGE_NAME, Lang.ENGLISH)
*/
const __FlashStringHelper* GetString(const __FlashStringHelper* arr[], byte lang){
  const __FlashStringHelper* ret_arr = arr[0];
  if(arr[lang] != nullptr){
    ret_arr = arr[lang];
  } else{
    Serial.println(">GetString OverBounds Detected");
  }
  return ret_arr;
}
// const __FlashStringHelper* GetString(const __FlashStringHelper* arr[], byte lang){
//   const __FlashStringHelper* ret_str; // -Return String (in RAM)
//   // Get English (idx=0) if no value at specified [lang]:
//   const __FlashStringHelper* prog_str = arr[0]; // - Program Memory String (in PROGMEM).
//   if(arr[lang] != nullptr){
//     prog_str = arr[lang];
//   }
//
//   Serial.print("PS: ");
//   Serial.println(prog_str);
//
//   ret_str = prog_str;
//   // //Rebuild String in RAM from PROGMEM:
//   // char c = 1;//Default value != 0
//   // for(int i=0; c!=0; i++){
//   //   c = pgm_read_byte_near(prog_str++);
//   //   Serial.print("C: ")
//   //   Serial.println(c);
//   //   ret_str[i] = c;
//   // }
//   //ret_str[strlen(prog_str)];
//
//   return ret_str;
// };
//Shorthand:
#define GS(a,l) GetString(a,l)
//Get String in Current Language:
#define GSC(a) GetString(a, CURR_LANG)

/*
 Stores the Given String in PROGMEM Flash.
 This macro exists in case MCUs are changed and the flash storage technique
 is different.
*/
#define SS(s) F(s)

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////// -- STRINGS --
////////////////////////////////////////////////////////////////////////////////
const __FlashStringHelper* STR__LANGUAGE_NAME[NUM_LANG];/// PROGMEMify?

const __FlashStringHelper* STR__X_Position[NUM_LANG];
const __FlashStringHelper* STR__Y_Position[NUM_LANG];
const __FlashStringHelper* STR__Z_Position[NUM_LANG];
bool strings_populated = 0;

// This class exists because #PSTR (inside #SS) must be called from within a
// function and these strings must be created at start (the class constructor
// allows #populateStrings to be called at start / when this header is called).
void populateStrings(){
  static bool populated = 0;
  //Name of Language at Index
  strings_populated = 1;
  if(!populated){
    STR__LANGUAGE_NAME[LID_ENGLISH] = SS("ENGLISH");
    STR__LANGUAGE_NAME[LID_SPANISH] = SS("ESPANOL");

    STR__X_Position[LID_ENGLISH] = SS("X Position");
    STR__X_Position[LID_SPANISH] = SS("Posicion X");

    STR__Y_Position[LID_ENGLISH] = SS("Y Position");
    STR__Y_Position[LID_SPANISH] = SS("Posicion Y");

    STR__Z_Position[LID_ENGLISH] = SS("Z Position");
    STR__Z_Position[LID_SPANISH] = SS("Posicion Z");
    populated = 1;
  }
};


#endif //_BB8_STRINGS_H
