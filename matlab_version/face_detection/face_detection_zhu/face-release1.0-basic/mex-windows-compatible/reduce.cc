#include <math.h>
#include <assert.h>
#include <string.h>
#include "mex.h"

#define	round(x)	((x-floor(x))>0.5 ? ceil(x) : floor(x))

// reduce(im) resizes im to half its size, using a 5-tap binomial filter for anti-aliasing
// (see Burt & Adelson's Laplacian Pyramid paper)

// reduce each column
// result is transposed, so we can apply it twice for a complete reduction
void reduce1dtran(double *src, int sheight, double *dst, int dheight, 
		  int width, int chan) {
  // resize each column of each color channel
  //bzero(dst, chan*width*dheight*sizeof(double));
  memset(dst, 0, chan*width*dheight*sizeof(double));
  int y;
  double *s, *d;

  for (int c = 0; c < chan; c++) {
    for (int x = 0; x < width; x++) {
      s  = src + c*width*sheight + x*sheight;
      d  = dst + c*dheight*width + x;

      // First row
      *d = s[0]*.6875 + s[1]*.2500 + s[2]*.0625;      

      for (y = 1; y < dheight-2; y++) {	
	s += 2;
	d += width;
	*d = s[-2]*0.0625 + s[-1]*.25 + s[0]*.375 + s[1]*.25 + s[2]*.0625;
      }

      // Last two rows
      s += 2;
      d += width;
      if (dheight*2 <= sheight) {
	*d = s[-2]*0.0625 + s[-1]*.25 + s[0]*.375 + s[1]*.25 + s[2]*.0625;
      } else {
	*d = s[1]*.3125 + s[0]*.3750 + s[-1]*.2500 + s[-2]*.0625;
      }
      s += 2;
      d += width;
      *d = s[0]*.6875 + s[-1]*.2500 + s[-2]*.0625;
    }
  }
}

// main function
// takes a double color image and a scaling factor
// returns resized image
mxArray *reduce(const mxArray *mxsrc) {
  double *src = (double *)mxGetPr(mxsrc);
  const int *sdims = mxGetDimensions(mxsrc);
  if (mxGetNumberOfDimensions(mxsrc) != 3 || 
      mxGetClassID(mxsrc) != mxDOUBLE_CLASS)
    mexErrMsgTxt("Invalid input");  

  int ddims[3];
  ddims[0] = (int)round(sdims[0]*.5);
  ddims[1] = (int)round(sdims[1]*.5);
  ddims[2] = sdims[2];
  mxArray *mxdst = mxCreateNumericArray(3, ddims, mxDOUBLE_CLASS, mxREAL);
  double *dst = (double *)mxGetPr(mxdst);

  double *tmp = (double *)mxCalloc(ddims[0]*sdims[1]*sdims[2], sizeof(double));
  reduce1dtran(src, sdims[0], tmp, ddims[0], sdims[1], sdims[2]);
  reduce1dtran(tmp, sdims[1], dst, ddims[1], ddims[0], sdims[2]);
  mxFree(tmp);

  return mxdst;
}

// matlab entry point
// dst = resize(src, scale)
// image should be color with double values
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) { 
  if (nrhs != 1)
    mexErrMsgTxt("Wrong number of inputs"); 
  if (nlhs != 1)
    mexErrMsgTxt("Wrong number of outputs");
  plhs[0] = reduce(prhs[0]);
}



