# This is a shell script that calls functions and scripts from
# tml@iki.fi's personal work environment. It is not expected to be
# usable unmodified by others, and is included only for reference.

MOD=cairo
VER=1.10.2
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

# Fix A1 format win32 surfaces, https://bugs.freedesktop.org/show_bug.cgi?id=42739
patch --verbose -p1 --fuzz=0 <<'EOF' &&
diff --git a/src/cairo-win32-surface.c b/src/cairo-win32-surface.c
index 660aaba..8ebd185 100644
--- a/src/cairo-win32-surface.c
+++ b/src/cairo-win32-surface.c
@@ -545,6 +545,79 @@ _cairo_win32_surface_get_subimage (cairo_win32_surface_t  *surface,
     return CAIRO_STATUS_SUCCESS;
 }
 
+static const unsigned char mirror[256] = {
+  0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
+  0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
+  0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
+  0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
+  0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
+  0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
+  0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
+  0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
+  0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
+  0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
+  0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
+  0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
+  0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
+  0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
+  0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
+  0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
+  0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
+  0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
+  0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
+  0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
+  0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
+  0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
+  0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
+  0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
+  0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
+  0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
+  0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
+  0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
+  0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
+  0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
+  0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
+  0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
+};
+
+static void
+mirror_a1_bits (cairo_surface_t *image_surface)
+{
+    int w, h, stride, x, y;
+    unsigned char *data;
+
+    h = cairo_image_surface_get_height (image_surface);
+    stride = cairo_image_surface_get_stride (image_surface);
+    data = cairo_image_surface_get_data (image_surface);
+
+    for (y = 0; y < h; y++) {
+	for (x = 0; x < stride; x++) {
+	    *data = mirror[*data];
+	    data++;
+	}
+    }
+}
+
+static cairo_bool_t
+_cairo_win32_surface_is_gdi_format (cairo_surface_t *image_surface)
+{
+    return cairo_image_surface_get_format (image_surface) != CAIRO_FORMAT_A1;
+}
+
+static void
+_cairo_win32_surface_convert_from_gdi_format (cairo_surface_t *image_surface)
+{
+    if (cairo_image_surface_get_format (image_surface) == CAIRO_FORMAT_A1)
+	mirror_a1_bits (image_surface);
+}
+
+static void
+_cairo_win32_surface_convert_to_gdi_format (cairo_surface_t *image_surface)
+{
+    if (cairo_image_surface_get_format (image_surface) == CAIRO_FORMAT_A1)
+	mirror_a1_bits (image_surface);
+}
+
 static cairo_status_t
 _cairo_win32_surface_acquire_source_image (void                    *abstract_surface,
 					   cairo_image_surface_t  **image_out,
@@ -554,7 +627,7 @@ _cairo_win32_surface_acquire_source_image (void                    *abstract_sur
     cairo_win32_surface_t *local;
     cairo_status_t status;
 
-    if (surface->image) {
+    if (surface->image && _cairo_win32_surface_is_gdi_format (surface->image)) {
 	*image_out = (cairo_image_surface_t *)surface->image;
 	*image_extra = NULL;
 	return CAIRO_STATUS_SUCCESS;
@@ -566,6 +639,8 @@ _cairo_win32_surface_acquire_source_image (void                    *abstract_sur
     if (status)
 	return status;
 
+    _cairo_win32_surface_convert_from_gdi_format (local->image);
+
     *image_out = (cairo_image_surface_t *)local->image;
     *image_extra = local;
     return CAIRO_STATUS_SUCCESS;
@@ -593,7 +668,7 @@ _cairo_win32_surface_acquire_dest_image (void                    *abstract_surfa
     cairo_win32_surface_t *local = NULL;
     cairo_status_t status;
 
-    if (surface->image) {
+    if (surface->image && _cairo_win32_surface_is_gdi_format (surface->image)) {
 	GdiFlush();
 
 	*image_out = (cairo_image_surface_t *) surface->image;
@@ -611,6 +686,8 @@ _cairo_win32_surface_acquire_dest_image (void                    *abstract_surfa
     if (status)
 	return status;
 
+    _cairo_win32_surface_convert_from_gdi_format (local->image);
+
     *image_out = (cairo_image_surface_t *) local->image;
     *image_extra = local;
     *image_rect = *interest_rect;
@@ -630,6 +707,8 @@ _cairo_win32_surface_release_dest_image (void                    *abstract_surfa
     if (!local)
 	return;
 
+    _cairo_win32_surface_convert_from_gdi_format (local->image);
+
     if (!BitBlt (surface->dc,
 		 image_rect->x, image_rect->y,
 		 image_rect->width, image_rect->height,
@@ -1313,7 +1392,7 @@ _cairo_win32_surface_composite (cairo_operator_t	op,
 
 UNSUPPORTED:
     /* Fall back to image surface directly, if this is a DIB surface */
-    if (dst->image) {
+    if (dst->image && _cairo_win32_surface_is_gdi_format (dst->image)) {
 	GdiFlush();
 
 	return dst->image->backend->composite (op, pattern, mask_pattern,
EOF

# Reset clip region when writing fallback results , https://bugs.freedesktop.org/show_bug.cgi?id=42821
patch --verbose -p1 --fuzz=0 <<'EOF' &&
commit a02dd8ddc7dde30ba71e87ab5e90bf910b68da2b
Author: Alexander Larsson <alexl@redhat.com>
Date:   Fri Nov 11 16:15:31 2011 +0100

    win32: Reset clip in _cairo_win32_surface_release_dest_image
    
    Without this we were using leftover clip regions from e.g. show_glyphs
    which made the fallback drawing results disappear.

diff --git a/src/cairo-win32-surface.c b/src/cairo-win32-surface.c
index 660aaba..59305f1 100644
--- a/src/cairo-win32-surface.c
+++ b/src/cairo-win32-surface.c
@@ -707,6 +707,7 @@ _cairo_win32_surface_release_dest_image (void                    *abstract_surfa
     if (!local)
 	return;
 
+    _cairo_win32_surface_set_clip_region (surface, NULL);
     _cairo_win32_surface_convert_from_gdi_format (local->image);

     if (!BitBlt (surface->dc,
EOF

sed -e 's/need_relink=yes/need_relink=no # no way --tml/' <build/ltmain.sh >build/ltmain.temp && mv build/ltmain.temp build/ltmain.sh

# Avoid using "file" in libtool. Otherwise libtool won't create a
# shared library, and give the warning "Trying to link with static lib
# archive [...] But I can only do this if you have shared version of
# the library, which you do not appear to have."  I know what I am
# doing, I do want to link with a static libpixman-1.

sed -e 's!file /!dont-want-to-use-file!' <configure >configure.temp && mv configure.temp configure

DEPS=`latest --arch=${ARCH} gettext-runtime glib pkg-config pixman libpng fontconfig freetype`

PKG_CONFIG_PATH=/dummy
for D in $DEPS; do
    PATH=/devel/dist/${ARCH}/$D/bin:$PATH
    PKG_CONFIG_PATH=/devel/dist/${ARCH}/$D/lib/pkgconfig:$PKG_CONFIG_PATH
done

GETTEXT_RUNTIME=`latest --arch=${ARCH} gettext-runtime`
ZLIB=`latest --arch=${ARCH} zlib`

png_REQUIRES=libpng \
CC='gcc -mms-bitfields -mthreads' \
CPPFLAGS="-I/devel/dist/${ARCH}/${ZLIB}/include" \
LDFLAGS="-L/devel/dist/${ARCH}/${GETTEXT_RUNTIME}/lib \
-L/devel/dist/${ARCH}/${ZLIB}/lib" \
CFLAGS=-O2 \
./configure --disable-static --enable-ft=yes --prefix=/devel/target/$HEX &&

make -j3 zips &&

cp $MOD-$VER.zip /tmp/$RUNZIP &&
cp $MOD-dev-$VER.zip /tmp/$DEVZIP &&

cp -p src/cairo.def /devel/target/$HEX/lib &&

mkdir -p /devel/target/$HEX/share/doc/$THIS &&
cp -p COPYING COPYING-LGPL-2.1 COPYING-MPL-1.1 /devel/target/$HEX/share/doc/$THIS &&

cd /devel/target/$HEX &&

zip /tmp/$RUNZIP bin/libcairo-gobject-2.dll &&
zip /tmp/$RUNZIP bin/libcairo-script-interpreter-2.dll &&

zip /tmp/$DEVZIP lib/libcairo-gobject.dll.a &&
zip /tmp/$DEVZIP lib/libcairo-script-interpreter.dll.a &&

(cd lib && lib.exe -machine:X86 -def:cairo.def -out:cairo.lib) &&

zip /tmp/$DEVZIP lib/cairo.def lib/cairo.lib &&
zip -r -D /tmp/$RUNZIP share/doc/$THIS &&

# Don't depend on pixman
sed -e 's/ pixman-1 >= 0.18.4//' <lib/pkgconfig/cairo.pc >lib/pkgconfig/cairo.pc.temp && mv lib/pkgconfig/cairo.pc.temp lib/pkgconfig/cairo.pc &&
zip /tmp/$DEVZIP lib/pkgconfig/cairo.pc

) 2>&1 | tee /devel/src/dieterv/packaging/$THIS.log &&

(cd /devel && zip /tmp/$DEVZIP src/dieterv/packaging/$THIS.{sh,log}) &&
manifestify /tmp/$RUNZIP /tmp/$DEVZIP
