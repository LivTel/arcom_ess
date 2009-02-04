# $Header: /home/cjm/cvs/arcom_ess/c/Makefile,v 1.3 2009-02-04 11:24:15 cjm Exp $

include			../../Makefile.common
include			../Makefile.common

INCDIR = $(ARCOM_ESS_SRC_HOME)/include
DOCSDIR = $(ARCOM_ESS_DOC_HOME)/cdocs

# Do we want to enable logging?
# no logging
#LOGGINGCFLAGS = -DLOGGING=0
LOGGINGCFLAGS = -DLOGGING=10

#Do we want MUTEX protected locking around controller command communication?
MUTEXCFLAGS = -DARCOM_ESS_MUTEXED=1
#MUTEXCFLAGS = 

# Disable Nagle's algorithm?
#TCP_NODELAY = -DARCOM_ESS_TCP_NODELAY=1
TCP_NODELAY = 

CFLAGS = -g $(CCHECKFLAG) -I$(INCDIR) -I$(JNIINCDIR) -I$(JNIMDINCDIR) -L$(LT_LIB_HOME) $(MUTEXCFLAGS) \
	$(TCP_NODELAY) $(LOGGINGCFLAGS) $(LOG_UDP_CFLAGS)

LINTFLAGS = -I$(INCDIR) -I$(JNIINCDIR) -I$(JNIMDINCDIR)
DOCFLAGS = -static
SRCS 		= 	arcom_ess_general.c arcom_ess_serial.c arcom_ess_socket.c arcom_ess_interface.c
HEADERS		=	$(SRCS:%.c=%.h)
OBJS		=	$(SRCS:%.c=%.o)
DOCS 		= 	$(SRCS:%.c=$(DOCSDIR)/%.html)
JAVASRCS 	= 	$(SRCS) ngat_serial_arcomess_ArcomESS.c
JAVAOBJS	=	$(JAVASRCS:%.c=%.o)
JAVADOCS 	= 	$(JAVASRCS:%.c=$(DOCSDIR)/%.html)
LIBNAME		=	lib$(ARCOM_ESS_HOME)

top: shared docs $(INCDIR)/ngat_serial_arcomess_ArcomESS.h 

shared: $(LT_LIB_HOME)/$(LIBNAME).so

$(LT_LIB_HOME)/$(LIBNAME).so: $(JAVAOBJS)
	cc $(CCSHAREDFLAG) $(CFLAGS) $(JAVAOBJS) -o $@ $(TIMELIB) 

static: $(LT_LIB_HOME)/$(LIBNAME).a

$(LT_LIB_HOME)/$(LIBNAME).a: $(OBJS)
	ar rcv $@ $?

# special case for ngat_serial_arcomess_ArcomESS - depending on whether ngat.serial.arcomess.ArcomESS has changed
# recreate the machine generated JNI header
# touch it, javah does not update the timestamp correctly
# $(LT_JAVALIB_HOME)/ngat_serial.jar
$(INCDIR)/ngat_serial_arcomess_ArcomESS.h: $(LT_SRC_HOME)/ngat/serial/arcomess/ArcomESS.java
	$(JAVAH) -classpath $(LT_HOME)/classes:$(CLASSPATH) -jni -d $(INCDIR) ngat.serial.arcomess.ArcomESS
	$(TOUCH) $@


docs: $(JAVADOCS)

$(JAVADOCS): $(JAVASRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(JAVASRCS)

$(JAVADOCS) : $(JAVASRCS)

checkout:
	-$(CO) $(CO_OPTIONS) $(JAVASRCS)
	-(cd $(INCDIR); $(CO) $(CO_OPTIONS) $(HEADERS);)

checkin:
	-$(CI) $(CI_OPTIONS) $(JAVASRCS)
	-(cd $(INCDIR); $(CI) $(CI_OPTIONS) $(HEADERS);)

staticdepend:
	makedepend $(MAKEDEPENDFLAGS) -- $(CFLAGS) -- $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -- $(CFLAGS) -- $(JAVASRCS)

lint:
	$(LINT)	$(LINTFLAGS) $(JAVASRCS)

clean:
	-$(RM) $(RM_OPTIONS) $(JAVAOBJS) $(INCDIR)/ngat_serial_arcomess_ArcomESS.h $(LT_LIB_HOME)/$(LIBNAME).so $(LT_LIB_HOME)/$(LIBNAME)_g.so $(LT_LIB_HOME)/$(LIBNAME).a $(TIDY_OPTIONS)

tidy:
	-$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)
	-(cd $(INCDIR); $(RM) $(RM_OPTIONS) $(TIDY_OPTIONS);)

