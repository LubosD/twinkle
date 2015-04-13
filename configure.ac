AC_INIT([twinkle],[1.4.2])
AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR(src/phone.h)
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([src/twinkle_config.h])

AC_ARG_ENABLE(qt-check, 
	AS_HELP_STRING([--disable-qt-check],[do not check Qt installation]),
	[ac_cv_qt_check=$enableval], [ac_cv_qt_check=yes])
	
AC_ARG_WITH(kde,
	AS_HELP_STRING([--without-kde],[do not compile KDE features]),
	[ac_cv_kde=$withval], [ac_cv_kde=yes])
	
AC_ARG_WITH(ilbc,
	AS_HELP_STRING([--without-ilbc],[do not compile iLBC]),
	[ac_cv_ilbc=$withval], [ac_cv_ilbc=yes])
	
AC_ARG_WITH(speex,
	AS_HELP_STRING([--without-speex],[do not compile speex]),
	[ac_cv_speex=$withval], [ac_cv_speex=yes])

AC_ARG_WITH(zrtp,
	AS_HELP_STRING([--without-zrtp],[do not compile zrtp support]),
	[ac_cv_zrtp=$withval], [ac_cv_zrtp=yes])	

AC_ARG_ENABLE(ilbc-cpp,
	AS_HELP_STRING([--enable-ilbc-cpp],[your ilbc library is built for C++ instead of C]),
	[ac_cv_ilbc_cpp=$enableval], [ac_cv_ilbc_cpp=no])

AC_DEFINE(VERSION_DATE,["February 25 2009"],[Version release date])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_RANLIB
AC_PROG_LEX
AC_PROG_YACC
AC_C_BIGENDIAN
AC_LANG(C++)
AC_FUNC_STRERROR_R

