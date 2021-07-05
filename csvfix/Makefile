# makefile for csvfix
# Copyright (C) 2009 Neil Butterworth
#
# To make the windows version:
#    make win
#
# To make the Linux/UNIX version
#    make lin

WINOUT = bin/csvfix.exe
LINOUT = bin/csvfix

CCTYPE = gcc
IDIR = inc
ODIR = obj
SDIR = src
INC = -Iinc -I../alib/inc

cc.gcc = g++ -std=c++0x
cc.clang = clang++ -std=c++11 -stdlib=libc++ 
CC = ${cc.${CCTYPE}}

ALIB = ../alib/lib/alib.a
WINLIBS = ../alib/lib/alib.a -lodbc32 
LINLIBS = ../alib/lib/alib.a 

_OBJS = csved_atable.o \
		csved_block.o \
		csved_case.o \
		csved_cli.o \
		csved_check.o \
		csved_command.o \
		csved_config.o \
		csved_date.o \
		csved_diff.o \
		csved_dsv.o \
		csved_echo.o \
		csved_edit.o \
		csved_escape.o \
		csved_eval.o \
		csved_evalvars.o \
		csved_exclude.o \
		csved_except.o \
		csved_exec.o \
		csved_fileinfo.o \
		csved_filesplit.o \
		csved_find.o \
		csved_fixed.o \
		csved_flatten.o \
		csved_fmerge.o \
		csved_fromxml.o \
		csved_headtail.o \
		csved_inter.o \
		csved_ioman.o \
		csved_join.o \
		csved_main.o \
		csved_map.o \
		csved_merge.o \
		csved_money.o \
		csved_number.o \
		csved_order.o \
		csved_printf.o \
		csved_put.o \
		csved_readmulti.o \
		csved_rmnewline.o \
		csved_rules.o \
		csved_seq.o \
		csved_shuffle.o \
		csved_sort.o \
		csved_sparse.o \
		csved_split.o \
		csved_sql.o \
		csved_stat.o \
		csved_sum.o \
		csved_template.o \
		csved_timestamp.o \
		csved_toxml.o \
		csved_trim.o \
		csved_truncpad.o \
		csved_unique.o \
		csved_util.o \
		csved_valid.o \
		csved_writemulti.o \

_WINOBJS = csved_odbc.o csved_call.o

OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))
WINOBJS = $(patsubst %,$(ODIR)/%,$(_WINOBJS))

.PHONY: null
null:
	@echo "must use 'make win', 'make lin' or 'make clean'"

$(ODIR)/%.o: $(SDIR)/%.cpp 
	$(CC) -c $(INC) -o $@ $< $(CFLAGS) 

$(WINOUT): $(OBJS) $(WINOBJS) $(ALIB)
	$(CC) -o $@ $^ $(WINLIBS)
	strip $(WINOUT)

$(LINOUT): $(OBJS) $(ALIB)
	$(CC) -o $@ $^ $(LINLIBS)
	strip $(LINOUT)

.PHONY: win
win:
	$(MAKE) $(WINOUT)

.PHONY: lin
lin:
	$(MAKE) $(LINOUT)

.PHONY: clean
clean:
	rm -Rf bin/* obj/* 
