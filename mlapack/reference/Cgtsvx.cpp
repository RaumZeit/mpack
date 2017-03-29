/*
 * Copyright (c) 2008-2010
 *      Nakata, Maho
 *      All rights reserved.
 *
 *  $Id: Cgtsvx.cpp,v 1.3 2010/08/07 04:48:32 nakatamaho Exp $ 
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

void Cgtsvx(const char *fact, const char *trans, INTEGER n, INTEGER nrhs, COMPLEX * dl, COMPLEX * d, COMPLEX * du,
	    COMPLEX * dlf, COMPLEX * df, COMPLEX * duf,
	    COMPLEX * du2, INTEGER * ipiv, COMPLEX * B, INTEGER ldb, COMPLEX * x, INTEGER ldx, REAL * rcond, REAL * ferr, REAL * berr, COMPLEX * work, REAL * rwork, INTEGER * info)
{
    char norm;
    REAL anorm;
    INTEGER nofact;
    INTEGER notran;
    REAL Zero = 0.0;

    *info = 0;
    nofact = Mlsame(fact, "N");
    notran = Mlsame(trans, "N");
    if (!nofact && !Mlsame(fact, "F")) {
	*info = -1;
    } else if (!notran && !Mlsame(trans, "T") && !Mlsame(trans, "C")) {
	*info = -2;
    } else if (n < 0) {
	*info = -3;
    } else if (nrhs < 0) {
	*info = -4;
    } else if (ldb < max((INTEGER) 1, n)) {
	*info = -14;
    } else if (ldx < max((INTEGER) 1, n)) {
	*info = -16;
    }
    if (*info != 0) {
	Mxerbla("Cgtsvx", -(*info));
	return;
    }
    if (nofact) {
//Compute the LU factorization of A.
	Ccopy(n, &d[0], 1, &df[1], 1);
	if (n > 1) {
	    Ccopy(n - 1, &dl[1], 1, &dlf[1], 1);
	    Ccopy(n - 1, &du[1], 1, &duf[1], 1);
	}
	Cgttrf(n, &dlf[1], &df[1], &duf[1], &du2[1], &ipiv[1], info);
//Return if INFO is non-zero.
	if (*info > 0) {
	    *rcond = Zero;
	    return;
	}
    }
//Compute the norm of the matrix A.
    if (notran) {
	norm = '1';
    } else {
	norm = 'I';
    }
    anorm = Clangt((const char *) norm, n, &dl[1], &d[0], &du[1]);
//Compute the reciprocal of the condition number of A.
    Cgtcon((const char *) norm, n, &dlf[1], &df[1], &duf[1], &du2[1], &ipiv[1], anorm, rcond, &work[0], info);
//Compute the solution vectors X.
    Clacpy("Full", n, nrhs, &B[0], ldb, &x[0], ldx);
    Cgttrs(trans, n, nrhs, &dlf[1], &df[1], &duf[1], &du2[1], &ipiv[1], &x[0], ldx, info);
//Use iterative refinement to improve the computed solutions and
//compute error bounds and backward error estimates for them.
    Cgtrfs(trans, n, nrhs, &dl[1], &d[0], &du[1], &dlf[1], &df[1], &duf[1], &du2[1], &ipiv[1], &B[0], ldb, &x[0], ldx, &ferr[1]
	   , &berr[1], &work[0], &rwork[1], info);
//Set INFO = N+1 if the matrix is singular to working precision.
    if (*rcond < Rlamch("Epsilon")) {
	*info = n + 1;
    }
    return;
}
