/* Code written by Lorenzo Torresani and Shoumen Saha */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "mat.h"
#include "mex.h"


#define IN 
#define OUT
#define MYDEBUG 0


static mxArray * McomputeH(int nargout_,
                           const mxArray * Uc,
                           const mxArray * Vc,
                           const mxArray * E_z,
                           const mxArray * E_zz,
                           const mxArray * RR);


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if (nlhs > 1) {
        mexErrMsgTxt(
            "Run-time Error: File: computeh Line: 1 Column:"
            " 1 The function \"computeh\" was called with"
            " more than the declared number of outputs (1).");
    }
    if (nrhs > 5) {
        mexErrMsgTxt(
            "Run-time Error: File: computeh Line: 1 Column:"
            " 1 The function \"computeh\" was called with"
            " more than the declared number of inputs (5).");
    }
    plhs[0] = McomputeH(nlhs, prhs[0], prhs[1], prhs[2], prhs[3], prhs[4]);
}



double ** getMxArray(IN const mxArray *mxArr, 
                     OUT int *rows, 
                     OUT int *cols) 
{
  int i, j, numDim;
  double *arrDataPt, **matrix;

  if((numDim = mxGetNumberOfDimensions(mxArr)) != 2)
  {
    printf("Input matrix has %d dimensions, not 2!\n", numDim);
    return NULL;
  }

  *rows = mxGetM(mxArr);
  *cols = mxGetN(mxArr);

  if(((matrix = (double **) malloc(sizeof(double *)*(*rows))) == NULL) ||
     ((matrix[0] = (double *) malloc(sizeof(double)*(*rows)*(*cols))) == NULL))
    return NULL;
  
  for(i=1; i<(*rows); i++)
    matrix[i] = matrix[0] + i*(*cols);

  arrDataPt = mxGetPr(mxArr);
  for(j=0; j<(*cols); j++)
    for(i=0; i<(*rows); i++)
      matrix[i][j] = *(arrDataPt ++);

  return matrix;
}


mxArray * putMxArray(double *img, int rows, int cols/*, char *nameImg*/)
{
	int i, j;
	double *ptr;
	mxArray *mxArr;

	mxArr  = mxCreateDoubleMatrix(rows, cols, mxREAL);

  ptr = mxGetPr(mxArr);

	for(j=0; j<cols; j++)
		for(i=0; i<rows; i++)
			*(ptr ++) = img[i*cols + j];
	
  /*mxSetPr(mxArr, data);*/
	/*mxSetName(mxArr, nameImg);*/

	return mxArr;
}



double ** createMatrix(IN int rows, 
                       IN int cols) 
{
  int i;
  double **matrix;

  if(((matrix = (double **) malloc(sizeof(double *)*rows)) == NULL) ||
     ((matrix[0] = (double *) malloc(sizeof(double)*rows*cols)) == NULL))
    return NULL;
  
  for(i=1; i<rows; i++)
    matrix[i] = matrix[0] + i*cols;


  memset(matrix[0], 0, sizeof(double)*rows*cols);

  return matrix;
}


int ** createIntMatrix(IN int rows, 
                       IN int cols) 
{
  int i;
  int **matrix;

  if(((matrix = (int **) malloc(sizeof(int *)*rows)) == NULL) ||
     ((matrix[0] = (int *) malloc(sizeof(int)*rows*cols)) == NULL))
    return NULL;
  
  for(i=1; i<rows; i++)
    matrix[i] = matrix[0] + i*cols;


  memset(matrix[0], 0, sizeof(int)*rows*cols);

  return matrix;
}


void freeMatrix(IN  double **matrix)
{
  free(*matrix);
  free(matrix);
}

void freeIntMatrix(IN  int **matrix)
{
  free(*matrix);
  free(matrix);
}


void IO_MLabCreateFile(char *fileName)
{
	MATFile *fp;

	fp = matOpen(fileName, "w");

	matClose(fp);
}


void IO_MLabWriteDoubleImg(char *fileName, char *nameImg, double *img, int rows, int cols)
{
	MATFile *fp;
	int i, j, dims[2];
	double *ptr;
	mxArray *mxArr;

  dims[0] = rows;
  dims[1] = cols;
	fp = matOpen(fileName, "u");

	mxArr  = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);

	ptr = mxGetPr(mxArr);
	for(j=0; j<cols; j++)
		for(i=0; i<rows; i++)
			*(ptr ++) = img[i*cols + j];
	
	/*mxSetName(mxArr, nameImg);
	matPutArray(fp, mxArr);*/
  matPutVariable(fp, nameImg, mxArr);

	matClose(fp);

	mxDestroyArray(mxArr);
}


