# This is a shell script that calls functions and scripts from
# tml@iki.fi's personal work environment. It is not expected to be
# usable unmodified by others, and is included only for reference.

MOD=zlib
VER=1.2.5
REV=2
ARCH=win32

THIS=${MOD}_${VER}-${REV}_${ARCH}

RUNZIP=${THIS}.zip
DEVZIP=${MOD}-dev_${VER}-${REV}_${ARCH}.zip

HEX=`echo $THIS | md5sum | cut -d' ' -f1`
TARGET=c:/devel/target/$HEX

usedev

(

set -x

UPSTREAM_ZIP=/c/tmp/downloads/zlib125-dll.zip
T=/tmp/zlib-$$

rm -rf $T &&

mkdir -p $TARGET/{bin,include,lib} $T &&

cd $T
unzip $UPSTREAM_ZIP &&

dos2unix include/zlib.h &&
dos2unix include/zconf.h &&

patch -p0 <<'EOF'
--- include/zlib.h
+++ include/zlib.h
@@ -1556,44 +1556,20 @@
         inflateBackInit_((strm), (windowBits), (window), \
                                             ZLIB_VERSION, sizeof(z_stream))
 
+/* LFS conventions have no meaning on Windows. Looking for feature
+ * macros like _LARGEFILE64_SOURCE or _FILE_OFFSET_BITS on Windows is
+ * wrong. So make sure any such macros misguidedly defined by the
+ * user have no effect. Windows has large file support, but the
+ * official zlib DLL has not been built to provide the 64-bit offset
+ * APIs, sigh.  So we have just patched out the 64-bit offset API
+ * from this header file.
+ */
-/* provide 64-bit offset functions if _LARGEFILE64_SOURCE defined, and/or
- * change the regular functions to 64 bits if _FILE_OFFSET_BITS is 64 (if
- * both are true, the application gets the *64 functions, and the regular
- * functions are changed to 64 bits) -- in case these are set on systems
- * without large file support, _LFS64_LARGEFILE must also be true
- */
-#if defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
-   ZEXTERN gzFile ZEXPORT gzopen64 OF((const char *, const char *));
-   ZEXTERN z_off64_t ZEXPORT gzseek64 OF((gzFile, z_off64_t, int));
-   ZEXTERN z_off64_t ZEXPORT gztell64 OF((gzFile));
-   ZEXTERN z_off64_t ZEXPORT gzoffset64 OF((gzFile));
-   ZEXTERN uLong ZEXPORT adler32_combine64 OF((uLong, uLong, z_off64_t));
-   ZEXTERN uLong ZEXPORT crc32_combine64 OF((uLong, uLong, z_off64_t));
-#endif
-
-#if !defined(ZLIB_INTERNAL) && _FILE_OFFSET_BITS-0 == 64 && _LFS64_LARGEFILE-0
-#  define gzopen gzopen64
-#  define gzseek gzseek64
-#  define gztell gztell64
-#  define gzoffset gzoffset64
-#  define adler32_combine adler32_combine64
-#  define crc32_combine crc32_combine64
-#  ifdef _LARGEFILE64_SOURCE
-     ZEXTERN gzFile ZEXPORT gzopen64 OF((const char *, const char *));
-     ZEXTERN z_off_t ZEXPORT gzseek64 OF((gzFile, z_off_t, int));
-     ZEXTERN z_off_t ZEXPORT gztell64 OF((gzFile));
-     ZEXTERN z_off_t ZEXPORT gzoffset64 OF((gzFile));
-     ZEXTERN uLong ZEXPORT adler32_combine64 OF((uLong, uLong, z_off_t));
-     ZEXTERN uLong ZEXPORT crc32_combine64 OF((uLong, uLong, z_off_t));
-#  endif
-#else
    ZEXTERN gzFile ZEXPORT gzopen OF((const char *, const char *));
    ZEXTERN z_off_t ZEXPORT gzseek OF((gzFile, z_off_t, int));
    ZEXTERN z_off_t ZEXPORT gztell OF((gzFile));
    ZEXTERN z_off_t ZEXPORT gzoffset OF((gzFile));
    ZEXTERN uLong ZEXPORT adler32_combine OF((uLong, uLong, z_off_t));
    ZEXTERN uLong ZEXPORT crc32_combine OF((uLong, uLong, z_off_t));
-#endif
 
 /* hack for buggy compilers */
 #if !defined(ZUTIL_H) && !defined(NO_DUMMY_DECL)
--- include/zconf.h
+++ include/zconf.h
@@ -270,7 +270,31 @@
 #  endif
 #endif
 
+/* When a specific build of zlib is done on Windows, it is either a
+ * DLL or not. That build should have a specific corresponding zconf.h
+ * distributed. The zconf.h thus knows a priori whether the
+ * corresponding library was built as a DLL or not. Requiring the
+ * library user to define ZLIB_DLL when compiling his code, intending
+ * to link against the import library for such a DLL build, is
+ * silly. Instead just unconditionally define ZLIB_DLL here as the
+ * official Windows build of zlib is a DLL, period.
+ *
+ * Similarly, when a specific build of zlib is done on (32-bit)
+ * Windows, it either uses the WINAPI calling convention or not. A
+ * user of a prebuilt library can not choose later. So it is pointless
+ * to require the user to define ZLIB_WINAPI when compiling. Instead,
+ * just have a specific copy of this zconf.h that corresponds to that
+ * build of zlib. For the case of the official build, it doe not use WINAPI,
+ * so ignore any attempt by a misguided user to use it.
+ */
+
+#undef ZLIB_DLL
+#define ZLIB_DLL 1
+
+#undef ZLIB_WINAPI
+
 #if defined(WINDOWS) || defined(WIN32)
+   /* NOTE: Bogus. See above comment about ZLIB_DLL */
    /* If building or using zlib as a DLL, define ZLIB_DLL.
     * This is not mandatory, but it offers a little performance increase.
     */
@@ -283,6 +307,8 @@
 #      endif
 #    endif
 #  endif  /* ZLIB_DLL */
+
+   /* NOTE: Bogus. See above comment about ZLIB_WINAPI */
    /* If building or using zlib with the WINAPI/WINAPIV calling convention,
     * define ZLIB_WINAPI.
     * Caution: the standard ZLIB1.DLL is NOT compiled using ZLIB_WINAPI.
@@ -364,17 +390,24 @@
 #  include <sys/types.h>    /* for off_t */
 #endif
 
+/* LFS conventions have no meaning on Windows. Looking for feature
+ * macros like _LARGEFILE64_SOURCE or _FILE_OFFSET_BITS on Windows is
+ * wrong. So make sure any such macros misguidedly defined by the user
+ * have no effect. Windows has large file support, but the official zlib
+ * DLL has not been built to provide the 64-bit offset APIs, sigh.
+ */
+
 /* a little trick to accommodate both "#define _LARGEFILE64_SOURCE" and
  * "#define _LARGEFILE64_SOURCE 1" as requesting 64-bit operations, (even
  * though the former does not conform to the LFS document), but considering
  * both "#undef _LARGEFILE64_SOURCE" and "#define _LARGEFILE64_SOURCE 0" as
  * equivalently requesting no 64-bit operations
  */
-#if -_LARGEFILE64_SOURCE - -1 == 1
+#if !defined(_WIN32) && -_LARGEFILE64_SOURCE - -1 == 1
 #  undef _LARGEFILE64_SOURCE
 #endif
 
-#if defined(Z_HAVE_UNISTD_H) || defined(_LARGEFILE64_SOURCE)
+#if !defined(_WIN32) && (defined(Z_HAVE_UNISTD_H) || defined(_LARGEFILE64_SOURCE))
 #  include <unistd.h>       /* for SEEK_* and off_t */
 #  ifdef VMS
 #    include <unixio.h>     /* for off_t */
@@ -394,7 +426,7 @@
 #  define z_off_t long
 #endif
 
-#if defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
+#if !defined(_WIN32) && (defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0)
 #  define z_off64_t off64_t
 #else
 #  define z_off64_t z_off_t
EOF

pexports zlib1.dll >zlib.def &&
dlltool --input-def zlib.def --dllname zlib1.dll --output-lib libz.dll.a &&
cp zlib1.dll $TARGET/bin &&
cp lib/zdll.lib libz.dll.a zlib.def $TARGET/lib &&
cp include/zlib.h include/zconf.h $TARGET/include &&

rm -f /tmp/$RUNZIP /tmp/$DEVZIP &&
(cd /devel/target/$HEX &&
zip /tmp/$RUNZIP bin/zlib1.dll &&
zip -r -D /tmp/$DEVZIP lib include
)

) 2>&1 | tee /devel/src/tml/packaging/$THIS.log

(cd /devel && zip /tmp/$DEVZIP src/tml/packaging/$THIS.{sh,log}) &&
manifestify /tmp/$RUNZIP /tmp/$DEVZIP
