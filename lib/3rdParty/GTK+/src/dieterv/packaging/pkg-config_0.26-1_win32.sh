# This is a shell script that calls functions and scripts from
# dieterv@optionexplicit's personal work environment. It is not expected to be
# usable unmodified by others, and is included only for reference.

MOD=pkg-config
VER=0.26
REV=1
ARCH=win32

THIS=${MOD}_${VER}-${REV}_${ARCH}

RUNZIP=${MOD}_${VER}-${REV}_${ARCH}.zip
DEVZIP=${MOD}-dev_${VER}-${REV}_${ARCH}.zip

HEX=`echo $THIS | md5sum | cut -d' ' -f1`
TARGET=c:/devel/target/$HEX

usedev

(

set -x

# https://bugs.freedesktop.org/show_bug.cgi?id=17053
patch --verbose -p1 --fuzz=0 <<'EOF' &&
commit e6f6c6a18dbe80f5a4804358a18dabc6220b6d8a
Author: Dieter Verfaillie <dieterv@optionexplicit.be>
Date:   Thu Apr 7 09:57:42 2011 +0200

    Revert "Print out \r\n on windows, not just \n"
    
    This reverts commit 25e8ca84acd7fc604fbc59213587887d5119d51a.

diff --git a/main.c b/main.c
index 8fd56ed..f87187d 100644
--- a/main.c
+++ b/main.c
@@ -752,11 +752,7 @@ main (int argc, char **argv)
     }
 
   if (need_newline)
-#ifdef G_OS_WIN32
-    printf ("\r\n");
-#else
     printf ("\n");
-#endif
 
   return 0;
 }
EOF

# Don't do any relinking and don't use any wrappers in libtool.
sed -e 's/need_relink=yes/need_relink=no # no way --tml/' \
    -e 's/wrappers_required=yes/wrappers_required=no # no thanks --tml/' \
    <ltmain.sh >ltmain.temp && mv ltmain.temp ltmain.sh

GLIB=`latest --arch=${ARCH} glib`
GETTEXT_RUNTIME=`latest --arch=${ARCH} gettext-runtime`
PKG_CONFIG=/opt/local/bin/pkg-config.sh

PATH=/devel/dist/$ARCH/$GLIB/bin:/devel/dist/$ARCH/$GETTEXT_RUNTIME/bin:$PATH
PKG_CONFIG_PATH=/devel/dist/$ARCH/$GLIB/lib/pkgconfig

CC='gcc -mthreads' \
CPPFLAGS="`$PKG_CONFIG --cflags glib-2.0` -I/devel/dist/$ARCH/$GETTEXT_RUNTIME/include" \
LDFLAGS="`$PKG_CONFIG --libs glib-2.0` -L/devel/dist/$ARCH/$GETTEXT_RUNTIME/lib" \
CFLAGS=-O2 \
./configure --disable-static --prefix=/devel/target/$HEX &&
make -j3 install &&

rm -f /tmp/$RUNZIP /tmp/$DEVZIP

cd /devel/target/$HEX && 
zip /tmp/$RUNZIP bin/pkg-config.exe &&
zip /tmp/$DEVZIP share/man/man1/pkg-config.1 share/aclocal/pkg.m4

) 2>&1 | tee /devel/src/dieterv/packaging/$THIS.log

(cd /devel && zip /tmp/$DEVZIP src/dieterv/packaging/$THIS.{sh,log}) &&
manifestify /tmp/$RUNZIP /tmp/$DEVZIP
