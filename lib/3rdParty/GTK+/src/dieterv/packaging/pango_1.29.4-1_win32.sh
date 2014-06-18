# This is a shell script that calls functions and scripts from
# tml@iki.fi's personal work environment. It is not expected to be
# usable unmodified by others, and is included only for reference.

MOD=pango
VER=1.29.4
REV=1
ARCH=win32

THIS=${MOD}_${VER}-${REV}_${ARCH}

RUNZIP=${MOD}_${VER}-${REV}_${ARCH}.zip
DEVZIP=${MOD}-dev_${VER}-${REV}_${ARCH}.zip

HEX=`echo $THIS | md5sum | cut -d' ' -f1`
TARGET=/devel/target/$HEX

usedev
usewinsdk52

(

set -x

DEPS=`latest --arch=${ARCH} gettext-runtime gettext-tools glib pkg-config zlib libpng pixman cairo expat fontconfig freetype`
GETTEXT_RUNTIME=`latest --arch=${ARCH} gettext-runtime`

PKG_CONFIG_PATH=/dummy
for D in $DEPS; do
    PKG_CONFIG_PATH=/devel/dist/${ARCH}/$D/lib/pkgconfig:$PKG_CONFIG_PATH
    PATH=/devel/dist/${ARCH}/$D/bin:$PATH
done

lt_cv_deplibs_check_method='pass_all' \
CC='gcc -mthreads' \
LDFLAGS="-L/devel/dist/${ARCH}/${GETTEXT_RUNTIME}/lib \
-Wl,--enable-auto-image-base" \
CFLAGS=-O2 \
./configure --enable-debug=yes --disable-gtk-doc --without-x --prefix=/devel/target/$HEX --enable-explicit-deps=no --with-included-modules=yes &&

PATH=/devel/target/$HEX/bin:.libs:$PATH make install &&

./pango-zip.sh &&

cd $TARGET &&

zip /tmp/${MOD}-dev-${VER}.zip bin/pango-view.exe &&
zip /tmp/${MOD}-dev-${VER}.zip share/man/man1/*.1 &&

mv /tmp/${MOD}-${VER}.zip /tmp/$RUNZIP &&
mv /tmp/${MOD}-dev-${VER}.zip /tmp/$DEVZIP &&

: ) 2>&1 | tee /devel/src/dieterv/packaging/$THIS.log

(cd /devel && zip /tmp/$DEVZIP src/dieterv/packaging/$THIS.{sh,log}) &&
manifestify /tmp/$RUNZIP /tmp/$DEVZIP
