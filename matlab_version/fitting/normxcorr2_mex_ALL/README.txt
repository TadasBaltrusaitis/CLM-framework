This software is taken verbatim from the OpenCV (http://sourceforge.net/projects/opencvlibrary/). 
I don't know who wrote the routine cvMatchTemplate, but it is exceptional code and deserves recognition. All I've done
is write a Matlab MEX interface, and provide the routine separately from the OpenCV library. My motivation
for doing this was simply to make it more accessible. The OpenCV is bloated and poorly documented -- two negatives
which make it confusing and inconvenient for many people to use, despite the fact it contains many excellent features.
It seems that providing this code meshes well with the OpenCV's aim for widespread use, but if I've 
violated the license, please indicate so.

Anyway, I hope this interface to the OpenCV code is beneficial to someone. I was frustrated by the lack of fast
NCC code available on the net -- many claim to have written fast routines, but don't readily share them. In my 
experience this far, the OpenCV routine is the fastest general purpose, exact way to do NCC ou there. There are
domain-dependent speed-ups, which I discuss at www.cs.ubc.ca/~deaton/remarks_ncc.html. You may find my remarks there 
useful.

I'd be interested to hear about your own NCC experiences,

Daniel Eaton
danieljameseaton@gmail.com