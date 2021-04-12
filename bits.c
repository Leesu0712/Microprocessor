/* 
 * CS:APP Data Lab 
 * 
 * Name: Su-hyun Lee  User id: y2016142109
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
/* We do not support C11 <threads.h>.  */
//1
/* 
 * minusOne - return a value of -1 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 1
 */
int minusOne(void) {
  return ~0;
}
/* 
 * thirdBits - return word with every third bit (starting from the LSB) set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
 */
int thirdBits(void) {
  int OneBlock = 0x49;
  int TwoBlock = (OneBlock << 9) | OneBlock;
  int ThreeBlock = (TwoBlock << 18) | TwoBlock;
  return ThreeBlock;
}
/* 
 * TMax - return maximum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmax(void) {
  int tmin= 1 << 31;
  return ~tmin;
}
/* 
 * upperBits - pads n upper bits with 1's
 *  You may assume 0 <= n <= 32
 *  Example: upperBits(4) = 0xF0000000
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 10
 *  Rating: 1
 */
int upperBits(int n) {
  int MSB = 1 << 31;
  int minusOne = ~0;
  /* ( !n is used for special case n = 0 )
   * If n is zero, result is MSB << 1. Otherwise, result is MSB >> (n-1). */
  return (MSB >> (n + minusOne + !n)) << !n;
}
//2
/* 
 * anyEvenBit - return 1 if any even-numbered bit in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples anyEvenBit(0xA) = 0, anyEvenBit(0xE) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int anyEvenBit(int x) {
  int QuarterMask = 0x55;
  int HalfMask = (QuarterMask << 8) | QuarterMask;
  int FullMask = (HalfMask << 16) | HalfMask;
  return !!(x & FullMask);
}
/* 
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2 
 */
int leastBitPos(int x) {
  int negx = ~x + 1;	// 2's complement of x
  return negx & x;
}
//3
/* 
 * bitMask - Generate a mask consisting of all 1's 
 *   lowbit and highbit
 *   Examples: bitMask(5,3) = 0x38
 *   Assume 0 <= lowbit <= 31, and 0 <= highbit <= 31
 *   If lowbit > highbit, then mask should be all 0's
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int bitMask(int highbit, int lowbit) {
  int highMask = ((1 << highbit) << 1) + ~0;
  int lowMask = ~((1 << lowbit) + ~0);
  return highMask & lowMask;
}
/* 
 * isNegative - return 1 if x < 0, return 0 otherwise 
 *   Example: isNegative(-1) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int isNegative(int x) {
  int SignMask = 1 << 31;
  /* If (SignMask & x) is 0, x is positive. Otherwise, x is negative. */
  return !!(SignMask & x);
}
/* 
 * isNonNegative - return 1 if x >= 0, return 0 otherwise 
 *   Example: isNonNegative(-1) = 0.  isNonNegative(0) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int isNonNegative(int x) {
  int SignMask = 1 << 31;
  /* If (SignMask & x) is 0, x is positive. Otherwise, x is negative. */
  return !(SignMask & x);
}
/*
 * multFiveEighths - multiplies by 5/8 rounding toward 0.
 *   Should exactly duplicate effect of C expression (x*5/8),
 *   including overflow behavior.
 *   Examples: multFiveEighths(77) = 48
 *             multFiveEighths(-22) = -13
 *             multFiveEighths(1073741824) = 13421728 (overflow)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 3
 */
int multFiveEighths(int x) {
  int SignMask = 1 << 31;
  int SignExtend = (x & SignMask) >> 31;	    // Extend the Sign bit for all 32 bits
  /* If x is negative, reaminCheck = 0x7. Otherwise, remainCheck = 0x0 */
  int remainCheck = 0x7 & SignExtend;
  /* Compute (4 * x + x (+ remaincheck)) / 8 
   * If x is negative and 5 * x has any nonzero value for low-order 3 bits, 
   * add remaincheck 0x7 to implement rounding toward 0. */
  return ((x << 2) + x + remainCheck) >> 3;
}
/*
 * satMul3 - multiplies by 3, saturating to Tmin or Tmax if overflow
 *  Examples: satMul3(0x10000000) = 0x30000000
 *            satMul3(0x30000000) = 0x7FFFFFFF (Saturate to TMax)
 *            satMul3(0x70000000) = 0x7FFFFFFF (Saturate to TMax)
 *            satMul3(0xD0000000) = 0x80000000 (Saturate to TMin)
 *            satMul3(0xA0000000) = 0x80000000 (Saturate to TMin)
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 3
 */
