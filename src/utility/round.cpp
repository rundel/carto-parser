#include <utility/round.hpp>

#include <cmath>

// Symmetric Rounding Algorithm  - equivalent to C99 round()
double sym_round(double val) {
    double n;
    double f = fabs(modf(val, &n));
    if (val >= 0) {
        if (f < 0.5) {
            return( floor(val) );
        } else if (f > 0.5) {
            return( ceil(val) );
        } else {
            return( n + 1.0 );
        }
    } else {
        if (f < 0.5) {
            return( ceil(val) );
        } else if (f > 0.5) {
            return( floor(val) );
        } else {
            return( n - 1.0 );
        }
    }
}


// Asymmetric Rounding Algorithm  - equivalent to Java Math.round()
double java_math_round(double val) {
    double n;
    double f = fabs(modf(val, &n));

    if (val >= 0) {
        if (f < 0.5) {
            return( floor(val) );
        } else if (f > 0.5) {
            return( ceil(val) );
        } else {
            return( n + 1.0 );
        }
    } else {
        if (f < 0.5) {
            return( ceil(val) );
        } else if (f > 0.5) {
            return( floor(val) );
        } else {
            return( n );
        }
    }
} 

// Implementation of rint() 
double rint_vc(double val) {
    double n;
    double f=fabs(modf(val,&n));
    if (val>=0) {
        if (f<0.5) {
            return( floor(val) );
        } else if (f>0.5) {
            return( ceil(val) );
        } else {
            return( (floor(n/2)==n/2)?n:n+1.0 );
        }
    } else {
        if (f<0.5) {
            return( ceil(val) );
        } else if (f>0.5) {
            return( floor(val) );
        } else {
            return(( floor(n/2)==n/2)?n:n-1.0 );
        }
    }
}

/*inline double round(double val) {
    return( java_math_round(val) );
}*/

