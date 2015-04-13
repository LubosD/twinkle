/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_NUM = 258,
     T_TOKEN = 259,
     T_SAFE = 260,
     T_LINE = 261,
     T_CRLF = 262,
     T_LINE_VERSION = 263,
     T_LINE_ORIGIN = 264,
     T_LINE_SESSION_NAME = 265,
     T_LINE_CONNECTION = 266,
     T_LINE_ATTRIBUTE = 267,
     T_LINE_MEDIA = 268,
     T_LINE_UNKNOWN = 269,
     T_NULL = 270
   };
#endif
/* Tokens.  */
#define T_NUM 258
#define T_TOKEN 259
#define T_SAFE 260
#define T_LINE 261
#define T_CRLF 262
#define T_LINE_VERSION 263
#define T_LINE_ORIGIN 264
#define T_LINE_SESSION_NAME 265
#define T_LINE_CONNECTION 266
#define T_LINE_ATTRIBUTE 267
#define T_LINE_MEDIA 268
#define T_LINE_UNKNOWN 269
#define T_NULL 270




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 50 "sdp_parser.yxx"
{
	int			yysdpt_int;
	string			*yysdpt_str;
	t_sdp_ntwk_type		yysdpt_ntwk_type;
	t_sdp_addr_type		yysdpt_addr_type;
	t_sdp_connection	*yysdpt_connection;
	list<t_sdp_attr>	*yysdpt_attributes;
	t_sdp_attr		*yysdpt_attribute;
	t_sdp_media		*yysdpt_media;
	list<string>		*yysdpt_token_list;
}
/* Line 1529 of yacc.c.  */
#line 91 "sdp_parser.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yysdplval;