backup: tidy
	-$(RM) $(RM_OPTIONS) $(JAVAOBJS) $(INCDIR)/ngat_serial_arcomess_ArcomESS.h

#
# $Log: not supported by cvs2svn $
# Revision 1.2  2008/07/22 13:42:39  cjm
# Fixed MAKEDEPENDFLAGS.
#
# Revision 1.1  2008/03/18 17:04:34  cjm
# Initial revision
#
#
# DO NOT DELETE

arcom_ess_general.o: /usr/include/errno.h /usr/include/features.h
arcom_ess_general.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
arcom_ess_general.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
arcom_ess_general.o: /usr/include/asm/errno.h /usr/include/stdio.h
arcom_ess_general.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
arcom_ess_general.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
arcom_ess_general.o: /usr/include/bits/typesizes.h /usr/include/libio.h
arcom_ess_general.o: /usr/include/_G_config.h /usr/include/wchar.h
arcom_ess_general.o: /usr/include/bits/wchar.h /usr/include/gconv.h
arcom_ess_general.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
arcom_ess_general.o: /usr/include/bits/stdio_lim.h
arcom_ess_general.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
arcom_ess_general.o: /usr/include/string.h /usr/include/time.h
arcom_ess_general.o: /usr/include/bits/time.h /usr/include/unistd.h
arcom_ess_general.o: /usr/include/bits/posix_opt.h
arcom_ess_general.o: /usr/include/bits/confname.h /usr/include/getopt.h
arcom_ess_general.o: /home/dev/src/arcom_ess/include/arcom_ess_general.h
arcom_ess_serial.o: /usr/include/errno.h /usr/include/features.h
arcom_ess_serial.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
arcom_ess_serial.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
arcom_ess_serial.o: /usr/include/asm/errno.h /usr/include/stdio.h
arcom_ess_serial.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
arcom_ess_serial.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
arcom_ess_serial.o: /usr/include/bits/typesizes.h /usr/include/libio.h
arcom_ess_serial.o: /usr/include/_G_config.h /usr/include/wchar.h
arcom_ess_serial.o: /usr/include/bits/wchar.h /usr/include/gconv.h
arcom_ess_serial.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
arcom_ess_serial.o: /usr/include/bits/stdio_lim.h
arcom_ess_serial.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
arcom_ess_serial.o: /usr/include/string.h /usr/include/fcntl.h
arcom_ess_serial.o: /usr/include/bits/fcntl.h /usr/include/sys/types.h
arcom_ess_serial.o: /usr/include/time.h /usr/include/bits/time.h
arcom_ess_serial.o: /usr/include/termios.h /usr/include/bits/termios.h
arcom_ess_serial.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
arcom_ess_serial.o: /usr/include/bits/confname.h /usr/include/getopt.h
arcom_ess_serial.o: /home/dev/src/arcom_ess/include/arcom_ess_general.h
arcom_ess_serial.o: /home/dev/src/arcom_ess/include/arcom_ess_serial.h
arcom_ess_serial.o: /home/dev/src/arcom_ess/include/arcom_ess_interface.h
arcom_ess_socket.o: /usr/include/arpa/inet.h /usr/include/features.h
arcom_ess_socket.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
arcom_ess_socket.o: /usr/include/netinet/in.h /usr/include/stdint.h
arcom_ess_socket.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
arcom_ess_socket.o: /usr/include/bits/types.h
arcom_ess_socket.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
arcom_ess_socket.o: /usr/include/bits/typesizes.h /usr/include/bits/socket.h
arcom_ess_socket.o: /usr/include/limits.h
arcom_ess_socket.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/limits.h
arcom_ess_socket.o: /usr/include/bits/posix1_lim.h
arcom_ess_socket.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
arcom_ess_socket.o: /usr/include/bits/posix2_lim.h /usr/include/sys/types.h
arcom_ess_socket.o: /usr/include/time.h /usr/include/bits/time.h
arcom_ess_socket.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
arcom_ess_socket.o: /usr/include/asm/sockios.h /usr/include/bits/in.h
arcom_ess_socket.o: /usr/include/endian.h /usr/include/bits/endian.h
arcom_ess_socket.o: /usr/include/bits/byteswap.h /usr/include/errno.h
arcom_ess_socket.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
arcom_ess_socket.o: /usr/include/asm/errno.h /usr/include/fcntl.h
arcom_ess_socket.o: /usr/include/bits/fcntl.h /usr/include/netdb.h
arcom_ess_socket.o: /usr/include/bits/netdb.h /usr/include/netinet/tcp.h
arcom_ess_socket.o: /usr/include/stdio.h /usr/include/libio.h
arcom_ess_socket.o: /usr/include/_G_config.h /usr/include/wchar.h
arcom_ess_socket.o: /usr/include/gconv.h
arcom_ess_socket.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
arcom_ess_socket.o: /usr/include/bits/stdio_lim.h
arcom_ess_socket.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
arcom_ess_socket.o: /usr/include/string.h /usr/include/sys/socket.h
arcom_ess_socket.o: /usr/include/sys/uio.h /usr/include/bits/uio.h
arcom_ess_socket.o: /usr/include/termios.h /usr/include/bits/termios.h
arcom_ess_socket.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
arcom_ess_socket.o: /usr/include/bits/confname.h /usr/include/getopt.h
arcom_ess_socket.o: /home/dev/src/arcom_ess/include/arcom_ess_general.h
arcom_ess_socket.o: /home/dev/src/arcom_ess/include/arcom_ess_socket.h
arcom_ess_socket.o: /home/dev/src/arcom_ess/include/arcom_ess_interface.h
arcom_ess_interface.o: /usr/include/errno.h /usr/include/features.h
arcom_ess_interface.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
arcom_ess_interface.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
arcom_ess_interface.o: /usr/include/asm/errno.h /usr/include/stdio.h
arcom_ess_interface.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
arcom_ess_interface.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
arcom_ess_interface.o: /usr/include/bits/typesizes.h /usr/include/libio.h
arcom_ess_interface.o: /usr/include/_G_config.h /usr/include/wchar.h
arcom_ess_interface.o: /usr/include/bits/wchar.h /usr/include/gconv.h
arcom_ess_interface.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
arcom_ess_interface.o: /usr/include/bits/stdio_lim.h
arcom_ess_interface.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
arcom_ess_interface.o: /usr/include/string.h /usr/include/time.h
arcom_ess_interface.o: /usr/include/bits/time.h /usr/include/unistd.h
arcom_ess_interface.o: /usr/include/bits/posix_opt.h
arcom_ess_interface.o: /usr/include/bits/confname.h /usr/include/getopt.h
arcom_ess_interface.o: /usr/include/pthread.h /usr/include/sched.h
arcom_ess_interface.o: /usr/include/bits/sched.h /usr/include/signal.h
arcom_ess_interface.o: /usr/include/bits/sigset.h
arcom_ess_interface.o: /usr/include/bits/pthreadtypes.h
arcom_ess_interface.o: /usr/include/bits/initspin.h
arcom_ess_interface.o: /usr/include/bits/sigthread.h
arcom_ess_interface.o: /home/dev/src/arcom_ess/include/arcom_ess_general.h
arcom_ess_interface.o: /home/dev/src/arcom_ess/include/arcom_ess_serial.h
arcom_ess_interface.o: /usr/include/termios.h /usr/include/bits/termios.h
arcom_ess_interface.o: /home/dev/src/arcom_ess/include/arcom_ess_interface.h
arcom_ess_interface.o: /home/dev/src/arcom_ess/include/arcom_ess_socket.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/stdio.h /usr/include/features.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/sys/cdefs.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/gnu/stubs.h
ngat_serial_arcomess_ArcomESS.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/bits/types.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/bits/wordsize.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/bits/typesizes.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/libio.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/_G_config.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/wchar.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/bits/wchar.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/gconv.h
ngat_serial_arcomess_ArcomESS.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/bits/stdio_lim.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/bits/sys_errlist.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/stdlib.h /usr/include/string.h
ngat_serial_arcomess_ArcomESS.o: /usr/java/j2sdk1.4.2_06/include/jni.h
ngat_serial_arcomess_ArcomESS.o: /usr/java/j2sdk1.4.2_06/include/linux/jni_md.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/time.h /usr/include/bits/time.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/termios.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/bits/termios.h
ngat_serial_arcomess_ArcomESS.o: /home/dev/src/arcom_ess/include/arcom_ess_general.h
ngat_serial_arcomess_ArcomESS.o: /home/dev/src/arcom_ess/include/arcom_ess_interface.h
ngat_serial_arcomess_ArcomESS.o: /home/dev/src/arcom_ess/include/arcom_ess_serial.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/unistd.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/bits/posix_opt.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/bits/confname.h
ngat_serial_arcomess_ArcomESS.o: /usr/include/getopt.h
