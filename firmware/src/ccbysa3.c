/*
 * CC BY-SA 3.0 Content
 * 
 * @Company
 *   Community
 * 
 * @File Name
 *   ccbysa3.c
 * 
 * @Summary
 *   Contents of this file are from StackOverflow or other sources which are
 *   licensed under CC BY-SA 3.0. This file is not covered by the main project
 *   license, the contents of this file are licensed under CC BY-SA 3.0.
 *   Attribution for each item will be found in an adjacent comment. Items
 *   lacking attribution can be assumed to original.
 * 
 * @par
 *   Copyright held by various authors as noted in source.
 *   Licensed under Creative Commons Attribution-ShareAlike 3.0 Unported:
 *   https://creativecommons.org/licenses/by-sa/3.0/legalcode
*/

#include <stdint.h>


// https://stackoverflow.com/a/2602885
uint8_t
cbs_reverse (uint8_t b)
{
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}
