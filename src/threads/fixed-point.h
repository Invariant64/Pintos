#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

/* Fixed point calculations. */

typedef int fixed_point;

/* Use a 32-bit signed integer to store a fixed point. 
   Fixed point in P.Q format. */
#define FP_P 17
#define FP_Q 14

#define FP_F (1 << FP_Q) 

/* Convertions between fixed point and integer. */
fixed_point int_to_fp (int);
int fp_to_int_0 (fixed_point);
int fp_to_int_n (fixed_point);

/* Caculations between a fixed point and a fixed point, 
   return a fixed point. */
fixed_point fp_add_fp (fixed_point, fixed_point);
fixed_point fp_sub_fp (fixed_point, fixed_point);
fixed_point fp_mul_fp (fixed_point, fixed_point);
fixed_point fp_div_fp (fixed_point, fixed_point);

/* Caculations between a fixed point and an integer, 
   return a fixed point. */
fixed_point fp_add_int (fixed_point, int);
fixed_point fp_sub_int (fixed_point, int);
fixed_point fp_mul_int (fixed_point, int);
fixed_point fp_div_int (fixed_point, int);

#endif /* threads/fixed-point.h */
