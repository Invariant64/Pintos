#include <stdint.h>
#include "threads/fixed-point.h"


/* Convert integer to fixed point. */
fixed_point int_to_fp (int n)
{
  return n * FP_F;
}

/* Convert fixed point to integer (rounding toward zero). */
int fp_to_int_0 (fixed_point x) 
{
  return x / FP_F;
}

/* Convert fixed point to integer (rounding toward nearest). */
int fp_to_int_n (fixed_point x)
{ 
  if (x > 0)
    return (x + FP_F / 2) / FP_F;
  else
    return (x - FP_F / 2) / FP_F;
}

fixed_point fp_add_fp (fixed_point x, fixed_point y)
{
  return x + y;
}

fixed_point fp_sub_fp (fixed_point x, fixed_point y)
{
  return x - y;
}

fixed_point fp_mul_fp (fixed_point x, fixed_point y)
{
  return (int64_t) x * y / FP_F;
}

fixed_point fp_div_fp (fixed_point x, fixed_point y)
{
  return (int64_t) x * FP_F / y;
}

fixed_point fp_add_int (fixed_point x, int n)
{
  return x + n * FP_F;
}

fixed_point fp_sub_int (fixed_point x, int n)
{
  return x - n * FP_F;
}

fixed_point fp_mul_int (fixed_point x, int n)
{
  return x * n;
}

fixed_point fp_div_int (fixed_point x, int n)
{
  return x / n;
}

fixed_point int_add_int (int n, int m)
{
  return n * FP_F + m * FP_F;
}

fixed_point int_sub_int (int n, int m)
{
  return n * FP_F - m * FP_F;
}

fixed_point int_mul_int (int n, int m)
{
  return (int64_t) FP_F * n * m;
}

fixed_point int_div_int (int n, int m)
{
  return (int64_t) FP_F * n / m;
}
