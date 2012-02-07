#ifndef ROUND_H
#define ROUND_H

// Symmetric Rounding Algorithm  - equivalent to C99 round()
double sym_round(double val);

// Asymmetric Rounding Algorithm  - equivalent to Java Math.round()
double java_math_round(double val);

// Implementation of rint() 
double rint_vc(double val);

//inline double round(double val);

#endif
