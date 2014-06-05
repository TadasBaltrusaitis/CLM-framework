mex -O resize.cc
mex -O reduce.cc
mex -O shiftdt.cc
mex -O features.cc

% use one of the following depending on your setup.
% 1 is fastest, 3 is slowest.
% If you are using a Windows machine, please use 3. 

% 1) multithreaded convolution using blas
% mex -O fconvblas.cc -lmwblas -o fconv
% 2) mulththreaded convolution without blas
% mex -O fconvMT.cc -o fconv
% 3) basic convolution, very compatible
mex -O fconv.cc

