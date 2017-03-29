/*
 * Copyright (c) 2008-2010
 *	Nakata, Maho
 * 	All rights reserved.
 *
 * $Id: Csymv.debug.cpp,v 1.4 2010/08/07 05:50:10 nakatamaho Exp $
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <mblas.h>
#include <mlapack.h>
#include <mpack_debug.h>

#include <blas.h>
#include <lapack.h>

#if defined VERBOSE_TEST
#include <iostream>
#endif

#define MIN_N -2
#define MAX_N 10
#define MIN_LDA 2
#define MAX_LDA 10
#define MIN_INCX -2
#define MAX_INCX 2
#define MIN_INCY -2
#define MAX_INCY 2
#define MAX_ITER 3

REAL_REF maxdiff = 0.0;

void Csymv_test3(const char *uplo, COMPLEX_REF alpha_ref, COMPLEX_REF beta_ref, COMPLEX alpha, COMPLEX beta)
{
    int errorflag = FALSE;
    int mpack_errno1, mpack_errno2;
    for (int incx = MIN_INCX; incx <= MAX_INCX; incx++) {
	for (int incy = MIN_INCY; incy < MAX_INCY; incy++) {
	    for (int n = MIN_N; n < MAX_N; n++) {
		for (int lda = max(1, n); lda < MAX_LDA; lda++) {
#if defined VERBOSE_TEST
		    printf("#n is %d, lda is %d incx %d incy %d uplo %s\n", n, lda, incx, incy, uplo);
#endif
		    COMPLEX_REF *A_ref = new COMPLEX_REF[matlen(lda, n)];
		    COMPLEX_REF *x_ref = new COMPLEX_REF[veclen(n, incx)];
		    COMPLEX_REF *y_ref = new COMPLEX_REF[veclen(n, incy)];
		    COMPLEX *A = new COMPLEX[matlen(lda, n)];
		    COMPLEX *x = new COMPLEX[veclen(n, incx)];
		    COMPLEX *y = new COMPLEX[veclen(n, incy)];

		    for (int iter = 0; iter < MAX_ITER; iter++) {
			set_random_vector(A_ref, A, matlen(lda, n));
			set_random_vector(x_ref, x, veclen(n, incx));
			set_random_vector(y_ref, y, veclen(n, incy));

			mpack_errno = 0; blas_errno = 0;
#if defined ___MPACK_BUILD_WITH_MPFR___
			zsymv_f77(uplo, &n, &alpha_ref, A_ref, &lda, x_ref, &incx, &beta_ref, y_ref, &incy);
			mpack_errno1 = blas_errno;
#else
			Csymv(uplo, n, alpha_ref, A_ref, lda, x_ref, incx, beta_ref, y_ref, incy);
			mpack_errno1 = mpack_errno;
#endif
			Csymv(uplo, n, alpha, A, lda, x, incx, beta, y, incy);
			mpack_errno2 = mpack_errno;

#if defined VERBOSE_TEST
			printf("errno: mpack %d, ref %d\n", mpack_errno1, mpack_errno2);
#endif
			if (mpack_errno1 != mpack_errno2) {
			    printf("error in Mxerbla!!\n");
			}
			REAL_REF diff = infnorm(y_ref, y, veclen(n, incy), 1);
			if (diff > EPSILON) {
			    printf("error: "); printnum(diff); printf("\n");
			    errorflag = TRUE;
			}
			if (maxdiff < diff)
			    maxdiff = diff;
#if defined VERBOSE_TEST
			printf("max error: "); printnum(maxdiff); printf("\n");
#endif
		    }
		    delete[]A_ref;
		    delete[]x_ref;
		    delete[]y_ref;
		    delete[]x;
		    delete[]y;
		    delete[]A;
		}
	    }
	}
    }
    if (errorflag == TRUE) {
        printf("*** Testing Csymv failed ***\n");
	exit(1);
    }
}

void Csymv_test2(const char *uplo)
{
    COMPLEX_REF alpha_ref, beta_ref;
    COMPLEX alpha, beta;

//alpha=*, beta=*
    set_random_number(alpha_ref, alpha);
    set_random_number(beta_ref, beta);
    Csymv_test3(uplo, alpha_ref, beta_ref, alpha, beta);

//a=0, b=0;
    alpha_ref = 0.0; beta_ref = 0.0;
    alpha = 0.0; beta = 0.0;
    Csymv_test3(uplo, alpha_ref, beta_ref, alpha, beta);

//a=1, b=0;
    alpha_ref = 1.0; beta_ref = 0.0;
    alpha = 1.0; beta = 0.0;
    Csymv_test3(uplo, alpha_ref, beta_ref, alpha, beta);

//a=0, b=1;
    alpha_ref = 0.0; beta_ref = 1.0;
    alpha = 0.0; beta = 1.0;
    Csymv_test3(uplo, alpha_ref, beta_ref, alpha, beta);

//a=1, b=1;
    alpha_ref = 1.0; beta_ref = 1.0;
    alpha = 1.0; beta = 1.0;
    Csymv_test3(uplo, alpha_ref, beta_ref, alpha, beta);

//a=*, b=0;
    set_random_number(alpha_ref, alpha);
    beta_ref = 0.0; beta = 0.0;
    Csymv_test3(uplo, alpha_ref, beta_ref, alpha, beta);

//a=*, b=1;
    set_random_number(alpha_ref, alpha);
    beta_ref = 1.0; beta = 1.0;
    Csymv_test3(uplo, alpha_ref, beta_ref, alpha, beta);

//a=0, b=*;
    alpha_ref = 0.0;
    alpha = 0.0;
    set_random_number(beta_ref, beta);
    Csymv_test3(uplo, alpha_ref, beta_ref, alpha, beta);

//a=1, b=*;
    alpha_ref = 1.0;
    alpha = 1.0;
    set_random_number(beta_ref, beta);
    Csymv_test3(uplo, alpha_ref, beta_ref, alpha, beta);
}

void Csymv_test()
{
    Csymv_test2("U");
    Csymv_test2("L");
}

int main(int argc, char *argv[])
{
    printf("*** Testing Csymv start ***\n");
    Csymv_test();
    printf("*** Testing Csymv successful ***\n");
    return (0);
}
