/*******************************************************************************
 *  File   : c.h
 *  Author : Suguru Aoki @ HVD
 *  Update : 2025/04/08
 *  Overview : 
 *******************************************************************************/

#ifndef _c_h
#define _c_h

/* ------------------------ *
 *   Type definition
 * ------------------------ */

  typedef unsigned char  U1;
  typedef signed   char  S1;
  typedef unsigned short U2;
  typedef signed   short S2;
  typedef unsigned long  U4;
  typedef signed   long  S4;
  typedef          int   LOOP;

  typedef union {
      U1 Byte;
      struct {
          U1 b0:   1;
          U1 b1:   1;
          U1 b2:   1;
          U1 b3:   1;
          U1 b4:   1;
          U1 b5:   1;
          U1 b6:   1;
          U1 b7:   1;
      } Bit;
  } ST_BYTE;


/* ------------------------ *
 *   Macro definition
 * ------------------------ */

  #define ON         (U1)(1)             /* U1型 ON    */
  #define OFF        (U1)(0)             /* U1型 OFF   */
  #define NA         (U1)(0xFF)          /* U1型 不定値 */

  #define U1MAX      (U1)(255)           /* U1型 最大値 */
  #define U2MAX      (U2)(65535)         /* U2型 最大値 */
  #define U4MAX      (U4)(4294967295L)   /* U4型 最大値 */

  #define S1MAX      (S1)(127)           /* S1型 最大値 */
  #define S1MIN      (S1)(-128)          /* S1型 最小値 */
  #define S2MAX      (S2)(32767)         /* S2型 最大値 */
  #define S2MIN      (S2)(-32768)        /* S2型 最小値 */
  #define S4MAX      (S4)(2147483647)    /* S4型 最大値 */
  #define S4MIN      (S4)(-2147483648)   /* S4型 最小値 */

  #define U2_ADC_MAX (U2)(1023)          /* ADC値 最大値 */
  #define U2_ADC_MIN (U2)(0)             /* ADC値 最小値 */

#endif