void kron(IN int rowsA,
          IN int colsA,
          IN double **A,
          IN int rowsB,
          IN int colsB,
          IN double **B,
          IN int rowsC,
          IN int colsC,
          OUT double **C)
{
  int i, j, k, l;

  if ((rowsC != (rowsA*rowsB)) || (colsC != (colsA*colsB)))
  {
    printf("kron: incompatible matrix size\n");
    return;
  }

  for(i=0; i<rowsA; i++)  
    for(k=0; k<rowsB; k++)
      for(j=0; j<colsA; j++)
        for(l=0; l<colsB; l++)
          C[rowsB*i+k][colsB*j+l] = A[i][j]*B[k][l];

}



double conjgradient(IN  int n,
                    IN  double **A, 
                    IN  double **b, 
                    IN  int i_max,      /* max # iterations */
                    IN  double epsilon, /* error tolerance */
                    IN OUT double **x)
{
  //my conjugate gradient solver for .5*x'*A*x -b'*x, based on the
  // tutorial by Jonathan Shewchuk 
  //FILE * fd;
  int i=0, numrecompute, row, col;
  double Ax, delta_old, delta_new, delta_0, **r, **d, **q, alpha, alpha_denom, beta;

  r = createMatrix(1, n);
  d = createMatrix(1, n);
  q = createMatrix(1, n);

  /* r = b - A*x */
  /* delta_new = r'*r */
  delta_new = 0;
  for (row=0; row<n; row++)
  {
    Ax = 0;
    for (col=0; col<n; col++)
      Ax += A[row][col]*x[0][col];

    d[0][row] = r[0][row] = b[0][row] - Ax;

    delta_new += r[0][row]*r[0][row];
  }

  delta_0 = delta_new;

  numrecompute = (int) sqrt((double) n);
  //  printf("numrecompute = %d\n",numrecompute);


  //fd = fopen("err.txt", "w");
  while ((i < i_max) && (delta_new > epsilon*epsilon*delta_0))
  {
    //printf("Step %d, delta_new = %f      \r",i,delta_new);
    //fprintf(fd, "%d %f\n", i, (float)delta_new);
  
    /* q = A*d */
    alpha_denom = 0;
    for (row=0; row<n; row++)
    {
      q[0][row] = 0;
      for (col=0; col<n; col++)
        q[0][row] += A[row][col]*d[0][col];

      alpha_denom += d[0][row]*q[0][row];
    }

    alpha = delta_new / alpha_denom;

    /* x += d*alpha */
    for (row=0; row<n; row++)
      x[0][row] += alpha*d[0][row];

    if (i % numrecompute == 0)
	  {
      /* r = b - A*x */
      for (row=0; row<n; row++)
      {
        Ax = 0;
        for (col=0; col<n; col++)
          Ax += A[row][col]*x[0][col];

        r[0][row] = b[0][row] - Ax;
      }	  
	  }
    else
    {
      /* r = r-q*alpha; */
      for (row=0; row<n; row++)
        r[0][row] -= alpha*q[0][row];
    }

    delta_old = delta_new;

    /* delta_new = r'*r */
    delta_new = 0;
    for (row=0; row<n; row++)
      delta_new += r[0][row]*r[0][row];

    beta = delta_new/delta_old;

    /* d = r + beta*d */
    for (row=0; row<n; row++)
      d[0][row] = r[0][row] + beta*d[0][row];

    i++;
  }
  //fclose(fd);

  freeMatrix(r);
  freeMatrix(d);
  freeMatrix(q);

  return delta_new;
}



