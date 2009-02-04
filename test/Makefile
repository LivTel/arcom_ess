# $Header: /home/cjm/cvs/arcom_ess/test/Makefile,v 1.3 2009-02-04 11:24:26 cjm Exp $

include ../../Makefile.common
include ../Makefile.common

INCDIR 		= $(ARCOM_ESS_SRC_HOME)/include
TESTDIR 	= test
BINDIR 		= $(ARCOM_ESS_BIN_HOME)/$(TESTDIR)/$(HOSTTYPE)
DOCSDIR 	= $(ARCOM_ESS_DOC_HOME)/$(TESTDIR)

CFLAGS 		= -g -I$(INCDIR) -I$(JNIINCDIR) -I$(JNIMDINCDIR) $(LOG_UDP_CFLAGS)
DOCFLAGS 	= -static

SRCS 		= arcom_ess_test.c
OBJS 		= $(SRCS:%.c=%.o)
PROGS 		= $(SRCS:%.c=$(BINDIR)/%)
DOCS 		= $(SRCS:%.c=$(DOCSDIR)/%.html)
LIBNAME         = $(ARCOM_ESS_HOME)

top: $(PROGS) docs

$(BINDIR)/%: %.o
	cc -o $@ $< -L$(LT_LIB_HOME) -l$(LIBNAME) $(TIMELIB) $(SOCKETLIB) -lm -lc

docs: $(DOCS)

$(DOCS): $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -- $(CFLAGS) -- $(SRCS)

clean:
	$(RM) $(RM_OPTIONS) $(PROGS) $(OBJS) $(TIDY_OPTIONS)

tidy:
	$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)

backup: tidy
	$(RM) $(RM_OPTIONS) $(OBJS)

checkin:
	-$(CI) $(CI_OPTIONS) $(SRCS)

checkout:
	-$(CO) $(CO_OPTIONS) $(SRCS)

# DO NOT DELETE

