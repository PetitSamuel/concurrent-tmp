//
// CSU33014 Summer 2020 Additional Assignment
// Part A of a two-part assignment
//

// Please examine version each of the following routines with names
// starting partA. Where the routine can be vectorized, please
// complete the corresponding vectorized routine using SSE vector
// intrinsics. Where is cannot be vectorized...

// Note the restrict qualifier in C indicates that "only the pointer
// itself or a value directly derived from it (such as pointer + 1)
// will be used to access the object to which it points".

#include <immintrin.h>
#include <stdio.h>

#include "csu33014-annual-partA-code.h"

/****************  routine 0 *******************/

// Here is an example routine that should be vectorized
void partA_routine0(float *restrict a, float *restrict b,
                    float *restrict c)
{
  for (int i = 0; i < 1024; i++)
  {
    a[i] = b[i] * c[i];
  }
}

// here is a vectorized solution for the example above
void partA_vectorized0(float *restrict a, float *restrict b,
                       float *restrict c)
{
  __m128 a4, b4, c4;

  for (int i = 0; i < 1024; i = i + 4)
  {
    b4 = _mm_loadu_ps(&b[i]);
    c4 = _mm_loadu_ps(&c[i]);
    a4 = _mm_mul_ps(b4, c4);
    _mm_storeu_ps(&a[i], a4);
  }
}

/***************** routine 1 *********************/

// in the following, size can have any positive value
float partA_routine1(float *restrict a, float *restrict b,
                     int size)
{
  float sum = 0.0;

  for (int i = 0; i < size; i++)
  {
    sum = sum + a[i] * b[i];
  }
  return sum;
}

// insert vectorized code for routine1 here
float partA_vectorized1(float *restrict a, float *restrict b,
                        int size)
{
  // replace the following code with vectorized code
  float sum = 0.0;

  // amount of multiple of 4s
  int vlen = size / 16;

  // convert to array of __m128
  __m128 *av = (__m128 *)a;
  __m128 *bv = (__m128 *)b;

  // sum __m128 variable
  __m128 sumv = _mm_setr_ps(0, 0, 0, 0);
  int i;
  for (i = 0; i < vlen; i++)
  {
    __m128 mult = _mm_mul_ps(av[i], bv[i]);
    sumv = _mm_add_ps(sumv, mult);
  }

  // align index with non vectorised values
  i *= 4;

  // iterate over the uncovered values (size % 4 values to go through)
  for (; i < size; i++)
  {
    sum = sum + a[i] * b[i];
  }

  // sum values stored in SSE
  sumv = _mm_hadd_ps(sumv, sumv);
  sumv = _mm_hadd_ps(sumv, sumv);

  // retrieve sum of vectorized code
  float r;
  _mm_store_ps(&r, sumv);
  return sum + r;
}

/******************* routine 2 ***********************/

// in the following, size can have any positive value
void partA_routine2(float *restrict a, float *restrict b, int size)
{
  for (int i = 0; i < size; i++)
  {
    a[i] = 1 - (1.0 / (b[i] + 1.0));
  }
}

// in the following, size can have any positive value
void partA_vectorized2(float *restrict a, float *restrict b, int size)
{
  // cast pointers to sse pointers
  __m128 *av = (__m128 *)a;
  __m128 *bv = (__m128 *)b;

  // create an sse variable with all 1's
  __m128 onev = _mm_setr_ps(1, 1, 1, 1);

  // go through the amount of times vectorizable
  int i;
  for (i = 0; i < size / 4; i++)
  {
    // add 1 to b[i], divide 1 by the result of the previous operation and subract one by the prev operations result.
    av[i] = _mm_add_ps(bv[i], onev);
    av[i] = _mm_div_ps(onev, av[i]);
    av[i] = _mm_sub_ps(onev, av[i]);
  }

  // the values changed by sse are still pointed to by a thus no need to cast back
  //compute size % 4 values (not multiple of 4 thus non vectorizable with SSE)
  for (i *= 4; i < size; i++)
  {
    a[i] = 1 - (1.0 / (b[i] + 1.0));
  }
}

/******************** routine 3 ************************/

// in the following, size can have any positive value
void partA_routine3(float *restrict a, float *restrict b, int size)
{
  for (int i = 0; i < size; i++)
  {
    if (a[i] < 0.0)
    {
      a[i] = b[i];
    }
  }
}

