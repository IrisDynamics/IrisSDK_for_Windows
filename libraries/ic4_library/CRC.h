/**
 * @file CRC.h
 * @author Dan Beddoes
 *  Created on: May 7, 2021
 */

#pragma once
#include "config.h"


#define CRC_POLYNOMIAL	0xD5

/*
The following CRCFast class was taken from
https://barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
and can be found here:
Barr, Michael. "Slow and Steady Never Lost the Race," Embedded Systems Programming, January 2000, pp. 37-46.

Big thank you for this!
*/
class CRC {

  public:
    static uint8_t table [256];
    static int is_built;

    static uint8_t generate (const uint8_t message[], int nBytes) {
      uint8_t data;
      uint8_t remainder = 0;

      // Divide the message by the polynomial, a byte at a time.
      for (int byte = 0; byte < nBytes; ++byte)
      {
          data = message[byte] ^ remainder;
          remainder = table[data] ^ (remainder << 8);
      }

      // The final remainder is the CRC.
      return (remainder);
    }

    static void build_table() {

    	if (is_built) return;  // prevents building the table multiple times
      uint8_t remainder;

      for (int dividend = 0; dividend < 256; ++dividend)
      {
        remainder = dividend;

        // Perform modulo-2 division, a bit at a time.
        for (uint8_t bit = 8; bit > 0; --bit)
        {
          if (remainder & 0x80)
            remainder = (remainder << 1) ^ CRC_POLYNOMIAL;
          else
            remainder = (remainder << 1);
         }

         //Store the result into the table.
         table [dividend] = remainder;
      }

      is_built = 1;
    }
};

