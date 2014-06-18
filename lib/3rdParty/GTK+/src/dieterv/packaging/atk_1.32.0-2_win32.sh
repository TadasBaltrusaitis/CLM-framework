# This is a shell script that calls functions and scripts from
# dieterv@optionexplicit.be's personal work environment. It is not expected to be
# usable unmodified by others, and is included only for reference.

MOD=atk
VER=1.32.0
REV=2
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

# export missing symbols, backported from atk-1.33.6
patch --verbose -p0 --fuzz=0 <<'EOF' &&
--- atk/atk.symbols
+++ atk/atk.symbols
@@ -70,6 +70,8 @@
 	atk_hypertext_get_n_links
 	atk_hypertext_get_type
 	atk_hyperlink_state_flags_get_type
+	atk_hyperlink_impl_get_type
+	atk_hyperlink_impl_get_hyperlink
 	atk_image_get_image_description
 	atk_image_get_image_locale
 	atk_image_get_image_position
@@ -177,6 +179,7 @@
 	atk_streamable_content_get_n_mime_types
 	atk_streamable_content_get_stream
 	atk_streamable_content_get_type
+	atk_streamable_content_get_uri
 	atk_table_add_column_selection
 	atk_table_add_row_selection
 	atk_table_get_caption
EOF

DEPS=`latest --arch=${ARCH} zlib gettext-runtime glib`

GETTEXT_RUNTIME=`latest --arch=${ARCH} gettext-runtime`

for D in $DEPS; do
    PATH="/devel/dist/${ARCH}/$D/bin:$PATH"
    PKG_CONFIG_PATH=/devel/dist/${ARCH}/$D/lib/pkgconfig:$PKG_CONFIG_PATH
done

lt_cv_deplibs_check_method='pass_all' \
CC='gcc -mtune=pentium3 -mthreads' \
CPPFLAGS="-I/devel/dist/${ARCH}/${GETTEXT_RUNTIME}/include" \
LDFLAGS="-L/devel/dist/${ARCH}/${GETTEXT_RUNTIME}/lib \
-Wl,--enable-auto-image-base" \
CFLAGS=-O2 \
./configure --disable-gtk-doc --disable-static --prefix=/devel/target/$HEX

(cd atk; make atkmarshal.h atkmarshal.c) &&
PATH=/devel/target/$HEX/bin:.libs:$PATH make install &&

./atk-zip.sh &&

mv /tmp/${MOD}-${VER}.zip /tmp/$RUNZIP &&
mv /tmp/${MOD}-dev-${VER}.zip /tmp/$DEVZIP

) 2>&1 | tee /devel/src/dieterv/packaging/$THIS.log

(cd /devel && zip /tmp/$DEVZIP src/dieterv/packaging/$THIS.{sh,log}) &&
manifestify /tmp/$RUNZIP /tmp/$DEVZIP
