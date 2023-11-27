/* Fixed point calculations. */

typedef int fixed_point;

/* Use a 32-bit signed integer to store a fixed point. 
   Fixed point in P.Q format. */
#define FP_P 17
#define FP_Q 14

#define FP_F (1 << FP_Q) 

/* Convert integer to fixed point. */
fixed_point int_to_fp (int n)
{
  return n * FP_F;
}

/* Convert fixed point ti integer (rounding toward zero). */
fixed_point fp_to_int_0 (fixed_point x) 
{
  return x / FP_F;
}

/* Convert fixed point ti integer (rounding toward nearest). */
fixed_point fp_to_int_n (fixed_point x)
{ 
  if (x > 0)
    return (x + FP_F / 2) / FP_F;
  else
    return (x - FP_F / 2) / FP_F;
}

/* Caculations between a fixed point and a fixed point, 
   return a fixed point. */
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

/* Caculations between a fixed point and an integer, 
   return a fixed point. */
fixed_point fp_add_int (fixed_point x, int n)
{
  return x + n / FP_F;
}

fixed_point fp_sub_int (fixed_point x, int n)
{
  return x - n / FP_F;
}

fixed_point fp_mul_int (fixed_point x, int n)
{
  return x * n;
}

fixed_point fp_div_int (fixed_point x, int n)
{
  return x / n;
}
