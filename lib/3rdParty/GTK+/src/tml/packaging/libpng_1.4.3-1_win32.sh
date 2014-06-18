# This is a shell script that calls functions and scripts from
# tml@iki.fi's personal work environment. It is not expected to be
# usable unmodified by others, and is included only for reference.

MOD=libpng
VER=1.4.3
REV=1
ARCH=win32

THIS=${MOD}_${VER}-${REV}_${ARCH}

RUNZIP=${THIS}.zip
DEVZIP=${MOD}-dev_${VER}-${REV}_${ARCH}.zip

HEX=`echo $THIS | md5sum | cut -d' ' -f1`
TARGET=c:/devel/target/$HEX

ZLIB=`latest --arch=${ARCH} zlib`

usedev
usemsvs6

(

set -x

# Avoid the silly "relink" stuff in libtool
sed -e 's/need_relink=yes/need_relink=no # no way --tml/' <ltmain.sh >ltmain.temp && mv ltmain.temp ltmain.sh

# Avoid using ld --version-script, doesn't seem to work?
sed -e 's/grep version-script/grep no-thanks-version-script/' <configure > configure.temp && mv configure.temp configure

patch -p0 <<'EOF' &&
--- Makefile.in
+++ Makefile.in
@@ -1285,7 +1285,7 @@
 # do evil things to libpng to cause libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@ to be used
 install-exec-hook:
 	cd $(DESTDIR)$(bindir); rm -f libpng-config
-	cd $(DESTDIR)$(bindir); $(LN_S) $(PNGLIB_BASENAME)-config libpng-config
+	-cd $(DESTDIR)$(bindir); $(LN_S) $(PNGLIB_BASENAME)-config libpng-config
 	@set -x;\
 	cd $(DESTDIR)$(libdir);\
 	for ext in a la so so.@PNGLIB_MAJOR@@PNGLIB_MINOR@.@PNGLIB_RELEASE@ sl dylib; do\
EOF

lt_cv_deplibs_check_method='pass_all' CC='gcc -mtune=pentium3 -mthreads -mms-bitfields' CPPFLAGS="-I /devel/dist/${ARCH}/${ZLIB}/include" LDFLAGS="-L/devel/dist/${ARCH}/${ZLIB}/lib -Wl,--enable-auto-image-base" CFLAGS=-O2 ./configure --disable-static --without-binconfigs --prefix=$TARGET &&
make install &&

rm -f /tmp/$RUNZIP /tmp/$DEVZIP &&

(cd /devel/target/$HEX &&
zip /tmp/$RUNZIP bin/libpng14-14.dll &&
zip -r -D /tmp/$DEVZIP include  &&
zip /tmp/$DEVZIP lib/libpng14.dll.a &&
pexports bin/libpng14-14.dll >lib/libpng.def &&
lib -machine:X86 -def:lib/libpng.def -name:libpng14-14.dll -out:lib/libpng.lib &&
zip /tmp/$DEVZIP lib/libpng.def lib/libpng.lib &&
zip /tmp/$DEVZIP lib/pkgconfig/libpng*.pc &&
zip -r -D /tmp/$DEVZIP share/man &&

:

)

) 2>&1 | tee /devel/src/tml/packaging/$THIS.log &&

(cd /devel && zip /tmp/$DEVZIP src/tml/packaging/$THIS.{sh,log}) &&
manifestify /tmp/$RUNZIP /tmp/$DEVZIP &&

:
