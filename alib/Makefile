# makefile for alib
# Copyright (C) 2009 Neil Butterworth

OUT = lib/alib.a
CCTYPE = gcc
IDIR = inc
ODIR = obj
XDIR = expat
SDIR = src
INC = -Iinc -Iexpat

cc.gcc = g++
cc.clang = clang++ -std=c++11 -stdlib=libc++  
CC = ${cc.${CCTYPE}}

_OBJS = a_chsrc.o a_csv.o a_enc.o a_env.o a_except.o \
		a_expr.o a_myth.o a_inifile.o  a_exec.o \
		a_file.o a_html.o a_io.o a_rand.o a_time.o \
		a_regex.o a_shstr.o a_slice.o a_sort.o a_str.o a_table.o \
		a_xmlevents.o a_xmlparser.o a_xmltree.o \
		a_date.o a_range.o 


OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

_WINOBJS = a_db.o a_dir.o a_opsys.o a_winerr.o
WINOBJS = $(patsubst %,$(ODIR)/%,$(_WINOBJS))

_EXPAT = xmlparse.o  xmlrole.o  xmltok.o  xmltok_impl.o  xmltok_ns.o
EXPAT = $(patsubst %,$(ODIR)/%,$(_EXPAT))

$(ODIR)/%.o: $(SDIR)/%.cpp $(IDIR)/%.h 
	$(CC) -c $(INC) $(CFLAGS) -o $@ $< 

$(ODIR)/%.o: $(XDIR)/%.cpp 
	$(CC) -c $(INC) $(CFLAGS) -o $@ $< 

win: $(OBJS) $(EXPAT) $(WINOBJS)
	ar rvs $(OUT) $^

lin: $(OBJS) $(EXPAT) 
	ar rvs $(OUT) $^

.PHONY: clean
clean:
	rm -rf lib/* obj/*  

