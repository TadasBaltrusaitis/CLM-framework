# This is a shell script that calls functions and scripts from
# tml@iki.fi's personal work envíronment. It is not expected to be
# usable unmodified by others, and is included only for reference.

MOD=gtk+
VER=2.24.10
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

# Fix broken gtk/gtkmarshal.c file included with 2.24.10 tarball.
patch --verbose -p1 --fuzz=0 <<'EOF' &&
--- gtk+-2.24.10/gtk/gtkmarshal.c       2012-02-06 16:35:22 +0100
+++ gtk+-2.24.8/gtk/gtkmarshal.c        2011-11-10 18:43:53 +0100
@@ -6,7 +6,7 @@

 #ifdef G_ENABLE_DEBUG
 #define g_marshal_value_peek_boolean(v)  g_value_get_boolean (v)
-#define g_marshal_value_peek_char(v)     g_value_get_schar (v)
+#define g_marshal_value_peek_char(v)     g_value_get_char (v)
 #define g_marshal_value_peek_uchar(v)    g_value_get_uchar (v)
 #define g_marshal_value_peek_int(v)      g_value_get_int (v)
 #define g_marshal_value_peek_uint(v)     g_value_get_uint (v)
EOF

DEPS=`latest --arch=${ARCH} zlib gettext-runtime gettext-tools glib pkg-config atk cairo freetype fontconfig pango gdk-pixbuf libpng`
GETTEXT_RUNTIME=`latest --arch=${ARCH} gettext-runtime`

PKG_CONFIG_PATH=/dummy
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
CFLAGS=-O2  \
./configure \
--with-gdktarget=win32 \
--with-included-immodules \
--enable-debug=yes \
--enable-explicit-deps=no \
--disable-gtk-doc \
--disable-static \
--prefix=$TARGET &&

rm gtk/gtk.def &&

mkdir -p /devel/target/$HEX/lib &&

PATH="/devel/target/$HEX/bin:$PATH" make -j3 install &&

grep -v -E 'Automatically generated|Created by' <$TARGET/etc/gtk-2.0/gtk.immodules >$TARGET/etc/gtk-2.0/gtk.immodules.temp &&
    mv $TARGET/etc/gtk-2.0/gtk.immodules.temp $TARGET/etc/gtk-2.0/gtk.immodules &&

./gtk-zip.sh &&

# Package also the gtk-update-icon-cache.exe.manifest
(cd $TARGET && zip /tmp/${MOD}-dev-${VER}.zip bin/gtk-update-icon-cache.exe.manifest) &&

mv /tmp/${MOD}-${VER}.zip /tmp/$RUNZIP &&
mv /tmp/${MOD}-dev-${VER}.zip /tmp/$DEVZIP

) 2>&1 | tee /devel/src/dieterv/packaging/$THIS.log

(cd /devel && zip /tmp/$DEVZIP src/dieterv/packaging/$THIS.{sh,log}) &&
manifestify /tmp/$RUNZIP /tmp/$DEVZIP
