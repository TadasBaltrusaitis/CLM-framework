# This is a shell script that calls functions and scripts from
# tml@iki.fi's personal work envíronment. It is not expected to be
# usable unmodified by others, and is included only for reference.

MOD=gdk-pixbuf
VER=2.24.0
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

DEPS=`latest --arch=${ARCH} zlib glib gettext-runtime pkg-config libpng`
GETTEXT_RUNTIME=`latest --arch=${ARCH} gettext-runtime`

PKG_CONFIG_PATH=
for D in $DEPS; do
    PATH=/devel/dist/${ARCH}/$D/bin:$PATH
    [ -d /devel/dist/${ARCH}/$D/lib/pkgconfig ] && PKG_CONFIG_PATH=/devel/dist/${ARCH}/$D/lib/pkgconfig:$PKG_CONFIG_PATH
done

LIBPNG=`latest --arch=${ARCH} libpng`
ZLIB=`latest --arch=${ARCH} zlib`

# Don't do any relinking and don't use any wrappers in libtool. It
# just causes trouble, IMHO.

sed -e 's/need_relink=yes/need_relink=no # no way --tml/' \
    -e 's/wrappers_required=yes/wrappers_required=no # no thanks --tml/' \
    <ltmain.sh >ltmain.temp && mv ltmain.temp ltmain.sh

lt_cv_deplibs_check_method='pass_all' \
CC='gcc -mthreads' \
CPPFLAGS="-I/devel/dist/${ARCH}/${LIBPNG}/include \
-I/devel/dist/${ARCH}/${ZLIB}/include \
-I/devel/dist/${ARCH}/${GETTEXT_RUNTIME}/include" \
LDFLAGS="-L/devel/dist/${ARCH}/${LIBPNG}/lib \
-L/devel/dist/${ARCH}/${ZLIB}/lib \
-L/devel/dist/${ARCH}/${GETTEXT_RUNTIME}/lib \
-Wl,--enable-auto-image-base" \
LIBS=-lintl \
LIBPNG=`pkg-config --libs libpng` \
CFLAGS=-O2 \
./configure \
--with-gdiplus \
--with-included-loaders \
--without-libjasper \
--enable-debug=yes \
--enable-explicit-deps=no \
--disable-gtk-doc \
--disable-static \
--prefix=$TARGET &&

PATH="$PWD/gdk-pixbuf/.libs:/devel/target/$HEX/bin:$PATH" make -j4 install &&

PATH="/devel/target/$HEX/bin:$PATH" gdk-pixbuf-query-loaders >$TARGET/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache &&

(echo '# Note: After adding a new separate gdk-pixbuf loader (for instance the svg one)' &&
    echo '# run gdk-pixbuf-query-loaders.exe redirecting its output into this file.' &&
    echo &&
    echo '# Note that the LoaderDir folder below does not name a folder that is' &&
    echo '# expected to exist. It was just a temporary directory used at build time.' &&
    echo &&
    cat $TARGET/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache ) >$TARGET/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache.temp &&
    mv $TARGET/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache.temp $TARGET/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache &&

rm -f /tmp/$RUNZIP /tmp/$DEVZIP &&

(cd $TARGET &&
zip /tmp/$RUNZIP bin/libgdk_pixbuf-2.0-0.dll &&
zip /tmp/$RUNZIP bin/gdk-pixbuf-query-loaders.exe &&
zip /tmp/$RUNZIP lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll lib/gdk-pixbuf-2.0/2.10.0/loaders.cache lib/gdk-pixbuf-2.0/2.10.0/loaders &&
zip -r -D /tmp/$RUNZIP share/locale &&

zip -r -D /tmp/$DEVZIP include/gdk-pixbuf-2.0 &&
zip -r -D /tmp/$DEVZIP lib/libgdk_pixbuf-2.0.dll.a lib/gdk_pixbuf-2.0.lib &&
zip -r -D /tmp/$DEVZIP lib/pkgconfig &&
zip /tmp/$DEVZIP bin/gdk-pixbuf-csource.exe &&
zip -r -D /tmp/$DEVZIP share/man share/gtk-doc &&
:) &&

:) 2>&1 | tee /devel/src/dieterv/packaging/$THIS.log

(cd /devel && zip /tmp/$DEVZIP src/dieterv/packaging/$THIS.{sh,log}) &&
manifestify /tmp/$RUNZIP /tmp/$DEVZIP