AC_CHECK_HEADERS([linux/types.h])
AC_CHECK_HEADERS([linux/errqueue.h],[],[],[
	#if HAVE_LINUX_TYPES_H
	#include <linux/types.h>
	#endif])

# Temporarily add some default directories to PKG_CONFIG_PATH such that
# the user will not be burdened with setting PKG_CONFIG_PATH
OLD_PKG_CONFIG_PATH=$PKG_CONFIG_PATH
PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig:/usr/lib/pkgconfig

if test "x${prefix}" != "xNONE"
then
	PKG_CONFIG_PATH=$PKG_CONFIG_PATH:${prefix}/lib/pkgconfig
fi

if test -n "$QTDIR"
then
	PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$QTDIR/lib/pkgconfig
fi

export PKG_CONFIG_PATH

PKG_CHECK_MODULES(CCRTP, libccrtp >= 2.0.0)
PKG_CHECK_MODULES(UCOMMON, ucommon >= 5.0.0)
CCRTP_LIBS="$CCRTP_LIBS $UCOMMON_LIBS"

PKG_CHECK_MODULES(XML2, libxml-2.0)
# AC_CHECK_HEADER(libxml/tree.h, [],
#	[AC_MSG_ERROR([libxml2 header files missing (libxml2-devel package)])])

if test "x$ac_cv_qt_check" = "xyes"
then
	PKG_CHECK_MODULES(QT, qt-mt >= 3.3.0 qt-mt < 4.0)
fi

# Restore the original value of PKG_CONFIG_PATH
PKG_CONFIG_PATH=$OLD_PKG_CONFIG_PATH
export PKG_CONFIG_PATH

# Check if QTDIR variable is set
AC_MSG_CHECKING([value of \$QTDIR])
if test -n "$QTDIR"
then
	AC_MSG_RESULT([$QTDIR])
else
	AC_MSG_RESULT([not set])
	AC_MSG_ERROR([Set \$QTDIR to the Qt directory, eg. /usr/lib/qt3])
fi

AC_MSG_CHECKING([for qmake])
if test -x $QTDIR/bin/qmake
then
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
	AC_MSG_ERROR([Cannot find qmake in \$QTDIR/bin. \$QTDIR is incorrect.])
fi

# Without this macro, compiling on non-kde systems does not work
AC_PATH_QT

# Check if lrelease is available
AC_CHECK_PROG(have_lrelease, lrelease, yes, no)
if test "x$have_lrelease" = "xno"
then
	AC_MSG_ERROR([lrelease is missing (qt3-devel-tools package)])
fi

dnl Create a Qt include project file for platform dependend variables
dnl This file will be included by the project file in the gui directory
dnl Determine include files and dynamic libs.
dnl Creation of qtccxxincl.pro is good to be in AC_CONFIG_COMMANDS
dnl macro so that config.status to be able to create it too.
dnl In this case we benefit from more ac_xxx variables.
dnl Disadvantage is autoconf don't group commands with same tag
dnl and we should use different tags. In result AC_OUTPUT will
dnl show all of them.
QT_INCL_PRO=qtccxxincl.pro
AC_MSG_NOTICE([creating $QT_INCL_PRO (include project file for Qt)])

# first set include path to generated files
echo "INCLUDEPATH += `pwd`/src" > $QT_INCL_PRO
# next get includes specified on command line
(
set -- $CPPFLAGS
while test -n "$1"; do
  echo $1
  shift
done
) | grep '^-I' | sed -e 's|^-I||' | xargs echo INCLUDEPATH += >> $QT_INCL_PRO

#echo "INCLUDEPATH += `$CCGNU2_CONFIG --includes`" | sed -e s/-I//g > $QT_INCL_PRO
# libccrtp1(ccrtp) depend from libccgnu2(commoncpp2) and
# should include above flags !
echo "INCLUDEPATH += `$PKG_CONFIG --cflags-only-I libccrtp`" | sed -e s/-I//g >> $QT_INCL_PRO
echo "INCLUDEPATH += `$PKG_CONFIG --cflags-only-I libxml-2.0`" | sed -e s/-I//g >> $QT_INCL_PRO

# get libraries specified on command line
echo $LDFLAGS | xargs echo LIBS += >> $QT_INCL_PRO

echo "LIBS += `$CCGNU2_CONFIG --stdlibs`" >> $QT_INCL_PRO
echo "LIBS += $CCRTP_LIBS" >> $QT_INCL_PRO
echo "LIBS += $UCOMMON_LIBS" >> $QT_INCL_PRO
echo "LIBS += $XML2_LIBS" >> $QT_INCL_PRO

# Check if KDE is available
if test "x$ac_cv_kde" = "xyes"
then
	AC_CHECK_PROG(have_kde, kde-config, yes, no)
else
	have_kde="no"
fi
	
if test "x$have_kde" = "xyes"
then
	KDE_SET_PREFIX
	AC_PATH_KDE

	AC_CHECK_FILES(${kde_includes}/kapplication.h,
		[], 
		[AC_MSG_ERROR([kde header files missing (kdelibs3-devel package)])])
	AC_DEFINE(HAVE_KDE, 1, [Define to 1 if you have KDE.])
	echo "INCLUDEPATH += ${kde_includes}" >> $QT_INCL_PRO
	echo "LIBS += -L${kde_libraries} -lkdecore -lkdeui -lkio" >> $QT_INCL_PRO
	echo "LIBS += -L${TWINKLE_KDE_PREFIX}/lib -lkabc" >> $QT_INCL_PRO
else
	include_x11_FALSE="yes"
	include_ARTS_FALSE="yes"
	AC_PREFIX_DEFAULT(${prefix:-/usr/local})
fi

# Check for libbbind or libresolv. libbind is preferred as libresolv gives
# GLIBC_PRIVATE on Fedora.
AC_CHECK_LIB(bind, main, [
	LIBS="-lbind $LIBS"
	echo "LIBS += -lbind" >> $QT_INCL_PRO],
	[
	LIBS="-lresolv $LIBS"
	echo "LIBS += -lresolv" >> $QT_INCL_PRO])

# Check if sndfile library is available
AC_CHECK_HEADER(sndfile.h, [],
	[AC_MSG_ERROR([sndfile header files missing (libsndfile-devel package)])])
AC_CHECK_LIB(sndfile, sf_open, [], 
	[AC_MSG_ERROR([libsndfile library is missing.])])
	
# Check if magic library is available
AC_CHECK_HEADER(magic.h, [],
	[AC_MSG_ERROR([magic.h is missing (file-devel or libmagic-dev package)])])
AC_CHECK_LIB(magic, magic_open, [],
	[AC_MSG_ERROR([libmagic library is missing (file or libmagic package)])])

# This check does not work on all platforms
# Check if libgsm is available
# AC_CHECK_LIB(gsm, sf_open, [
#	AC_CHECK_HEADER(gsm.h, [],
#		[AC_MSG_ERROR([gsm header files missing (gsm.h)])])
#	AC_DEFINE(HAVE_GSM, 1, [Define to 1 if you have the <libgsm> library.])
#	GSM_LIBS="-lgsm"
#	echo "LIBS += -lgsm" >> $QT_INCL_PRO
#	have_gsm="yes" ], [
#	have_gsm="no"
#	GSM_LIBS="\$(top_builddir)/src/audio/gsm/libgsm.a"
#	echo "LIBS += ../audio/gsm/libgsm.a" >> $QT_INCL_PRO ])
have_gsm="no"
GSM_LIBS="\$(top_builddir)/src/audio/gsm/libgsm.a"
echo "LIBS += ../audio/gsm/libgsm.a" >> $QT_INCL_PRO

AC_SUBST(GSM_LIBS)

# Check if ALSA is available
AC_CHECK_LIB(asound, main, [
	AC_CHECK_HEADER(alsa/asoundlib.h, [],
		[AC_MSG_ERROR([alsa header files missing (alsa-devel package)])])
	AC_DEFINE(HAVE_LIBASOUND, 1, [Define to 1 if you have the <libasound> library.])
	LIBS="-lasound $LIBS"
	echo "LIBS += -lasound" >> $QT_INCL_PRO
	have_libasound="yes"
	], [have_libasound="no"])
	
# Check if SPEEX (libspeex & libspeexdsp) is available 
if test "x$ac_cv_speex" = "xyes"
then
	AC_CHECK_LIB(speex, main, [
		AC_CHECK_HEADER(speex/speex.h, [],
			[AC_MSG_ERROR([speex header files missing])])
		AC_CHECK_LIB(speexdsp, main, [
			AC_CHECK_HEADERS([speex/speex_preprocess.h speex/speex_echo.h], [],
				[AC_MSG_ERROR([speexdsp header files missing])])
			AC_DEFINE(HAVE_SPEEX, 1, [Define to 1 if you have the <speex> library.])
			LIBS="-lspeex -lspeexdsp $LIBS"
			echo "LIBS += -lspeex" >> $QT_INCL_PRO
			echo "LIBS += -lspeexdsp" >> $QT_INCL_PRO
			have_speex="yes"
			], [have_speex="no"])
		], [have_speex="no"])
else
	have_speex="no"
fi

# iLBC
if test "x$ac_cv_ilbc" = "xyes"
then
	AC_CHECK_LIB(ilbc, iLBC_decode, [
		AC_CHECK_HEADER(ilbc/iLBC_define.h, [],
			[AC_MSG_ERROR([ilbc header files missing])])
		AC_DEFINE(HAVE_ILBC, 1, [Define to 1 if you have the <ilbc> library.])
		LIBS="-lilbc $LIBS"
		echo "LIBS += -lilbc" >> $QT_INCL_PRO
		have_ilbc="yes"
		], [have_ilbc="no"])
		
	if test "x$ac_cv_ilbc_cpp" = "xyes"
	then
		AC_DEFINE(HAVE_ILBC_CPP, 1, [Define to 1 if you have a C++ ilbc library.])
	fi
else
	have_ilbc="no"
fi

# Check if zrtp is available
if test "x$ac_cv_zrtp" = "xyes"
then
	PKG_CHECK_MODULES(ZRTP, libzrtpcpp >= 2.0.0)
	AC_CHECK_LIB(zrtpcpp, main, [
		AC_CHECK_HEADER(libzrtpcpp/ZrtpQueue.h, [],
			[AC_MSG_ERROR([zrtp header files missing])])
		AC_DEFINE(HAVE_ZRTP, 1, [Define to 1 if you have the <libzrtpcpp> library.])
		LIBS="-lzrtpcpp $LIBS"
		echo "LIBS += -lzrtpcpp" >> $QT_INCL_PRO
		have_zrtp="yes"
		], [have_zrtp="no"])
else
	have_zrtp="no"
fi

# check if GNU readline or readline compatible libraries are avaliable
VL_LIB_READLINE()
if test "x$vl_cv_lib_readline" == "xno"
then
	AC_MSG_ERROR([readline-devel package is not installed])
fi

# Check if boost regex is available
AC_CHECK_HEADER(boost/regex.h, [],
	[AC_MSG_ERROR([boost/regex.h missing (boost-devel package)])])
AC_CHECK_LIB(boost_regex, main, [
	LIBS="-lboost_regex $LIBS"
	echo "LIBS += -lboost_regex" >> $QT_INCL_PRO], 
	[
	AC_CHECK_LIB(boost_regex-gcc, main, [
		LIBS="-lboost_regex-gcc $LIBS"
		echo "LIBS += -lboost_regex-gcc" >> $QT_INCL_PRO],
		[AC_MSG_ERROR([libboost_regex library is missing (boost package).])])])

ms_CHECK_LRELEASE()
	
dnl $QTDIR/bin/qmake -o src/gui/Makefile src/gui/twinkle.pro
dnl Command above is incorrect because srcdir may differ from builddir
dnl
dnl $QTDIR/bin/qmake -o src/gui/Makefile $srcdir/src/gui/twinkle.pro
dnl Command above although is correct in both cases, i.e. when srcdir
dnl is equal or differ from builddir will not generate proper Makefile.
dnl Reason is unknown.
dnl
dnl The sed command adds the KDE3 libraries in front of the LIB path.
dnl This is needed to correctly build on a KDE4 system. Otherwise
dnl gcc will link against KDE4 libraries.
AC_CONFIG_COMMANDS([src/gui/Makefile],
	[
	$QTDIR/bin/qmake -o src/gui/Makefile $srcdir/src/gui/twinkle.pro
	echo "distdir:" >> src/gui/Makefile
	echo "check:" >> src/gui/Makefile
	
	if test -n "$kde_libraries"
	then
		sed -i -e "s|\(LIBS *= \)|\1-L${kde_libraries} |" src/gui/Makefile
	fi
	],
	[kde_libraries=${kde_libraries}]
)

dnl Next is useless. See comments in src/parser/Makefile.am .
dnl # Strip the -O2 flag from CXXFLAGS for building the SIP parser.
dnl # g++ cannot compile the generated C code from bison with -O2 or -Os
dnl # Note: -Os is used on VIA C3 processor
dnl PARSER_CXXFLAGS=`echo $CXXFLAGS | sed -e "s/\-O2//;s/\-Os//"`
dnl AC_SUBST(PARSER_CXXFLAGS)

AC_CONFIG_FILES([Makefile src/Makefile src/audio/Makefile src/audio/gsm/Makefile \
	src/audits/Makefile src/sdp/Makefile src/parser/Makefile \
	src/sockets/Makefile src/stun/Makefile src/threads/Makefile \
	src/gui/lang/Makefile src/mwi/Makefile src/im/Makefile \
	src/patterns/Makefile src/utils/Makefile src/presence/Makefile \
	twinkle.spec])
AC_OUTPUT
	
AC_MSG_RESULT([])
AC_MSG_RESULT([Configured optional components:])
AC_MSG_RESULT([KDE support: $have_kde])
AC_MSG_RESULT([ALSA:        $have_libasound])
AC_MSG_RESULT([Speex:       $have_speex])
AC_MSG_RESULT([iLBC:        $have_ilbc])
AC_MSG_RESULT([ZRTP:        $have_zrtp])
