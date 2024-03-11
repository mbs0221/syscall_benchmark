AC_INIT([syscall_benchmark], [1.0], [mbs2021@163.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Check for programs
AC_PROG_CC([musl-gcc])

# Specify the desired archiver program with options
AR=ar

# Checks for programs
AM_PROG_AR
AM_PROG_AS

# Set installation prefix
AC_PREFIX_DEFAULT([/usr/local/musl])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIRS([m4])

# Check for required libraries
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([keyutils.h])
AC_CHECK_HEADERS([numaif.h])
AC_CHECK_HEADERS([sys/ioctl.h])

AC_CHECK_LIB([keyutils], [keyctl_get_keyring_ID], [], [AM_CONDITIONAL([HAVE_KEYUTILS], [false])])
AC_CHECK_LIB([numa], [numa_alloc_onnode], [], [AM_CONDITIONAL([HAVE_NUMA], [false])])

AC_CHECK_FUNCS([clock_gettime])

AC_OUTPUT