int satMul3(int x) {
  int xMul2 = x << 1;
  int xMul3 = x + xMul2;
  int s1 = x >> 31; 		// SignBit for x
  int s2 = xMul2 >> 31; 	// SignBit for 2 * x
  int s3 = xMul3 >> 31;		// SignBit for 3 * x
  int sat = (s1 ^ s2) | (s1 ^ s3); 	// If saturating is needed, sat is 1. Otherwise, sat is 0.
  int Tmin = 1 << 31; 
  int Tmax = ~Tmin;
  /* For positive int x (s1 = 0) : without overflow (sat = 0), return x_sum
   * 				 : with overflow (sat = 1), return Tmax
   * For negative int x (s1 = -1) : without overflow (sat = 0), return x_sum
   * 				  : with overflow (sat = 1), return Tmin */
  return (((~s1 & Tmax) + (s1 & Tmin)) & sat) + (xMul3 & ~sat);
}
//4
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  int Tmin = 1 << 31; 
  int Tmax = ~Tmin;
  /* If x is positive nonzero, (Tmax + x) should be overflow */
  int P_check = ((Tmax + x) >> 31); 
  /* If x is negative nonzero, (Tmin + x) should be overflow */
  int N_check = (~((Tmin + x) >> 31));
  return (~(P_check | N_check)) & 1;
}
/* 
 * isNonZero - Check whether x is nonzero using
 *              the legal operators except !
 *   Examples: isNonZero(3) = 1, isNonZero(0) = 0
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4 
 */
int isNonZero(int x) {
  int Tmin = 1 << 31; 
  int Tmax = ~Tmin;
  /* If x is positive nonzero, (Tmax + x) should be overflow */
  int P_check = ((Tmax + x) >> 31); 
  /* If x is negative nonzero, (Tmin + x) should be overflow */
  int N_check = (~((Tmin + x) >> 31));
  return (P_check | N_check) & 1;
}
//float
/* 
 * floatAbsVal - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned floatAbsVal(unsigned uf) {
  int sign = uf >> 31; 
  int exp = (uf >> 23) & 0xFF;
  int AbsMask = ~(1 << 31);
  if ((!(exp ^ 0xFF)) && (uf << 9))
    return uf;			// Case 1: NaN (exp is 0xFF and frac isn't 0x00)
  if (sign)
    return AbsMask & uf;	// Case 2: f is negative.
  else 
    return uf;			// Case 3: f is positive.
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  int posInf = 0xFF << 23;
  if ((x < 129) && (x > -127)) 
    return (x + 127) << 23;		// Case 1: Normalized (x = -126 ~ 128)
  else if (!(x >> 31))
    return posInf;			// Case 2: Too large (x = 129 ~ Tmax)
  else if(x > -150)
    return 1 << (x + 149);		// Case 3: Denormalized (x = -149 ~ -127)
  else
    return 0;				// Case 4: Too small (x = Tmin ~ -150)
}
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  unsigned exp, frac;
  exp = (uf >> 23) & 0xFF;
  frac = (uf << 9) >> 9;
  /* Case 1: Normalized (exp = 1 ~ 253) */
  if ((exp > 0) && (exp < 254)) {
    return ((exp + 1) << 23) | (~(0xFF << 23) & uf); }	// Replace exp to exp + 1
  /* Case 2: Denormalized (exp = 0) */  
  else if (exp == 0) {		
    /* Case 2-1: frac is overflow */	  
    if (uf & (1 << 22))
      return (frac << 1) | ((uf >> 31) << 31);		// Set 0x00 for exp
    /* Case 2-2: frac is not overflow */
    else 
      return (frac << 1) | ((uf >> 23) << 23); }	// Remain exp value
  /* Case 3: exp is overflow (exp = 254) */
  else if (exp == 254)
    return uf | (1 << 23); 				// Set 0xFF for exp
  /* Case 4: Special, argument f is infinity or NaN */
  else
    return uf;
}
