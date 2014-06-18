# This is a shell script that calls functions and scripts from
# tml@iki.fi's personal work environment. It is not expected to be
# usable unmodified by others, and is included only for reference.

MOD=gettext
VER=0.18.1.1
REV=2
ARCH=win32

THIS=${MOD}_${VER}-${REV}_${ARCH}

# Note that this build script is different from most of my build
# scripts, as it produces three sets of packages: "gettext-runtime"m
# "gettext-tools", and "gettext-examples". No packages called just "gettext".

RUNTIMERUNZIP=${MOD}-runtime_${VER}-${REV}_${ARCH}.zip
RUNTIMEDEVZIP=${MOD}-runtime-dev_${VER}-${REV}_${ARCH}.zip

TOOLSRUNZIP=${MOD}-tools_${VER}-${REV}_${ARCH}.zip
TOOLSDEVZIP=${MOD}-tools-dev_${VER}-${REV}_${ARCH}.zip

EXAMPLESRUNZIP=${MOD}-examples_${VER}-${REV}_${ARCH}.zip
EXAMPLESDEVZIP=${MOD}-examples-dev_${VER}-${REV}_${ARCH}.zip

HEX=`echo $THIS | md5sum | cut -d' ' -f1`
TARGET=c:/devel/target/$HEX

usedev
usemsvs9

