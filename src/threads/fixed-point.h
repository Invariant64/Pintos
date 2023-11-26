/* Fixed point calculations. */

/* Use a 32-bit signed integer to store a fixed point. 
   Fixed point in P.Q format. */
#define FP_P 17
#define FP_Q 14

#define FP_F (1 << FP_Q) 

/* Convert integer to fixed point. */
#define int_to_fp(n) ((n) * FP_F)
/* Convert fixed point ti integer (rounding toward zero). */
#define fp_to_int_0(x) ((x) / FP_F)
/* Convert fixed point ti integer (rounding toward nearest). */
#define fp_to_int_n(x) (((x) > 0) ? ((x + FP_F / 2) / FP_F) :\
                                    ((x - FP_F / 2) / FP_F))

/* Caculations between a fixed point and a fixed point, 
   return a fixed point. */
#define fp_add_fp(x, y) ((x) + (y))
#define fp_sub_fp(x, y) ((x) - (y))
#define fp_mul_fp (x, y) (((int64_t) (x)) * (y) / FP_F)
#define fp_div_fp (x, y) (((int64_t) (x)) * FP_F / (y))

/* Caculations between a fixed point and an integer, 
   return a fixed point. */
#define fp_add_int(x, n) ((x) + (n) / FP_F)
#define fp_sub_int(x, n) ((x) - (n) / FP_F)
#define fp_mul_int (x, n) ((x) * (n))
#define fp_div_int (x, n) ((x) / (n))


              



