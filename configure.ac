# see semver.org for version info
AC_INIT([cconfigspace],m4_normalize(m4_include([VERSION])),[bvideau@anl.gov])

# are we in the right source dir ?
AC_CONFIG_SRCDIR([src/cconfigspace.c])

# build artefacts in separate dir
AC_CONFIG_AUX_DIR([m4])
AC_CONFIG_MACRO_DIR([m4])

# detect OS
AC_CANONICAL_HOST
case $host_os in
  *macos*) macos="yes" ;;
  *darwin*) macos="yes" ;;
  *) macos="no" ;;
esac

# automake should fail on any error
AM_INIT_AUTOMAKE([-Wall -Werror foreign 1.12])
AM_PROG_AR
AM_PROG_CC_C_O

# check for libtool
LT_INIT

# check for programs
AC_LANG([C])
AC_PROG_CC
AC_PROG_CC_STDC
AC_TYPE_SIZE_T
AC_HEADER_ASSERT
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX([14], [noext], [mandatory])

AM_CONDITIONAL([ISMACOS], [test "x$macos" = xyes])

AC_ARG_ENABLE([strict], AS_HELP_STRING([--enable-strict], [Enable -Werror]))
AM_CONDITIONAL([STRICT], [test "x$enable_strict" = xyes])

AC_ARG_ENABLE([kokkos-connector],
               AS_HELP_STRING([--enable-kokkos-connector], [Enable CConfigSpace Kokkos connector]),
               [], [enable_kokkos_connector=yes])
AM_CONDITIONAL([KOKKOS], [test "x$enable_kokkos_connector" = xyes])

# Check Ruby
PKG_CHECK_MODULES([RUBY], [ruby >= 2.3], [HAVE_RB=yes], [HAVE_RB=no])
if test "x$HAVE_RB" != xyes;
then
  AC_PATH_PROG([RUBY], [ruby])
  if test "x$RUBY" != x;
  then
    RUBYHDRDIR=`$RUBY -e 'puts RbConfig::CONFIG[["rubyhdrdir"]]'`
    RUBYARCHHDRDIR=`$RUBY -e 'puts RbConfig::CONFIG[["rubyarchhdrdir"]]'`
    LIBRUBYARG=`$RUBY -e 'puts RbConfig::CONFIG[["LIBRUBYARG"]]'`
    RUBYLIBDIR=`$RUBY -e 'puts RbConfig::CONFIG[[ RbConfig::CONFIG["libdirname"] ]]'`
    RUBY_CFLAGS="-I$RUBYARCHHDRDIR -I$RUBYHDRDIR"
    RUBY_LIBS="-L$RUBYLIBDIR $LIBRUBYARG -lm"
    AC_SUBST([RUBY_CFLAGS])
    AC_SUBST([RUBY_LIBS])
    HAVE_RB=yes
  else
    HAVE_RB=no
  fi
fi
AM_CONDITIONAL([HAVE_RUBY], [test "x$HAVE_RB" = xyes])
PKG_CHECK_MODULES([PYTHON], [python3-embed >= 3.6])

#check for libraries
AC_CHECK_LIB([m],[cos])
AC_CHECK_LIB([gslcblas],[cblas_dgemm])
AC_CHECK_LIB([gsl],[gsl_blas_dgemm])

#check for valgrind
AX_VALGRIND_DFLT([helgrind], [off])
AX_VALGRIND_DFLT([drd], [off])
AX_VALGRIND_DFLT([sgcheck], [off])
AX_VALGRIND_CHECK

AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([Makefile
		include/Makefile
		src/Makefile
		tests/Makefile
		samples/Makefile
		connectors/Makefile
		cconfigspace.pc])
if test "x$enable_kokkos_connector" = xyes
then
AC_CONFIG_FILES([connectors/kokkos/Makefile])
fi
AC_CONFIG_LINKS([samples/test_ruby.rb:samples/test_ruby.rb
                 samples/test_python.py:samples/test_python.py])
AC_OUTPUT
