#include "mex.h"
#include "cv_src/cv.h"

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	int i, j, oi, oj, k;

	int imw, imh;
	int tpw, tph;
	int resw, resh;

	int outw, outh; // real size of output (taking into account 'shape')
	int offw, offh;

	double* im;
	double* tp;
	double* res;

	IplImage* cvIm;
	IplImage* cvTp;
	IplImage* cvRes;

	int shapeTypeCharSize = 6;
	char* shapeTypeChar;
	int shapeType = 1; 
	/* 1, full
	   2, valid
	   3, same
	   */

	float* cvImP;
	float* cvTpP;
	float* cvResP;

	if( nrhs < 2 ) {
		mexErrMsgTxt("result = normxcorr2Mex(TEMPLATE, IMAGE)");
	} else if( !( mxIsDouble(prhs[0])&&mxIsDouble(prhs[1])) ) {
		mexErrMsgTxt("IMAGE and TEMPLATE must be of type double");
	} else if( nrhs > 2 && !mxIsChar(prhs[2]) ) {
		mexErrMsgTxt("SHAPE parameter must be a character string");
	}

	shapeTypeChar = (char*)malloc(shapeTypeCharSize);
	if( nrhs>2 ) {
		mxGetString( prhs[2], shapeTypeChar, shapeTypeCharSize );
		for( int ci=0; ci<strlen(shapeTypeChar); ci++ )
			shapeTypeChar[ci] = tolower(shapeTypeChar[ci]);

		if( !strcmp(shapeTypeChar, "full") )
			shapeType = 1;
		else if( !strcmp(shapeTypeChar, "valid") )
			shapeType = 2;
		else if( !strcmp(shapeTypeChar, "same") )
			shapeType = 3;
		else
			shapeType = 0;
	}

	free(shapeTypeChar);
	if( !shapeType )
		mexErrMsgTxt("unknown shape type");

	im = mxGetPr(prhs[1]);
	imw = mxGetN(prhs[1]);
	imh = mxGetM(prhs[1]);

	tp = mxGetPr(prhs[0]);
	tpw = mxGetN(prhs[0]);
	tph = mxGetM(prhs[0]);

	resw = imw-tpw+1;
	resh = imh-tph+1;

	if( resw<=0 || resh<=0 ) {
		mexErrMsgTxt("size(image) < size(template)");
	} 

	cvIm = cvCreateImage(cvSize(imw,imh), IPL_DEPTH_32F, 1);
	cvTp = cvCreateImage(cvSize(tpw,tph), IPL_DEPTH_32F, 1);
	cvRes = cvCreateImage(cvSize(resw,resh), IPL_DEPTH_32F, 1);

	cvImP = (float*)cvIm->imageData;
	cvTpP = (float*)cvTp->imageData;
	cvResP = (float*)cvRes->imageData;

	if( shapeType==1 ) { //full
		outw = imw+tpw-1;
		outh = imh+tph-1;
		offw = tpw-1;
		offh = tph-1;
	} else if( shapeType==2 ) { //valid
		outw = resw;
		outh = resh;
		offw = 0;
		offh = 0;
	} else if( shapeType==3 ) { // same
		outw = imw;
		outh = imh;
		offw = ceil((float)(tpw-1)/2.0f);
		offh = ceil((float)(tph-1)/2.0f);
	}

	plhs[0] = mxCreateDoubleMatrix( outh, outw, mxREAL );
	res = mxGetPr(plhs[0]);

	// we don't need to worry about alignment since we're using 32f

	k = 0;
	for( i=0; i<imw; i++ ) {
		for( j=0; j<imh; j++ ) {
			cvImP[ j*imw+i ] = im[ k ];
			k++;
		}
	}

	k = 0;
	for( i=0; i<tpw; i++ ) {
		for( j=0; j<tph; j++ ) {
			cvTpP[ j*tpw+i ] = tp[ k ];
			k++;
		}
	}

	cvMatchTemplate( cvIm, cvTp, cvRes, CV_TM_CCOEFF_NORMED );

	for( i=0, oi=offw; i<resw; i++, oi++ ) {
		for( j=0, oj=offh; j<resh; j++, oj++ ) {
			res[ oi*outh + oj ] = cvResP[ j*resw + i ];
		}
	}

	cvReleaseImage(&cvIm);
	cvReleaseImage(&cvTp);
	cvReleaseImage(&cvRes);
}