(

set -x

WIN_ICONV=`latest --arch=${ARCH} win-iconv`

# Verify this from gettext-runtime/intl/Makefile.in
LTV_CURRENT_MINUS_AGE=8

patch -p0 --fuzz=0 <<'EOF' &&
--- gettext-runtime/gnulib-lib/stdio.in.h
+++ gettext-runtime/gnulib-lib/stdio.in.h
@@ -642,10 +642,6 @@
 # if (@GNULIB_PRINTF_POSIX@ && @REPLACE_PRINTF@) \
      || (@GNULIB_PRINTF@ && @REPLACE_STDIO_WRITE_FUNCS@ && @GNULIB_STDIO_H_SIGPIPE@)
 #  if defined __GNUC__
-#   if !(defined __cplusplus && defined GNULIB_NAMESPACE)
-/* Don't break __attribute__((format(printf,M,N))).  */
-#    define printf __printf__
-#   endif
 _GL_FUNCDECL_RPL_1 (__printf__, int,
                     (const char *format, ...)
                     __asm__ (@ASM_SYMBOL_PREFIX@
--- /dev/null
+++ gettext-runtime/intl/intl.def
@@ -0,0 +1,31 @@
+EXPORTS
+_nl_expand_alias
+_nl_msg_cat_cntr DATA
+bind_textdomain_codeset
+bindtextdomain
+dcgettext
+dcngettext
+dgettext
+dngettext
+gettext
+libintl_bind_textdomain_codeset
+libintl_bindtextdomain
+libintl_dcgettext
+libintl_dcngettext
+libintl_dgettext
+libintl_dngettext
+libintl_fprintf
+libintl_gettext
+libintl_ngettext
+libintl_printf
+libintl_set_relocation_prefix
+libintl_setlocale
+libintl_snprintf
+libintl_sprintf
+libintl_textdomain
+libintl_vfprintf
+libintl_vprintf
+libintl_vsnprintf
+libintl_vsprintf
+ngettext
+textdomain
--- gettext-runtime/intl/printf.c
+++ gettext-runtime/intl/printf.c
@@ -69,7 +69,7 @@
 #define STATIC static
 
 /* This needs to be consistent with libgnuintl.h.in.  */
-#if defined __NetBSD__ || defined __BEOS__ || defined __CYGWIN__ || defined __MINGW32__
+#if defined __NetBSD__ || defined __BEOS__ || defined __CYGWIN__
 /* Don't break __attribute__((format(printf,M,N))).
    This redefinition is only possible because the libc in NetBSD, Cygwin,
    mingw does not have a function __printf__.  */
--- gettext-runtime/intl/libgnuintl.h.in
+++ gettext-runtime/intl/libgnuintl.h.in
@@ -330,7 +330,7 @@
 
 #if !(defined printf && defined _GL_STDIO_H) /* don't override gnulib */
 #undef printf
-#if defined __NetBSD__ || defined __BEOS__ || defined __CYGWIN__ || defined __MINGW32__
+#if defined __NetBSD__ || defined __BEOS__ || defined __CYGWIN__
 /* Don't break __attribute__((format(printf,M,N))).
    This redefinition is only possible because the libc in NetBSD, Cygwin,
    mingw does not have a function __printf__.
--- gettext-runtime/intl/Makefile.in
+++ gettext-runtime/intl/Makefile.in
@@ -85,7 +85,7 @@
 CPPFLAGS = @CPPFLAGS@
 CFLAGS = @CFLAGS@ @CFLAG_VISIBILITY@
 LDFLAGS = @LDFLAGS@ $(LDFLAGS_@WOE32DLL@)
-LDFLAGS_yes = -Wl,--export-all-symbols
+LDFLAGS_yes = intl.def
 LDFLAGS_no =
 LIBS = @LIBS@
 
@@ -193,7 +193,7 @@
   version.$lo \
   osdep.$lo \
   intl-compat.$lo
-OBJECTS_RES_yes = libintl.res.$lo
+OBJECTS_RES_yes = libintl.res.o
 OBJECTS_RES_no =
 DISTFILES.common = Makefile.in \
 config.charset locale.alias ref-add.sin ref-del.sin export.h libintl.rc \
@@ -206,8 +206,9 @@
 libgnuintl.h_vms Makefile.vms libgnuintl.h.msvc-static \
 libgnuintl.h.msvc-shared Makefile.msvc
 
-all: all-@USE_INCLUDED_LIBINTL@
-all-yes: libintl.$la libintl.h charset.alias ref-add.sed ref-del.sed
+all: all-@USE_INCLUDED_LIBINTL@-@WOE32DLL@
+all-yes-no: libintl.$la libintl.h charset.alias ref-add.sed ref-del.sed
+all-yes-yes: intl.$la libintl.h charset.alias ref-add.sed ref-del.sed
 all-no: all-no-@BUILD_INCLUDED_LIBINTL@
 all-no-yes: libgnuintl.$la
 all-no-no:
@@ -199,6 +200,16 @@
 	  -rpath $(libdir) \
 	  -no-undefined
 
+intl.la: $(OBJECTS) $(OBJECTS_RES_@WOE32@)
+	$(LIBTOOL) --mode=link \
+	  $(CC) $(CPPFLAGS) $(CFLAGS) $(XCFLAGS) $(LDFLAGS) -o $@ \
+	  $(OBJECTS) @LTLIBICONV@ @INTL_MACOSX_LIBS@ $(LIBS) @LTLIBTHREAD@ @LTLIBC@ \
+	  -Wl,$(OBJECTS_RES_@WOE32@) -Wl,$(srcdir)/intl.def \
+	  -module -avoid-version \
+	  -rpath $(libdir) \
+	  -no-undefined
+	  cp intl.$la libintl.$la
+
 # Libtool's library version information for libintl.
 # Before making a gettext release, the gettext maintainer must change this
 # according to the libtool documentation, section "Library interface versions".
@@ -337,11 +348,11 @@
 # If you want to use the one which comes with this version of the
 # package, you have to use `configure --with-included-gettext'.
 install: install-exec install-data
-install-exec: all
+
+install-libintl-no:
 	if { test "$(PACKAGE)" = "gettext-runtime" || test "$(PACKAGE)" = "gettext-tools"; } \
 	   && test '@USE_INCLUDED_LIBINTL@' = yes; then \
-	  $(mkdir_p) $(DESTDIR)$(libdir) $(DESTDIR)$(includedir); \
-	  $(INSTALL_DATA) libintl.h $(DESTDIR)$(includedir)/libintl.h; \
+	  $(mkdir_p) $(DESTDIR)$(libdir); \
 	  $(LIBTOOL) --mode=install \
 	    $(INSTALL_DATA) libintl.$la $(DESTDIR)$(libdir)/libintl.$la; \
 	  if test "@RELOCATABLE@" = yes; then \
@@ -353,6 +364,31 @@
 	else \
 	  : ; \
 	fi
+
+install-libintl-yes:
+	if { test "$(PACKAGE)" = "gettext-runtime" || test "$(PACKAGE)" = "gettext-tools"; } \
+	   && test '@USE_INCLUDED_LIBINTL@' = yes; then \
+	  $(mkdir_p) $(DESTDIR)$(libdir) $(DESTDIR)$(libdir)/../bin; \
+	  $(LIBTOOL) --mode=install \
+	    $(INSTALL_DATA) intl.$la $(DESTDIR)$(libdir)/intl.$la; \
+	  if test "@RELOCATABLE@" = yes; then \
+	    dependencies=`sed -n -e 's,^dependency_libs=\(.*\),\1,p' < $(DESTDIR)$(libdir)/intl.la | sed -e "s,^',," -e "s,'\$$,,"`; \
+	  fi; \
+	  rm -f $(DESTDIR)$(libdir)/intl.$la; \
+	  mv $(DESTDIR)$(libdir)/intl.dll.a $(DESTDIR)$(libdir)/libintl.dll.a; \
+	  mv $(DESTDIR)$(libdir)/intl.dll $(DESTDIR)$(libdir)/../bin; \
+	else \
+	  : ; \
+	fi
+
+install-exec: all install-libintl-@WOE32DLL@
+	if { test "$(PACKAGE)" = "gettext-runtime" || test "$(PACKAGE)" = "gettext-tools"; } \
+	   && test '@USE_INCLUDED_LIBINTL@' = yes; then \
+	  $(mkdir_p) $(DESTDIR)$(libdir) $(DESTDIR)$(includedir); \
+	  $(INSTALL_DATA) libintl.h $(DESTDIR)$(includedir)/libintl.h; \
+	else \
+	  : ; \
+	fi
 	if test "$(PACKAGE)" = "gettext-tools" \
 	   && test '@USE_INCLUDED_LIBINTL@' = no \
 	   && test @GLIBC2@ != no; then \
@@ -395,6 +431,7 @@
 	else \
 	  : ; \
 	fi
+
 install-data: all
 	if test "$(PACKAGE)" = "gettext-tools"; then \
 	  $(mkdir_p) $(DESTDIR)$(gettextsrcdir); \
--- gettext-tools/gnulib-lib/clean-temp.c
+++ gettext-tools/gnulib-lib/clean-temp.c
@@ -66,9 +66,11 @@
 # endif
 #endif
 
+#ifndef _WIN64
 #ifndef uintptr_t
 # define uintptr_t unsigned long
 #endif
+#endif
 
 #if !GNULIB_FCNTL_SAFER
 /* The results of open() in this file are not used with fchdir,
--- gettext-tools/gnulib-lib/fstrcmp.c
+++ gettext-tools/gnulib-lib/fstrcmp.c
@@ -55,9 +55,11 @@
 #include "minmax.h"
 #include "xalloc.h"
 
+#ifndef _WIN64
 #ifndef uintptr_t
 # define uintptr_t unsigned long
 #endif
+#endif
 
 
 #define ELEMENT char
--- gettext-tools/gnulib-lib/gl_array_list.c
+++ gettext-tools/gnulib-lib/gl_array_list.c
@@ -55,9 +55,11 @@
 /* Checked size_t computations.  */
 #include "xsize.h"
 
+#ifndef _WIN64
 #ifndef uintptr_t
 # define uintptr_t unsigned long
 #endif
+#endif
 
 /* -------------------------- gl_list_t Data Type -------------------------- */
 
--- gettext-tools/gnulib-lib/gl_linkedhash_list.c
+++ gettext-tools/gnulib-lib/gl_linkedhash_list.c
@@ -55,9 +55,11 @@
 
 #include "xsize.h"
 
+#ifndef _WIN64
 #ifndef uintptr_t
 # define uintptr_t unsigned long
 #endif
+#endif
 
 #define WITH_HASHTABLE 1
 
--- gettext-tools/gnulib-lib/stdio.in.h
+++ gettext-tools/gnulib-lib/stdio.in.h
@@ -642,10 +642,6 @@
 # if (@GNULIB_PRINTF_POSIX@ && @REPLACE_PRINTF@) \
      || (@GNULIB_PRINTF@ && @REPLACE_STDIO_WRITE_FUNCS@ && @GNULIB_STDIO_H_SIGPIPE@)
 #  if defined __GNUC__
-#   if !(defined __cplusplus && defined GNULIB_NAMESPACE)
-/* Don't break __attribute__((format(printf,M,N))).  */
-#    define printf __printf__
-#   endif
 _GL_FUNCDECL_RPL_1 (__printf__, int,
                     (const char *format, ...)
                     __asm__ (@ASM_SYMBOL_PREFIX@
--- gettext-tools/gnulib-lib/tempname.c
+++ gettext-tools/gnulib-lib/tempname.c
@@ -54,6 +54,10 @@
 #include <stdint.h>
 #include <unistd.h>
 
+#ifdef _WIN32
+# include <direct.h>
+#endif
+
 #include <sys/stat.h>
 
 #if _LIBC
@@ -73,6 +73,10 @@
 # define __xstat64(version, file, buf) stat (file, buf)
 #endif
 
+#ifdef _WIN32
+# define mkdir(path,mode) _mkdir(path)
+#endif
+
 #if ! (HAVE___SECURE_GETENV || _LIBC)
 # define __secure_getenv getenv
 #endif
--- gettext-tools/src/write-java.c
+++ gettext-tools/src/write-java.c
@@ -30,6 +30,10 @@
 #include <stdio.h>
 #include <string.h>
 
+#ifdef _WIN32
+# include <direct.h>
+#endif
+
 #include <sys/stat.h>
 #if !defined S_ISDIR && defined S_IFDIR
 # define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
@@ -53,6 +53,10 @@
 # define S_IXUSR 00100
 #endif
 
+#ifdef _WIN32
+# define mkdir(path,mode) _mkdir(path)
+#endif
+
 #include "c-ctype.h"
 #include "error.h"
 #include "xerror.h"
--- gettext-tools/src/write-csharp.c
+++ gettext-tools/src/write-csharp.c
@@ -29,6 +29,10 @@
 #include <stdio.h>
 #include <string.h>
 
+#ifdef _WIN32
+# include <direct.h>
+#endif
+
 #include <sys/stat.h>
 #if !defined S_ISDIR && defined S_IFDIR
 # define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
@@ -70,6 +70,10 @@
 # define S_IXOTH (S_IXUSR >> 6)
 #endif
 
+#ifdef _WIN32
+# define mkdir(path,mode) _mkdir(path)
+#endif
+
 #include "c-ctype.h"
 #include "relocatable.h"
 #include "error.h"
EOF

lt_cv_deplibs_check_method='pass_all' \
CC='gcc -mthreads -D__USE_MINGW_ANSI_STDIO=0' \
CFLAGS=-O2 \
./configure \
--disable-static \
--disable-java \
--disable-native-java \
--enable-relocatable \
--disable-acl \
--disable-openmp \
--disable-largefile \
--without-cvs \
--with-libiconv-prefix=/devel/dist/${ARCH}/${WIN_ICONV} \
--prefix=c:/devel/target/$HEX &&

sed -e 's/need_relink=yes/need_relink=no/' <gettext-tools/libtool >gettext-tools/libtool.temp && mv gettext-tools/libtool.temp gettext-tools/libtool &&
PATH=/devel/target/$HEX/bin:$PATH make install &&

cp gettext-runtime/intl/intl.def /devel/target/$HEX/lib/libintl.def &&

(cd /devel/target/$HEX &&

# Split into gettext-runtime and gettext-tools as suggested in the
# PACKAGING file. Additionally split gettext-runtime into "real
# runtime" and "developer" packages. For gettext-tools the "runtime"
# package is empty, as tools by definition are for developers.

rm -f /tmp/$RUNTIMERUNZIP /tmp/$RUNTIMEDEVZIP &&

#zip /tmp/$RUNTIMERUNZIP bin/libintl-${LTV_CURRENT_MINUS_AGE}.dll &&
zip /tmp/$RUNTIMERUNZIP bin/intl.dll &&
zip /tmp/$RUNTIMERUNZIP share/locale/locale.alias &&

zip /tmp/$RUNTIMEDEVZIP lib/libintl.dll.a &&
(cd lib && lib.exe -machine:x86 -def:libintl.def -out:intl.lib) &&
zip /tmp/$RUNTIMEDEVZIP lib/libintl.def lib/intl.lib &&
zip /tmp/$RUNTIMEDEVZIP include/libintl.h &&
zip /tmp/$RUNTIMEDEVZIP bin/{{,n}gettext,envsubst}.exe &&
zip /tmp/$RUNTIMEDEVZIP bin/gettext.sh &&
zip -r -D /tmp/$RUNTIMEDEVZIP share/man/man1/{{,n}gettext,envsubst}.1 share/man/man3 &&
zip -r -D /tmp/$RUNTIMEDEVZIP share/doc/gettext/{{,n}gettext,envsubst}.1.html &&
zip /tmp/$RUNTIMEDEVZIP share/locale/*/LC_MESSAGES/gettext-runtime.mo &&
zip /tmp/$RUNTIMEDEVZIP share/doc/gettext/*.3.html &&
zip /tmp/$RUNTIMEDEVZIP lib/GNU.Gettext.dll &&
zip -r -D /tmp/$RUNTIMEDEVZIP share/doc/gettext/csharpdoc &&
zip /tmp/$RUNTIMEDEVZIP bin/libasprintf*.dll &&
(cd /opt/mingw && zip /tmp/$RUNTIMEDEVZIP bin/libgcc_s_dw2-1.dll) &&
zip /tmp/$RUNTIMEDEVZIP lib/libasprintf.dll.a  &&
zip /tmp/$RUNTIMEDEVZIP include/autosprintf.h &&
zip -r -D /tmp/$RUNTIMEDEVZIP share/doc/libasprintf &&
# zip /tmp/$RUNTIMEDEVZIP info/autosprintf.info &&

rm -f /tmp/$TOOLSRUNZIP /tmp/$TOOLSDEVZIP &&

# the TOOLSRUNZIP is empty. created only because some of my scripts
# want both a foo and foo-dev zipfile.
zip /tmp/$TOOLSRUNZIP nul &&
zip -d /tmp/$TOOLSRUNZIP nul &&

# Now the problem is to put just the relevant stuff into the TOOLSDEVZIP

zip /tmp/$TOOLSDEVZIP bin/{autopoint,gettextize,msg*,xgettext.exe} &&
(cd /opt/mingw && zip /tmp/$TOOLSDEVZIP bin/libgcc_s_dw2-1.dll) &&
zip /tmp/$TOOLSDEVZIP share/man/man1/msg*.1 &&
zip /tmp/$TOOLSDEVZIP share/man/man1/xgettext.1 &&
zip /tmp/$TOOLSDEVZIP share/man/man1/gettextize.1 &&
zip /tmp/$TOOLSDEVZIP share/man/man1/autopoint.1 &&
zip /tmp/$TOOLSDEVZIP share/doc/gettext/*.html &&
zip -d /tmp/$TOOLSDEVZIP share/doc/gettext/*.3.html &&
# zip /tmp/$TOOLSDEVZIP info/gettext.info* &&
zip /tmp/$TOOLSDEVZIP include/gettext-po.h &&
zip /tmp/$TOOLSDEVZIP bin/libgettext*.dll lib/libgettext*.dll.a &&
zip /tmp/$TOOLSDEVZIP lib/gettext/* &&
zip /tmp/$TOOLSDEVZIP share/locale/*/LC_MESSAGES/gettext-tools.mo &&
zip -r -D /tmp/$TOOLSDEVZIP share/gettext &&
zip -r -D /tmp/$TOOLSDEVZIP share/info &&
zip -r -D /tmp/$TOOLSDEVZIP share/aclocal &&
zip -r -D /tmp/$TOOLSDEVZIP share/emacs &&

rm -f /tmp/$EXAMPLESRUNZIP /tmp/$EXAMPLESDEVZIP &&

# the EXAMPLESRUNZIP is empty. created only because some of my scripts
# want both a foo and foo-dev zipfile.
zip /tmp/$EXAMPLESRUNZIP nul &&
zip -d /tmp/$EXAMPLESRUNZIP nul &&

zip -r -D /tmp/$EXAMPLESDEVZIP share/doc/gettext/examples &&

: )

) 2>&1 | tee /devel/src/tml/packaging/$THIS.log

# Put this script and the log file in both dev packages
(cd /devel && zip /tmp/$RUNTIMEDEVZIP src/tml/packaging/$THIS.{sh,log}) &&
(cd /devel && zip /tmp/$TOOLSDEVZIP src/tml/packaging/$THIS.{sh,log}) &&

manifestify /tmp/$RUNTIMERUNZIP /tmp/$RUNTIMEDEVZIP &&
manifestify /tmp/$TOOLSRUNZIP /tmp/$TOOLSDEVZIP &&
manifestify /tmp/$EXAMPLESRUNZIP /tmp/$EXAMPLESDEVZIP

:
