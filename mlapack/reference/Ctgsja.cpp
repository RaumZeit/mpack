/*
 * Copyright (c) 2008-2010
 *      Nakata, Maho
 *      All rights reserved.
 *
 *  $Id: Ctgsja.cpp,v 1.4 2010/08/07 04:48:32 nakatamaho Exp $ 
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
/*
Copyright (c) 1992-2007 The University of Tennessee.  All rights reserved.

$COPYRIGHT$

Additional copyrights may follow

$HEADER$

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer. 
  
- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer listed
  in this license in the documentation and/or other materials
  provided with the distribution.
  
- Neither the name of the copyright holders nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.
  
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT  
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT  
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include <mblas.h>
#include <mlapack.h>

#define MTRUE 1
#define MFALSE 0

void Ctgsja(const char *jobu, const char *jobv, const char *jobq, INTEGER m,
	    INTEGER p, INTEGER n, INTEGER k, INTEGER l, COMPLEX * A,
	    INTEGER lda, COMPLEX * B, INTEGER ldb, REAL tola,
	    REAL tolb, REAL * alpha, REAL * beta, COMPLEX * u, INTEGER ldu, COMPLEX * v, INTEGER ldv, COMPLEX * q, INTEGER ldq, COMPLEX * work, INTEGER * ncycle, INTEGER * info)
{
    INTEGER i, j;
    REAL a1, b1, a3, b3;
    COMPLEX a2, b2;
    REAL csq, csu, csv;
    COMPLEX snq;
    REAL rwk;
    COMPLEX snu, snv;
    REAL gamma;
    INTEGER initq, initu, initv, wantq, upper;
    REAL error, ssmin;
    INTEGER wantu, wantv;
    INTEGER kcycle;
    REAL Zero = 0.0, One = 1.0;

//Decode and test the input parameters
    initu = Mlsame(jobu, "I");
    wantu = initu || Mlsame(jobu, "U");
    initv = Mlsame(jobv, "I");
    wantv = initv || Mlsame(jobv, "V");
    initq = Mlsame(jobq, "I");
    wantq = initq || Mlsame(jobq, "Q");
    *info = 0;
    if (!(initu || wantu || Mlsame(jobu, "N"))) {
	*info = -1;
    } else if (!(initv || wantv || Mlsame(jobv, "N"))) {
	*info = -2;
    } else if (!(initq || wantq || Mlsame(jobq, "N"))) {
	*info = -3;
    } else if (m < 0) {
	*info = -4;
    } else if (p < 0) {
	*info = -5;
    } else if (n < 0) {
	*info = -6;
    } else if (lda < max((INTEGER) 1, m)) {
	*info = -10;
    } else if (ldb < max((INTEGER) 1, p)) {
	*info = -12;
    } else if (ldu < 1 || (wantu && ldu < m)) {
	*info = -18;
    } else if (ldv < 1 || (wantv && ldv < p)) {
	*info = -20;
    } else if (ldq < 1 || (wantq && ldq < n)) {
	*info = -22;
    }
    if (*info != 0) {
	Mxerbla("Ctgsja", -(*info));
	return;
    }
//Initialize U, V and Q, if necessary
    if (initu) {
	Claset("Full", m, m, Zero, One, &u[0], ldu);
    }
    if (initv) {
	Claset("Full", p, p, Zero, One, &v[0], ldv);
    }
    if (initq) {
	Claset("Full", n, n, Zero, One, &q[0], ldq);
    }
//Loop until convergence
    upper = MFALSE;
    for (kcycle = 1; kcycle <= 40; kcycle++) {
	upper = !upper;
	for (i = 0; i < l - 1; i++) {
	    for (j = i + 1; j <= l; j++) {
		a1 = Zero;
		a2 = Zero;
		a3 = Zero;
		if (k + i <= m) {
		    a1 = A[k + i + (n - l + i) * lda].real();
		}
		if (k + j <= m) {
		    a3 = A[k + j + (n - l + j) * lda].real();
		}
		b1 = B[i + (n - l + i) * ldb].real();
		b3 = B[j + (n - l + j) * ldb].real();
		if (upper) {
		    if (k + i <= m) {
			a2 = A[k + i + (n - l + j) * lda];
		    }
		    b2 = B[i + (n - l + j) * ldb];
		} else {
		    if (k + j <= m) {
			a2 = A[k + j + (n - l + i) * lda];
		    }
		    b2 = B[j + (n - l + i) * ldb];
		}
		Clags2(&upper, a1, a2, a3, b1, b2, b3, &csu, &snu, &csv, &snv, &csq, &snq);
//Update (K+I)-th and (K+J)-th rows of matrix A: U'*A
		if (k + j <= m) {
		    Crot(l, &A[k + j + (n - l + 1) * lda], lda, &A[k + i + (n - l + 1) * lda], lda, csu, conj(snu));
		}
//Update I-th and J-th rows of matrix B: V'*B
		Crot(l, &B[j + (n - l + 1) * ldb], ldb, &B[i + (n - l + 1) * ldb], ldb, csv, conj(snv));
//Update (N-L+I)-th and (N-L+J)-th columns of matrices
//A and B: A*Q and B*Q
		Crot(min(k + l, m), &A[(n - l + j) * lda], 1, &A[(n - l + i) * lda], 1, csq, snq);
		Crot(l, &B[(n - l + j) * ldb + 1], 1, &B[(n - l + i) * ldb + 1], 1, csq, snq);
		if (upper) {
		    if (k + i <= m) {
			A[k + i + (n - l + j) * lda] = Zero;
		    }
		    B[i + (n - l + j) * ldb] = Zero;
		} else {
		    if (k + j <= m) {
			A[k + j + (n - l + i) * lda] = Zero;
		    }
		    B[j + (n - l + i) * ldb] = Zero;
		}
//Ensure that the diagonal elements of A and B are real.
		if (k + i <= m) {
		    A[k + i + (n - l + i) * lda] = A[k + i + (n - l + i) * lda].real();
		}
		if (k + j <= m) {
		    A[k + j + (n - l + j) * lda] = A[k + j + (n - l + j) * lda].real();
		}
		B[i + (n - l + i) * ldb] = B[i + (n - l + i) * ldb].real();
		B[j + (n - l + j) * ldb] = B[j + (n - l + j) * ldb].real();
//Update unitary matrices U, V, Q, if desired.
		if (wantu && k + j <= m) {
		    Crot(m, &u[(k + j) * ldu + 1], 1, &u[(k + i) * ldu + 1], 1, csu, snu);
		}
		if (wantv) {
		    Crot(p, &v[j * ldv + 1], 1, &v[i * ldv + 1], 1, csv, snv);
		}
		if (wantq) {
		    Crot(n, &q[(n - l + j) * ldq + 1], 1, &q[(n - l + i) * ldq + 1], 1, csq, snq);
		}
	    }
	}
	if (!upper) {
//The matrices A13 and B13 were lower triangular at the start
//of the cycle, and are now upper triangular.
//Convergence test: test the parallelism of the corresponding
//rows of A and B.
	    error = Zero;
	    for (i = 0; i < min(l, m - k); i++) {
		Ccopy(l - i + 1, &A[k + i + (n - l + i) * lda], lda, &work[0], 1);
		Ccopy(l - i + 1, &B[i + (n - l + i) * ldb], ldb, &work[l + 1], 1);
		Clapll(l - i + 1, &work[0], 1, &work[l + 1], 1, &ssmin);
		error = max(error, ssmin);
	    }
	    if (abs(error) <= min(tola, tolb)) {
		goto L50;
	    }
	}
    }
//The algorithm has not converged after MAXIT cycles.
    *info = 1;
    goto L100;
  L50:
//If ERROR <= MIN(TOLA,TOLB), then the algorithm has converged.
//Compute the generalized singular value pairs (ALPHA, BETA), and
//set the triangular matrix R to array A.
    for (i = 0; i < k; i++) {
	alpha[i] = One;
	beta[i] = Zero;
    }
    for (i = 0; i < min(l, m - k); i++) {
	a1 = A[k + i + (n - l + i) * lda].real();
	b1 = B[i + (n - l + i) * ldb].real();
	if (a1 != Zero) {
	    gamma = b1 / a1;
	    if (gamma < Zero) {
		CRscal(l - i + 1, -One, &B[i + (n - l + i) * ldb], ldb);
		if (wantv) {
		    CRscal(p, -One, &v[i * ldv + 1], 1);
		}
	    }
	    Rlartg(abs(gamma), One, &beta[k + i], &alpha[k + i], &rwk);
	    if (alpha[k + i] >= beta[k + i]) {
		CRscal(l - i + 1, One / alpha[k + i], &A[k + i + (n - l + i) * lda], lda);
	    } else {
		CRscal(l - i + 1, One / beta[k + i], &B[i + (n - l + i) * ldb], ldb);
		Ccopy(l - i + 1, &B[i + (n - l + i) * ldb], ldb, &A[k + i + (n - l + i) * lda], lda);
	    }
	} else {
	    alpha[k + i] = Zero;
	    beta[k + i] = One;
	    Ccopy(l - i + 1, &B[i + (n - l + i) * ldb], ldb, &A[k + i + (n - l + i) * lda], lda);
	}
    }
//Post-assignment
    for (i = m + 1; i <= k + l; i++) {
	alpha[i] = Zero;
	beta[i] = One;
    }
    if (k + l < n) {
	for (i = k + l + 1; i <= n; i++) {
	    alpha[i] = Zero;
	    beta[i] = Zero;
	}
    }
  L100:
    *ncycle = kcycle;
    return;
}