// in the following, size can have any positive value
void partA_vectorized3(float *restrict a, float *restrict b, int size)
{
  // cast to __m128 arrays
  __m128 *av = (__m128 *)a;
  __m128 *bv = (__m128 *)b;
  __m128 zeros = _mm_setzero_ps();
  int i;
  for (i = 0; i < size / 4; i++)
  {
    // execute SSE comparison with zeros
    __m128 cmpv = _mm_cmplt_ps(av[i], zeros);

    // get values to copy from av and set others to zero
    // this is done by inversing the comparison mask and executing a logical
    // AND with the values in av
    __m128 copyFromAv = _mm_andnot_ps(cmpv, av[i]);

    // get values to copy from bv and set the ones not to copy to 0
    // this is done by ANDing the values in bv with the comparison bits
    __m128 copyFromBv = _mm_and_ps(cmpv, bv[i]);

    // OR the two obtained values together to obtain the result.
    av[i] = _mm_or_ps(copyFromAv, copyFromBv);
  }

  // Execute for non multiple of 4s (non vectorizable)
  for (i *= 4; i < size; i++)
  {
    if (a[i] < 0.0)
    {
      a[i] = b[i];
    }
  }
}

/********************* routine 4 ***********************/

// hint: one way to vectorize the following code might use
// vector shuffle operations
void partA_routine4(float *restrict a, float *restrict b,
                    float *restrict c)
{
  for (int i = 0; i < 2048; i = i + 2)
  {
    a[i] = b[i] * c[i] - b[i + 1] * c[i + 1];
    a[i + 1] = b[i] * c[i + 1] + b[i + 1] * c[i];
  }
}

void partA_vectorized4(float *restrict a, float *restrict b,
                       float *restrict c)
{
  __m128 *av = (__m128 *)a;
  __m128 *bv = (__m128 *)b;
  __m128 *cv = (__m128 *)c;
  __m128 mask = _mm_setr_ps(-1, 1, -1, 1);
  for (int i = 0; i < 512; i++)
  {
    // CONSIDERING : left side of multiplications (b[i] * c[i] and b[i+2]c[i+1])
    // store odd indexed values in even ones for b values
    __m128 left = _mm_moveldup_ps(bv[i]);

    // left side multiplication doesn't require to rearange values in c
    left = _mm_mul_ps(left, cv[i]);

    // CONSIDERING : right side of multiplications (b[i+1]c[i+1] and b[i+1]c[i])
    // store even indexed values in odd ones for b values
    __m128 bRight = _mm_movehdup_ps(bv[i]);

    // re arrange values in C with order : i + 1, i, i + 3, i + 2
    __m128 cRight = _mm_shuffle_ps(cv[i], cv[i], _MM_SHUFFLE(2, 3, 0, 1));

    // multiply newly arranged b and c values
    __m128 right = _mm_mul_ps(bRight, cRight);

    // change the sign of odd indexed values of the right side multiplication as these are subtracted
    right = _mm_mul_ps(right, mask);

    // add into output
    av[i] = _mm_add_ps(left, right);
  }
}

/********************* routine 5 ***********************/

// in the following, size can have any positive value
void partA_routine5(unsigned char *restrict a,
                    unsigned char *restrict b, int size)
{
  for (int i = 0; i < size; i++)
  {
    a[i] = b[i];
  }
}

void partA_vectorized5(unsigned char *restrict a,
                       unsigned char *restrict b, int size)
{
  int i;
  // cast a and b into a pair of __m128i pointers.
  __m128i *av = (__m128i *)a;
  __m128i *bv = (__m128i *)b;

  // copy values from b into a, 16 at a time
  for (i = 0; i < size / 16; i++)
  {
    av[i] = bv[i];
  }

  // align back i with non vectorised code
  i *= 16;
  // copy size % 16 remaining values
  for (; i < size; i++)
  {
    a[i] = b[i];
  }
}

/********************* routine 6 ***********************/

void partA_routine6(float *restrict a, float *restrict b,
                    float *restrict c)
{
  a[0] = 0.0;
  for (int i = 1; i < 1023; i++)
  {
    float sum = 0.0;
    for (int j = 0; j < 3; j++)
    {
      sum = sum + b[i + j - 1] * c[j];
    }
    a[i] = sum;
  }
  a[1023] = 0.0;
}

void partA_vectorized6(float *restrict a, float *restrict b,
                       float *restrict c)
{
  __m128 *av = (__m128 *)a;
  __m128 *bv = (__m128 *)b;
  __m128 *cv = (__m128 *)c;
  /*
  // replace the following code with vectorized code
  
  __m128 mask = _mm_setr_ps(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0);
  __m128 currentv;
  for (int i = 0; i < 1020; i++)
  {
    currentv = _mm_setzero_ps();
    float sum = 0.0;

    __m128 mult = _mm_mul_ps(bv[i], cv[0]);
    mult = _mm_and_ps(mult, mask);
    mult = _mm_hadd_ps(mult, mult);
    mult = _mm_hadd_ps(mult, mult);
    _mm_store_ps(&sum, mult);
    a[i + 1] = sum;
  }
  a[1023] = 0.0;
  */

  a[0] = 0.0;
  for (int i = 0; i < 1022; i++)
  {
    __m128 mult = _mm_mul_ps(bv[i], cv[0]);
    float r[4];
    _mm_store_ps(&r, mult);
    a[i + 1] = r[0] + r[1] + r[2];
  }
  a[1023] = 0.0;
}