double conjgradient2(IN  int n,
                     IN  double **A, 
                     IN  double **b, 
                     IN  int i_max,      /* max # iterations */
                     IN  double epsilon, /* error tolerance */
                     IN  int k,
                     IN OUT double **x)
{
  //my conjugate gradient solver for .5*x'*A*x -b'*x, based on the
  // tutorial by Jonathan Shewchuk  (or is it +b'*x?)
  //FILE * fd;
  int i=0, numrecompute, row, sparsity_offset, j, ind, col_offset;
  double Ax, delta_old, delta_new, delta_0, **r, **d, **q, alpha, alpha_denom, beta;

  r = createMatrix(1, n);
  d = createMatrix(1, n);
  q = createMatrix(1, n);

  sparsity_offset = n / (k+1);

  /* r = b - A*x */
  /* delta_new = r'*r */
  delta_new = 0;
  for (row=0; row<n; row++)
  {
    Ax = 0;
    
    col_offset = ((row%sparsity_offset)/3) * 3;
    for (j=0; j<(k+1); j++)
    {
      ind = col_offset + j*sparsity_offset;
      Ax += A[row][ind]*x[0][ind] + A[row][ind+1]*x[0][ind+1] + A[row][ind+2]*x[0][ind+2];
    }

    d[0][row] = r[0][row] = b[0][row] - Ax;

    delta_new += r[0][row]*r[0][row];
  }

  delta_0 = delta_new;

  numrecompute = (int) sqrt((double) n);
  //  printf("numrecompute = %d\n",numrecompute);


  //fd = fopen("err.txt", "w");
  while ((i < i_max) && (delta_new > epsilon*epsilon*delta_0))
  {
    //printf("Step %d, delta_new = %f      \r",i,delta_new);
    //fprintf(fd, "%d %f\n", i, (float)delta_new);
  
    /* q = A*d */
    alpha_denom = 0;
    for (row=0; row<n; row++)
    {
      q[0][row] = 0;

      col_offset = ((row%sparsity_offset)/3) * 3;
      for (j=0; j<(k+1); j++)
      {
        ind = col_offset + j*sparsity_offset;
        q[0][row] += A[row][ind]*d[0][ind] + A[row][ind+1]*d[0][ind+1] + A[row][ind+2]*d[0][ind+2];
      }

      alpha_denom += d[0][row]*q[0][row];
    }

    alpha = delta_new / alpha_denom;

    /* x += d*alpha */
    for (row=0; row<n; row++)
      x[0][row] += alpha*d[0][row];

    if (i % numrecompute == 0)
	  {
      /* r = b - A*x */
      for (row=0; row<n; row++)
      {
        Ax = 0;

        col_offset = ((row%sparsity_offset)/3) * 3;
        for (j=0; j<(k+1); j++)
        {
          ind = col_offset + j*sparsity_offset;
          Ax += A[row][ind]*x[0][ind] + A[row][ind+1]*x[0][ind+1] + A[row][ind+2]*x[0][ind+2];
        }

        r[0][row] = b[0][row] - Ax;
      }	  
	  }
    else
    {
      /* r = r-q*alpha; */
      for (row=0; row<n; row++)
        r[0][row] -= alpha*q[0][row];
    }

    delta_old = delta_new;

    /* delta_new = r'*r */
    delta_new = 0;
    for (row=0; row<n; row++)
      delta_new += r[0][row]*r[0][row];

    beta = delta_new/delta_old;

    /* d = r + beta*d */
    for (row=0; row<n; row++)
      d[0][row] = r[0][row] + beta*d[0][row];

    i++;
  }
  //fclose(fd);

  freeMatrix(r);
  freeMatrix(d);
  freeMatrix(q);

  return delta_new;
}



