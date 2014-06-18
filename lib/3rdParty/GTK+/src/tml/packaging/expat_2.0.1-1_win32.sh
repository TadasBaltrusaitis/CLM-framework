# This is a shell script that calls functions and scripts from
# tml@iki.fi's personal work environment. It is not expected to be
# usable unmodified by others, and is included only for reference.

MOD=expat
VER=2.0.1
REV=1
ARCH=win32

THIS=${MOD}_${VER}-${REV}_${ARCH}

RUNZIP=${MOD}_${VER}-${REV}_${ARCH}.zip
DEVZIP=${MOD}-dev_${VER}-${REV}_${ARCH}.zip

# We use a string of hex digits to make it more evident that it is
# just a hash value and not supposed to be relevant at end-user
# machines.
HEX=`echo $THIS | md5sum | cut -d' ' -f1`
TARGET=c:/devel/target/$HEX

usestable
usemsvs6

(

set -x

# Don't let libtool do its relinking dance. Don't know how relevant
# this is, but it doesn't hurt anyway.

sed -e 's/need_relink=yes/need_relink=no # no way --tml/' <conftools/ltmain.sh >conftools/ltmain.temp && mv conftools/ltmain.temp conftools/ltmain.sh &&

patch -p0 <<\EOF &&
--- Makefile.in
+++ Makefile.in
@@ -117,7 +117,7 @@
 COMPILE = $(CC) $(INCLUDES) $(CFLAGS) $(DEFS) $(CPPFLAGS)
 CXXCOMPILE = $(CXX) $(INCLUDES) $(CXXFLAGS) $(DEFS) $(CPPFLAGS)
 LTCOMPILE = $(LIBTOOL) $(LTFLAGS) --mode=compile $(COMPILE)
-LINK_LIB = $(LIBTOOL) $(LTFLAGS) --mode=link $(COMPILE) -no-undefined $(VSNFLAG) -rpath $(libdir) $(LDFLAGS) -o $@
+LINK_LIB = $(LIBTOOL) $(LTFLAGS) --mode=link $(COMPILE) -no-undefined $(VSNFLAG) -rpath $(libdir) $(LDFLAGS) -o $@ -export-symbols lib/libexpat.def
 LINK_EXE = $(LIBTOOL) $(LTFLAGS) --mode=link $(COMPILE) $(LDFLAGS) -o $@
 LINK_CXX_EXE = $(LIBTOOL) $(LTFLAGS) --mode=link $(CXXCOMPILE) $(LDFLAGS) -o $@
 
EOF

CC='gcc -mthreads' LDFLAGS='-Wl,--enable-auto-image-base' CFLAGS=-O2 ./configure --prefix=c:/devel/target/$HEX --disable-static &&

make install &&

cp lib/libexpat.def /devel/target/$HEX/lib &&

(cd /devel/target/$HEX/lib && lib.exe -machine:IX86 -def:libexpat.def -out:expat.lib) &&

rm -f /tmp/$RUNZIP /tmp/$DEVZIP &&

(cd /devel/target/$HEX &&
zip /tmp/$RUNZIP bin/libexpat-1.dll &&
zip /tmp/$DEVZIP bin/xmlwf.exe &&
zip -r -D /tmp/$DEVZIP include &&
zip /tmp/$DEVZIP lib/{libexpat.dll.a,libexpat.def,expat.lib} &&
zip -r -D /tmp/$DEVZIP man
)

) 2>&1 | tee /devel/src/tml/packaging/$THIS.log

(cd /devel && zip /tmp/$DEVZIP src/tml/packaging/$THIS.{sh,log}) &&
manifestify /tmp/$RUNZIP /tmp/$DEVZIP
