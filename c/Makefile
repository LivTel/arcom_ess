# $Header: /home/cjm/cvs/arcom_ess/c/Makefile,v 1.2 2008-07-22 13:42:39 cjm Exp $

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
	$(TCP_NODELAY) $(LOGGINGCFLAGS)

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
# Revision 1.1  2008/03/18 17:04:34  cjm
# Initial revision
#
#
