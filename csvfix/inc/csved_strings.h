//---------------------------------------------------------------------------
// csved_strings.h
//
// string constants for CSVED
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_STRINGS_H
#define INC_CSVED_STRINGS_H

#include "a_base.h"

namespace CSVED {

//---------------------------------------------------------------------------
// Command names
//---------------------------------------------------------------------------

const char * const CMD_ATABLE	= "ascii_table";
const char * const CMD_BLOCK	= "block";
const char * const CMD_CALL		= "call";
const char * const CMD_CHECK	= "check";
const char * const CMD_DIFF		= "diff";
const char * const CMD_DREAD	= "date_iso";
const char * const CMD_DFMT		= "date_format";
const char * const CMD_DSVR		= "read_dsv";
const char * const CMD_DSVW		= "write_dsv";
const char * const CMD_ECHO		= "echo";
const char * const CMD_EDIT		= "edit";
const char * const CMD_EVAL		= "eval";
const char * const CMD_EXCLUDE	= "exclude";
const char * const CMD_EXEC		= "exec";
const char * const CMD_FINFO	= "file_info";
const char * const CMD_FIND		= "find";
const char * const CMD_FIXREAD	= "read_fixed";
const char * const CMD_FIXWRITE	= "write_fixed";
const char * const CMD_FLATTEN	= "flatten";
const char * const CMD_FMERGE	= "file_merge";
const char * const CMD_FROMXML	= "from_xml";
const char * const CMD_FSPLIT	= "file_split";
const char * const CMD_JOIN		= "join";
const char * const CMD_HEAD		= "head";
const char * const CMD_HELP		= "help";
const char * const CMD_INTER	= "inter";
const char * const CMD_LOWER	= "lower";
const char * const CMD_MERGE	= "merge";
const char * const CMD_MAP		= "map";
const char * const CMD_MIXED	= "mixed";
const char * const CMD_MONEY	= "money";
const char * const CMD_NUMBER	= "number";
const char * const CMD_ODBCGET	= "odbc_get";
const char * const CMD_ORDER	= "order";
const char * const CMD_PRINTF	= "printf";
const char * const CMD_PAD		= "pad";
const char * const CMD_PUT		= "put";
const char * const CMD_ESC		= "escape";
const char * const CMD_REMOVE	= "remove";
const char * const CMD_READMUL	= "read_multi";
const char * const CMD_RMNEW	= "rmnew";
const char * const CMD_SEQ		= "sequence";
const char * const CMD_SHUFFLE	= "shuffle";
const char * const CMD_SORT		= "sort";
const char * const CMD_SPLFIX	= "split_fixed";
const char * const CMD_SPLCHR	= "split_char";
const char * const CMD_SQLDEL	= "sql_delete";
const char * const CMD_SQLIN	= "sql_insert";
const char * const CMD_SQLUP	= "sql_update";
const char * const CMD_STAT		= "stat";
const char * const CMD_SUMMARY	= "summary";
const char * const CMD_TABLE	= "xml_table";
const char * const CMD_TAIL		= "tail";
const char * const CMD_TOXML	= "to_xml";
const char * const CMD_TPLATE	= "template";
const char * const CMD_TRIM		= "trim";
const char * const CMD_TRUNC	= "truncate";
const char * const CMD_TSTAMP	= "timestamp";
const char * const CMD_UPPER	= "upper";
const char * const CMD_UNFLATTEN = "unflatten";
const char * const CMD_UNIQUE	= "unique";
const char * const CMD_USAGE	= "usage";
const char * const CMD_VALID	= "validate";
const char * const CMD_WRITEMUL	= "write_multi";
const char * const CMD_WRITEXML	= "write_xml";
const char * const CMD_XMLR		= "read_xml";

//---------------------------------------------------------------------------
// Flag names
//---------------------------------------------------------------------------

const char * const FLAG_ARG		= "-arg";
const char * const FLAG_AVG		= "-avg";
const char * const FLAG_ACTKEEP	= "-k";
const char * const FLAG_ACTMARK	= "-m";
const char * const FLAG_ACTREM	= "-r";
const char * const FLAG_BASEN	= "-b";
const char * const FLAG_BDEXCL	= "-bdx";
const char * const FLAG_BDLIST	= "-bdl";
const char * const FLAG_BEXPR	= "-be";
const char * const FLAG_BLKEXC	= "-x";
const char * const FLAG_BLKMARK	= "-m";
const char * const FLAG_BSIZE	= "-bs";
const char * const FLAG_CDATE	= "-cy";
const char * const FLAG_CENTS	= "-cn";
const char * const FLAG_CHARS	= "-s";
const char * const FLAG_CHAR	= "-c";
const char * const FLAG_CMD		= "-c";
const char * const FLAG_COLS	= "-f";
const char * const FLAG_CMULTI	= "-cm";
const char * const FLAG_CONSTR	= "-cs";
const char * const FLAG_CURSYM	= "-cs";
const char * const FLAG_CSV		= "-csv";
const char * const FLAG_CSVSEP	= "-sep";
const char * const FLAG_CSVSEPR	= "-rsep";
const char * const FLAG_DUPES	= "-d";
const char * const FLAG_DATE	= "-d";
const char * const FLAG_DISCARD	= "-d";
const char * const FLAG_DETAIL	= "-d";
const char * const FLAG_DIR		= "-dir";
const char * const FLAG_DLL		= "-dll";
const char * const FLAG_DONLY	= "-d";
const char * const FLAG_DTIME 	= "-dt";
const char * const FLAG_DECR	= "-d";
const char * const FLAG_DPOINT	= "-dp";
const char * const FLAG_EDIT	= "-e";
const char * const FLAG_ENDTAG	= "-et";
const char * const FLAG_ENULLS	= "-en";
const char * const FLAG_EEXPR	= "-ee";
const char * const FLAG_ERRCODE = "-ec";
const char * const FLAG_ERRSTR = "-es";
const char * const FLAG_ESC		= "-e";
const char * const FLAG_ESCOFF	= "-noc";
const char * const FLAG_EXPR	= "-e";
const char * const FLAG_EXCLF	= "-xf";
const char * const FLAG_EXCLNL	= "-x";
const char * const FLAG_FCOUNT	= "-fc";
const char * const FLAG_FSEP	= "-fs";
const char * const FLAG_FILTBAD	= "-x";
const char * const FLAG_FMT		= "-fmt";
const char * const FLAG_FNAMES	= "-fn";
const char * const FLAG_FSPRE	= "-fp";
const char * const FLAG_FSEXT	= "-fx";
const char * const FLAG_FSDIR	= "-fd";
const char * const FLAG_FREQ	= "-frq";
const char * const FLAG_FUNC	= "-fnc";
const char * const FLAG_EXPRIC	= "-ei";
const char * const FLAG_FROMV	= "-fv";
const char * const FLAG_HEADER	= "-h";
const char * const FLAG_HEADEXP	= "-me";
const char * const FLAG_HDRREC	= "-hdr";
const char * const FLAG_ICASE	= "-ic";
const char * const FLAG_IF		= "-if";
const char * const FLAG_ICNAMES	= "-ifn";
const char * const FLAG_ILTEXT	= "-ilt";
const char * const FLAG_INC		= "-i";
const char * const FLAG_IND		= "-in";
const char * const FLAG_INVERT	= "-inv";
const char * const FLAG_IGNBL	= "-ibl";
const char * const FLAG_INDTAB	= "-it";
const char * const FLAG_ISPACE	= "-is";
const char * const FLAG_KEEP	= "-k";
const char * const FLAG_KEY		= "-k";
const char * const FLAG_KSEP	= "-ts";
const char * const FLAG_LEN		= "-l";
const char * const FLAG_MASK	= "-m";
const char * const FLAG_MASTER	= "-m";
const char * const FLAG_MEDIAN	= "-med";
const char * const FLAG_MAX		= "-max";
const char * const FLAG_MIN		= "-min";
const char * const FLAG_MINUS	= "-ms";
const char * const FLAG_MODE	= "-mod";
const char * const FLAG_MNAMES	= "-mn";
const char * const FLAG_NOQUOTE	= "-nq";
const char * const FLAG_NLOK	= "-nl";
const char * const FLAG_NOCREAT = "-nc";
const char * const FLAG_NOINFO	= "-ni";
const char * const FLAG_NUM		= "-n";
const char * const FLAG_NULLSTR	= "-ns";
const char * const FLAG_OMODE	= "-om";
const char * const FLAG_OUT		= "-o";
const char * const FLAG_OUTSEP	= "-osep";
const char * const FLAG_OUTERJ	= "-oj";
const char * const FLAG_PAD		= "-p";
const char * const FLAG_PADCHAR	= "-pc";
const char * const FLAG_PLUS	= "-ps";
const char * const FLAG_POS		= "-p";
const char * const FLAG_QLIST	= "-sqf";
const char * const FLAG_QNULLS	= "-qn";
const char * const FLAG_QUIET	= "-q";
const char * const FLAG_QUOTE	= "-q";
const char * const FLAG_REPLACE	= "-r";
const char * const FLAG_RHEAD	= "-rh";
const char * const FLAG_RANGE	= "-r";
const char * const FLAG_RFC		= "-rfc";
const char * const FLAG_REMOVE	= "-r";
const char * const FLAG_REVCOLS	= "-rf";
const char * const FLAG_RECSEP	= "-rs";
const char * const FLAG_RSEED	= "-rs";
const char * const FLAG_RFSEED	= "-seed";
const char * const FLAG_RTIME	= "-rt";
const char * const FLAG_RULER	= "-ru";
const char * const FLAG_RALIGN	= "-ra";
const char * const FLAG_SEP		= "-s";
const char * const FLAG_SIZE	= "-siz";
const char * const FLAG_SMARTQ	= "-smq";
const char * const FLAG_SQLQ	= "-sql";
const char * const FLAG_SUM		= "-sum";
const char * const FLAG_SQLSEP	= "-s";
const char * const FLAG_SUBS	= "-s";
const char * const FLAG_STR		= "-s";
const char * const FLAG_STRIC	= "-si";
const char * const FLAG_TABLE	= "-t";
const char * const FLAG_SQLTBL	= "-tbl";
const char * const FLAG_TFILE	= "-tf";
const char * const FLAG_TONLY	= "-t";
const char * const FLAG_TOV		= "-tv";
const char * const FLAG_TRLEAD	= "-l";
const char * const FLAG_TRTRAIL	= "-t";
const char * const FLAG_TWOC	= "-tc";
const char * const FLAG_USEFLD	= "-ufn";
const char * const FLAG_VAL		= "-v";
const char * const FLAG_VALENV	= "-e";
const char * const FLAG_VERBOSE	= "-v";
const char * const FLAG_VFILE	= "-vf";
const char * const FLAG_WHERE	= "-w";
const char * const FLAG_WIDTH	= "-w";

const char * const FLAG_XMLSPEC		= "-xf";
const char * const FLAG_XMLEREC		= "-re";
const char * const FLAG_XMLNOPAR	= "-np";
const char * const FLAG_XMLNOATTR	= "-na";
const char * const FLAG_XMLREMNL	= "-rn";
const char * const FLAG_XMLNOKIDS	= "-nc";
const char * const FLAG_XMLIPATH	= "-ip";
const char * const FLAG_XMLMLSEP	= "-ml";
const char * const FLAG_XMLEXCL		= "-ex";

//----------------------------------------------------------------------------
// Skip and pass options
//----------------------------------------------------------------------------

const char * const FLAG_SKIP		= "-skip";
const char * const FLAG_PASS		= "-pass";

//---------------------------------------------------------------------------
// Other stuff
//---------------------------------------------------------------------------

const char * const DISP_STDIN	= "<stdin>";// display name for stdin
const char * const NAME_STDIN	= "-";		// name for stdin on cmd line

} // end namespace

#endif