static mxArray * McomputeH(int nargout_,
                           const mxArray * Uc,
                           const mxArray * Vc,
                           const mxArray * E_z,
                           const mxArray * E_zz,
                           const mxArray * RR) {
	double **Ucc, **Vcc, **E_zc, **E_zzc, **RRc, **KK1c, **KK2c, **kk3c, **P_tc, **R_tc, **zz_hat_tc, **Tmp1c, **Tmp2c, **vecH_hatc=NULL,
         delta_err, epsilon;
	int tc, Fc, Nc, junk1, junk2, kc, ic, jc, ii, jj, kk, ll, i_max, **sparsityMap;

  mxArray * vecH_hat = NULL;

  mxArray * KK1 = NULL;
  mxArray * R_t = NULL;
  mxArray * P_t = NULL;
  mxArray * t = NULL;
  mxArray * KK2 = NULL;
  mxArray * F = NULL;
  mxArray * N = NULL;
  mxArray * k = NULL;


#if MYDEBUG
#define DEBUGFILENAME ".\\debug.mat"
  IO_MLabCreateFile(DEBUGFILENAME);
#endif

	Ucc = getMxArray(Uc, &Fc, &Nc);
#if MYDEBUG
    IO_MLabWriteDoubleImg(DEBUGFILENAME, "Ucc", Ucc[0], Fc, Nc);
#endif

	Vcc = getMxArray(Vc, &junk1, &junk2);
#if MYDEBUG
    IO_MLabWriteDoubleImg(DEBUGFILENAME, "Vcc", Vcc[0], Fc, Nc);
#endif

  E_zc = getMxArray(E_z, &kc, &junk1);
#if MYDEBUG
    IO_MLabWriteDoubleImg(DEBUGFILENAME, "E_zc", E_zc[0], kc, Fc);
#endif

  E_zzc = getMxArray(E_zz, &junk1, &junk2);
#if MYDEBUG
    IO_MLabWriteDoubleImg(DEBUGFILENAME, "E_zzc", E_zzc[0], kc*Fc, kc);
#endif
    
    
  RRc = getMxArray(RR, &junk1, &junk2);
#if MYDEBUG
    IO_MLabWriteDoubleImg(DEBUGFILENAME, "RRc", RRc[0], 2*Fc, 3);
#endif

	/*
	 * KK2 = zeros(3*N*(k+1), 3*N*(k+1)); 
	 */
	KK2c = createMatrix(3*Nc*(kc+1), 3*Nc*(kc+1));
    
  /*
   * KK3 = zeros(3*N, k+1);
   */
	kk3c = createMatrix(1,3*Nc*(kc+1));

	P_tc = createMatrix(1, 2*Nc);
  zz_hat_tc = createMatrix(kc+1, kc+1);
  R_tc = createMatrix(2, 3);
  KK1c = createMatrix(2*Nc, 3*Nc);
  Tmp1c = createMatrix(3*Nc, 3*Nc);
  Tmp2c = createMatrix(1, 3*Nc);
	for(tc=0; tc<Fc; tc++) 
  {    

    /*
     * P_t = [Uc(t,:); Vc(t,:)];
     */
    memset(P_tc[0], 0, sizeof(double)*2*Nc);     
    for(jc=0; jc<Nc; jc++)
	  {
      P_tc[0][2*jc] = Ucc[tc][jc];
	    P_tc[0][2*jc+1] = Vcc[tc][jc];
	  }
#if MYDEBUG
    IO_MLabWriteDoubleImg(DEBUGFILENAME, "P_tc", P_tc[0], 1, 2*Nc);
#endif

    /*
     * zz_hat_t = [1 E_z(:,t)'; E_z(:,t) E_zz((t-1)*k+1:t*k,:)];
     */
    memset(zz_hat_tc[0], 0, sizeof(double)*(kc+1)*(kc+1));     
    zz_hat_tc[0][0] = 1;
    for(jc=1; jc<kc+1; jc++)
    {
      zz_hat_tc[jc][0] = E_zc[jc-1][tc];
      zz_hat_tc[0][jc] = E_zc[jc-1][tc];
    }
    for(ic=1; ic<kc+1; ic++)
    {
      for(jc=1; jc<kc+1; jc++)
	    {
        zz_hat_tc[ic][jc] = E_zzc[tc*kc + (ic-1)][jc-1];
      }
	  }
#if MYDEBUG
    IO_MLabWriteDoubleImg(DEBUGFILENAME, "zz_hat_tc", zz_hat_tc[0], kc+1, kc+1);
#endif

    /*
     * R_t = [RR(t, :); RR(t+F, :)];
     */
    for(jc=0; jc<3; jc++)
	  {
      R_tc[0][jc] = RRc[tc][jc];
      R_tc[1][jc] = RRc[tc+Fc][jc];
    }

    /*
     * KK1 = kron(speye(N), R_t);
     */
    memset(KK1c[0], 0, sizeof(double)*2*Nc*3*Nc);     
    for(ic=0; ic<Nc; ic++)
	  {
      KK1c[ic*2][ic*3] = R_tc[0][0];
      KK1c[ic*2][ic*3+1] = R_tc[0][1];
      KK1c[ic*2][ic*3+2] = R_tc[0][2];
      KK1c[ic*2+1][ic*3] = R_tc[1][0];
      KK1c[ic*2+1][ic*3+1] = R_tc[1][1];
      KK1c[ic*2+1][ic*3+2] = R_tc[1][2];
    }
#if MYDEBUG
    IO_MLabWriteDoubleImg(DEBUGFILENAME, "KK1c", KK1c[0], 2*Nc, 3*Nc);
#endif

    /* computes KK1'*KK1 */
    memset(Tmp1c[0], 0, sizeof(double)*3*Nc*3*Nc);
    Tmp1c[0][0] = KK1c[0][0]*KK1c[0][0] + KK1c[1][0]*KK1c[1][0];
    Tmp1c[0][1] = KK1c[0][0]*KK1c[0][1] + KK1c[1][0]*KK1c[1][1];
    Tmp1c[0][2] = KK1c[0][0]*KK1c[0][2] + KK1c[1][0]*KK1c[1][2];


    Tmp1c[1][0] = KK1c[0][1]*KK1c[0][0] + KK1c[1][1]*KK1c[1][0];
    Tmp1c[1][1] = KK1c[0][1]*KK1c[0][1] + KK1c[1][1]*KK1c[1][1];
    Tmp1c[1][2] = KK1c[0][1]*KK1c[0][2] + KK1c[1][1]*KK1c[1][2];

    Tmp1c[2][0] = KK1c[0][2]*KK1c[0][0] + KK1c[1][2]*KK1c[1][0];
    Tmp1c[2][1] = KK1c[0][2]*KK1c[0][1] + KK1c[1][2]*KK1c[1][1];
    Tmp1c[2][2] = KK1c[0][2]*KK1c[0][2] + KK1c[1][2]*KK1c[1][2];
    for(ic=1; ic<Nc; ic++)
    {
      Tmp1c[ic*3][ic*3] =   Tmp1c[0][0];
      Tmp1c[ic*3][ic*3+1] = Tmp1c[0][1];
      Tmp1c[ic*3][ic*3+2] = Tmp1c[0][2];


      Tmp1c[ic*3+1][ic*3] =   Tmp1c[1][0];
      Tmp1c[ic*3+1][ic*3+1] = Tmp1c[1][1];
      Tmp1c[ic*3+1][ic*3+2] = Tmp1c[1][2];

      Tmp1c[ic*3+2][ic*3] =   Tmp1c[2][0];
      Tmp1c[ic*3+2][ic*3+1] = Tmp1c[2][1];
      Tmp1c[ic*3+2][ic*3+2] = Tmp1c[2][2];
    }
#if MYDEBUG
    IO_MLabWriteDoubleImg(DEBUGFILENAME, "Tmp1c", Tmp1c[0], 3*Nc, 3*Nc);
#endif

    /*
     * KK2 = KK2 + kron(zz_hat_t', KK1'*KK1);
     */ 

    /*for(ii=0; ii<(kc+1); ii++)  
     for(kk=0; kk<(3*Nc); kk++)
       for(jj=0; jj<(kc+1); jj++)
         for(ll=0; ll<(3*Nc); ll++)
           KK2c[(3*Nc)*ii+kk][(3*Nc)*jj+ll] += zz_hat_tc[ii][jj]*Tmp1c[kk][ll];*/
    for(ii=0; ii<(kc+1); ii++)  
     for(kk=0; kk<Nc; kk++)
       for(jj=0; jj<(kc+1); jj++)
       {
         KK2c[(3*Nc)*ii+kk*3][(3*Nc)*jj+kk*3] += zz_hat_tc[ii][jj]*Tmp1c[kk*3][kk*3];
         KK2c[(3*Nc)*ii+kk*3][(3*Nc)*jj+kk*3+1] += zz_hat_tc[ii][jj]*Tmp1c[kk*3][kk*3+1];
         KK2c[(3*Nc)*ii+kk*3][(3*Nc)*jj+kk*3+2] += zz_hat_tc[ii][jj]*Tmp1c[kk*3][kk*3+2];

         KK2c[(3*Nc)*ii+kk*3+1][(3*Nc)*jj+kk*3] += zz_hat_tc[ii][jj]*Tmp1c[kk*3+1][kk*3];
         KK2c[(3*Nc)*ii+kk*3+1][(3*Nc)*jj+kk*3+1] += zz_hat_tc[ii][jj]*Tmp1c[kk*3+1][kk*3+1];
         KK2c[(3*Nc)*ii+kk*3+1][(3*Nc)*jj+kk*3+2] += zz_hat_tc[ii][jj]*Tmp1c[kk*3+1][kk*3+2];

         KK2c[(3*Nc)*ii+kk*3+2][(3*Nc)*jj+kk*3] += zz_hat_tc[ii][jj]*Tmp1c[kk*3+2][kk*3];
         KK2c[(3*Nc)*ii+kk*3+2][(3*Nc)*jj+kk*3+1] += zz_hat_tc[ii][jj]*Tmp1c[kk*3+2][kk*3+1];
         KK2c[(3*Nc)*ii+kk*3+2][(3*Nc)*jj+kk*3+2] += zz_hat_tc[ii][jj]*Tmp1c[kk*3+2][kk*3+2];
       }
#if MYDEBUG
    IO_MLabWriteDoubleImg(DEBUGFILENAME, "KK2c", KK2c[0], 3*Nc*(kc+1), 3*Nc*(kc+1));
#endif
    

    /*
     * KK3 = KK3 + KK1'* P_t(:) * [1 E_z(:,t)'];
     */

    /* computes KK1'*P_t(:) */
    memset(Tmp2c[0], 0, sizeof(double)*3*Nc);
    for(ic=0; ic<Nc; ic++)
    {
      Tmp2c[0][3*ic] =   KK1c[2*ic][3*ic]*P_tc[0][2*ic] + KK1c[2*ic+1][3*ic]*P_tc[0][2*ic+1];
      Tmp2c[0][3*ic+1] = KK1c[2*ic][3*ic+1]*P_tc[0][2*ic] + KK1c[2*ic+1][3*ic+1]*P_tc[0][2*ic+1];
      Tmp2c[0][3*ic+2] = KK1c[2*ic][3*ic+2]*P_tc[0][2*ic] + KK1c[2*ic+1][3*ic+2]*P_tc[0][2*ic+1];
    }
#if MYDEBUG
    IO_MLabWriteDoubleImg(DEBUGFILENAME, "Tmp2c", Tmp2c[0], 1, 3*Nc);
#endif

    for(ic=0; ic<3*Nc; ic++)
    {
      /*KK3c[ic][0] = KK3c[ic][0] + Tmp2c[0][ic];*/
      kk3c[0][ic] = kk3c[0][ic] + Tmp2c[0][ic];

      for(jc=1; jc<(kc+1); jc++)
        /*KK3c[ic][jc] = KK3c[ic][jc] + Tmp2c[0][ic]*E_zc[jc-1][tc];*/
        kk3c[0][ic + jc*(3*Nc)] = kk3c[0][ic + jc*(3*Nc)] + Tmp2c[0][ic]*E_zc[jc-1][tc];
    }
#if MYDEBUG
    IO_MLabWriteDoubleImg(DEBUGFILENAME, "kk3c", kk3c[0], 1, 3*Nc*(kc+1));
#endif


    //printf("debug\n");

  }


  freeMatrix(Ucc);
  freeMatrix(Vcc);
  freeMatrix(E_zc);
  freeMatrix(E_zzc);
  freeMatrix(RRc);
  freeMatrix(KK1c);
  freeMatrix(P_tc);
  freeMatrix(R_tc);
  freeMatrix(zz_hat_tc);
  freeMatrix(Tmp1c);
  freeMatrix(Tmp2c);


  vecH_hatc = createMatrix(1, 3*Nc*(kc+1));
  for(ic=0; ic<3*Nc; ic++)
    vecH_hatc[0][ic] = (double) rand() / (double) RAND_MAX;

  epsilon = 0.000000001;
  i_max = 1000;
  
  //sparsityMap = createIntMatrix(3*Nc*(kc+1), 3*Nc*(kc+1)+1);
  //getSparsityMap(3*Nc*(kc+1), 3*Nc*(kc+1), KK2c, sparsityMap);

  //delta_err = conjgradient(3*Nc*(kc+1), KK2c, kk3c, i_max, epsilon, vecH_hatc);
  delta_err = conjgradient2(3*Nc*(kc+1), KK2c, kk3c, i_max, epsilon, kc, vecH_hatc);

  if (delta_err>0.01)
    printf("Large CG error\n");

  vecH_hat = mxCreateDoubleMatrix(3*Nc*(kc+1), 1, mxREAL);
  memcpy(mxGetPr(vecH_hat), vecH_hatc[0], sizeof(double)*3*Nc*(kc+1));

  freeMatrix(KK2c);
  freeMatrix(kk3c);
  freeMatrix(vecH_hatc);

  return vecH_hat;
}
