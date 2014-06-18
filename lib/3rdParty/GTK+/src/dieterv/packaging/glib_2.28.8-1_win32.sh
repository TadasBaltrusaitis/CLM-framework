# This is a shell script that calls functions and scripts from
# dieterv@optionexplicit.be's personal work environment. It is not expected to be
# usable unmodified by others, and is included only for reference.

MOD=glib
VER=2.28.8
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

# https://bugzilla.gnome.org/show_bug.cgi?id=654846
patch --verbose -p1 --fuzz=0 <<'EOF' &&
commit 0ab806379fb1e3a344e714cf2b49d5cd2157a2d4
Author: Dieter Verfaillie <dieterv@optionexplicit.be>
Date:   Mon Jul 18 14:20:18 2011 +0200

    Update glib-zip.in

diff --git a/glib-zip.in b/glib-zip.in
index b8ceb09..e2f41de 100755
--- a/glib-zip.in
+++ b/glib-zip.in
@@ -24,8 +24,8 @@ rm $ZIP
 zip $ZIP -@ <<EOF
 bin/gspawn-win${helperbits}-helper.exe
 bin/gspawn-win${helperbits}-helper-console.exe
-bin/libglib-2.0-@LT_CURRENT_MINUS_AGE@.dll
 bin/libgio-2.0-@LT_CURRENT_MINUS_AGE@.dll
+bin/libglib-2.0-@LT_CURRENT_MINUS_AGE@.dll
 bin/libgmodule-2.0-@LT_CURRENT_MINUS_AGE@.dll
 bin/libgobject-2.0-@LT_CURRENT_MINUS_AGE@.dll
 bin/libgthread-2.0-@LT_CURRENT_MINUS_AGE@.dll
@@ -37,26 +37,19 @@ zip -r -D $ZIP share/doc/glib-@GLIB_VERSION@
 
 rm $DEVZIP
 zip -r -D $DEVZIP -@ <<EOF
+bin/gdbus.exe
+bin/gio-querymodules.exe
+bin/glib-compile-schemas.exe
 bin/glib-genmarshal.exe
 bin/glib-gettextize
 bin/glib-mkenums
 bin/gobject-query.exe
+bin/gsettings.
+etc/bash_completion.d/gdbus-bash-completion.sh
+etc/bash_completion.d/gsettings-bash-completion.sh
+include/gio-win32-2.0
 include/glib-2.0
-lib/libglib-2.0.dll.a
-lib/glib-2.0.lib
-lib/glib-2.0.def
-lib/libgmodule-2.0.dll.a
-lib/gmodule-2.0.lib
-lib/gmodule-2.0.def
-lib/libgobject-2.0.dll.a
-lib/gobject-2.0.lib
-lib/gobject-2.0.def
-lib/libgthread-2.0.dll.a
-lib/gthread-2.0.lib
-lib/gthread-2.0.def
-lib/libgio-2.0.dll.a
-lib/gio-2.0.lib
-lib/gio-2.0.def
+lib/gio
 lib/glib-2.0
 lib/pkgconfig/glib-2.0.pc
 lib/pkgconfig/gmodule-2.0.pc
@@ -65,13 +58,34 @@ lib/pkgconfig/gobject-2.0.pc
 lib/pkgconfig/gthread-2.0.pc
 lib/pkgconfig/gio-2.0.pc
 lib/pkgconfig/gio-windows-2.0.pc
+lib/gio-2.0.def
+lib/gio-2.0.lib
+lib/glib-2.0.def
+lib/glib-2.0.lib
+lib/gmodule-2.0.def
+lib/gmodule-2.0.lib
+lib/gobject-2.0.def
+lib/gobject-2.0.lib
+lib/gthread-2.0.def
+lib/gthread-2.0.lib
+lib/libgio-2.0.dll.a
+lib/libglib-2.0.dll.a
+lib/libgmodule-2.0.dll.a
+lib/libgobject-2.0.dll.a
+lib/libgthread-2.0.dll.a
 share/aclocal/glib-2.0.m4
 share/aclocal/glib-gettext.m4
+share/aclocal/gsettings.m4
 share/glib-2.0
 share/gtk-doc/html
-share/man/man1/glib-mkenums.1
+share/man/man1/gdbus.1
+share/man/man1/gio-querymodules.1
+share/man/man1/glib-compile-schemas.1
 share/man/man1/glib-genmarshal.1
+share/man/man1/glib-gettextize.1
+share/man/man1/glib-mkenums.1
 share/man/man1/gobject-query.1
+share/man/man1/gsettings.1
 EOF
 
 zip -r $DEVZIP share/doc/glib-dev-@GLIB_VERSION@
EOF

DEPS=`latest --arch=${ARCH} gettext-runtime gettext-tools glib pkg-config`
GETTEXT_RUNTIME=`latest --arch=${ARCH} gettext-runtime`
ZLIB=`latest --arch=${ARCH} zlib`

PKG_CONFIG_PATH=/dummy
for D in $DEPS; do
    PATH=/devel/dist/${ARCH}/$D/bin:$PATH
    [ -d /devel/dist/${ARCH}/$D/lib/pkgconfig ] && PKG_CONFIG_PATH=/devel/dist/${ARCH}/$D/lib/pkgconfig:$PKG_CONFIG_PATH
done

lt_cv_deplibs_check_method='pass_all' \
CC='gcc -mthreads' \
CPPFLAGS="-I/devel/dist/${ARCH}/${GETTEXT_RUNTIME}/include \
-I/devel/dist/${ARCH}/${ZLIB}/include" \
LDFLAGS="-L/devel/dist/${ARCH}/${GETTEXT_RUNTIME}/lib -Wl,--enable-auto-image-base \
-L/devel/dist/${ARCH}/${ZLIB}/lib" \
CFLAGS=-O2 \
./configure \
--enable-silent-rules \
--disable-gtk-doc \
--prefix=$TARGET &&

(cd glib &&
make glibconfig.h.win32 &&
make glibconfig-stamp &&
mv glibconfig.h glibconfig.h.autogened &&
cp glibconfig.h.win32 glibconfig.h) &&
PATH="/devel/target/$HEX/bin:$PATH" make -j3 install &&

./glib-zip &&

mv /tmp/glib-$VER.zip /tmp/$RUNZIP &&
mv /tmp/glib-dev-$VER.zip /tmp/$DEVZIP

) 2>&1 | tee /devel/src/dieterv/packaging/$THIS.log

(cd /devel && zip /tmp/$DEVZIP src/dieterv/packaging/$THIS.{sh,log}) &&
manifestify /tmp/$RUNZIP /tmp/$DEVZIP
