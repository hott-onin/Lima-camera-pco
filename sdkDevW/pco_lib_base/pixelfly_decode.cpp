//-----------------------------------------------------------------//
// Name        | pixelfly_decode.cpp         | Type: (*) source    //
//-------------------------------------------|       ( ) header    //
// Project     | pco.camera                  |       ( ) others    //
//-----------------------------------------------------------------//
// Platform    | Linux, Windows2000/XP                             //
//-----------------------------------------------------------------//
// Environment |                                                   //
//             |                                                   //
//-----------------------------------------------------------------//
// Purpose     | pco.camera - Pixelfly USB                         //
//-----------------------------------------------------------------//
// Author      | EIJ, PCO AG                                       //
//-----------------------------------------------------------------//
// Revision    | rev. 1.00                                         //
//-----------------------------------------------------------------//
// Notes       | Extract image data from Pixelfly stream           //
//             | For linux library                                 //
//             |                                                   //
//-----------------------------------------------------------------//
// (c) 2015 PCO AG                                                 //
// Donaupark 11 D-93309  Kelheim / Germany                         //
// Phone: +49 (0)9441 / 2005-0   Fax: +49 (0)9441 / 2005-20        //
// Email: info@pco.de                                              //
//-----------------------------------------------------------------//


#include "defs.h"


void Pixelfly_decode(WORD* buffer, int len, WORD align)
{
    WORD* adrdc = (WORD*) buffer;
    WORD bits;
    WORD tempadr;

    if(align == 0)
    {
      WORD bittable[4] = {0x1040,0x1000,0x0040,0x0000};  // Table for this alignment
      for(LONG ly=0;ly<len/2;ly++)
      {
        tempadr = *(adrdc+ly);             // Get entry from memory
        bits = tempadr & 0x0003;           // Get "Reserve" bits (0+1)
        tempadr |= bittable[bits];         // logical OR with the appropriate table entry
        tempadr &= 0xFFFC;                 // Set "Reserve" bits to 0
        *(adrdc+ly) = tempadr;             // write entry back to memory
      }
    }
    else if(align == 1)
    {
      WORD bittable[4] = {0x0410,0x0400,0x0010,0x0000};
      for(LONG ly=0;ly<len/2;ly++)
      {
        tempadr = *(adrdc+ly);
        bits = tempadr & 0xC000;          // Get "Reserve" bits (14+15)
        bits >>= 14;                      // Shift bits to 0+1
        tempadr |= bittable[bits];        // logical OR with the appropriate table entry
        tempadr &= 0x3FFF;                // Set "Reserve bits to 0
        *(adrdc+ly) = tempadr;
      }
    }
}
