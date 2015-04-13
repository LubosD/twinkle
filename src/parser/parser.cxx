/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_NUM = 258,
     T_TOKEN = 259,
     T_QSTRING = 260,
     T_COMMENT = 261,
     T_LINE = 262,
     T_URI = 263,
     T_URI_WILDCARD = 264,
     T_DISPLAY = 265,
     T_LANG = 266,
     T_WORD = 267,
     T_WKDAY = 268,
     T_MONTH = 269,
     T_GMT = 270,
     T_SIP = 271,
     T_METHOD = 272,
     T_AUTH_DIGEST = 273,
     T_AUTH_OTHER = 274,
     T_IPV6ADDR = 275,
     T_PARAMVAL = 276,
     T_HDR_ACCEPT = 277,
     T_HDR_ACCEPT_ENCODING = 278,
     T_HDR_ACCEPT_LANGUAGE = 279,
     T_HDR_ALERT_INFO = 280,
     T_HDR_ALLOW = 281,
     T_HDR_ALLOW_EVENTS = 282,
     T_HDR_AUTHENTICATION_INFO = 283,
     T_HDR_AUTHORIZATION = 284,
     T_HDR_CALL_ID = 285,
     T_HDR_CALL_INFO = 286,
     T_HDR_CONTACT = 287,
     T_HDR_CONTENT_DISP = 288,
     T_HDR_CONTENT_ENCODING = 289,
     T_HDR_CONTENT_LANGUAGE = 290,
     T_HDR_CONTENT_LENGTH = 291,
     T_HDR_CONTENT_TYPE = 292,
     T_HDR_CSEQ = 293,
     T_HDR_DATE = 294,
     T_HDR_ERROR_INFO = 295,
     T_HDR_EVENT = 296,
     T_HDR_EXPIRES = 297,
     T_HDR_FROM = 298,
     T_HDR_IN_REPLY_TO = 299,
     T_HDR_MAX_FORWARDS = 300,
     T_HDR_MIN_EXPIRES = 301,
     T_HDR_MIME_VERSION = 302,
     T_HDR_ORGANIZATION = 303,
     T_HDR_P_ASSERTED_IDENTITY = 304,
     T_HDR_P_PREFERRED_IDENTITY = 305,
     T_HDR_PRIORITY = 306,
     T_HDR_PRIVACY = 307,
     T_HDR_PROXY_AUTHENTICATE = 308,
     T_HDR_PROXY_AUTHORIZATION = 309,
     T_HDR_PROXY_REQUIRE = 310,
     T_HDR_RACK = 311,
     T_HDR_RECORD_ROUTE = 312,
     T_HDR_SERVICE_ROUTE = 313,
     T_HDR_REFER_SUB = 314,
     T_HDR_REFER_TO = 315,
     T_HDR_REFERRED_BY = 316,
     T_HDR_REPLACES = 317,
     T_HDR_REPLY_TO = 318,
     T_HDR_REQUIRE = 319,
     T_HDR_REQUEST_DISPOSITION = 320,
     T_HDR_RETRY_AFTER = 321,
     T_HDR_ROUTE = 322,
     T_HDR_RSEQ = 323,
     T_HDR_SERVER = 324,
     T_HDR_SIP_ETAG = 325,
     T_HDR_SIP_IF_MATCH = 326,
     T_HDR_SUBJECT = 327,
     T_HDR_SUBSCRIPTION_STATE = 328,
     T_HDR_SUPPORTED = 329,
     T_HDR_TIMESTAMP = 330,
     T_HDR_TO = 331,
     T_HDR_UNSUPPORTED = 332,
     T_HDR_USER_AGENT = 333,
     T_HDR_VIA = 334,
     T_HDR_WARNING = 335,
     T_HDR_WWW_AUTHENTICATE = 336,
     T_HDR_UNKNOWN = 337,
     T_CRLF = 338,
     T_ERROR = 339,
     T_NULL = 340
   };
#endif
/* Tokens.  */
#define T_NUM 258
#define T_TOKEN 259
#define T_QSTRING 260
#define T_COMMENT 261
#define T_LINE 262
#define T_URI 263
#define T_URI_WILDCARD 264
#define T_DISPLAY 265
#define T_LANG 266
#define T_WORD 267
#define T_WKDAY 268
#define T_MONTH 269
#define T_GMT 270
#define T_SIP 271
#define T_METHOD 272
#define T_AUTH_DIGEST 273
#define T_AUTH_OTHER 274
#define T_IPV6ADDR 275
#define T_PARAMVAL 276
#define T_HDR_ACCEPT 277
#define T_HDR_ACCEPT_ENCODING 278
#define T_HDR_ACCEPT_LANGUAGE 279
#define T_HDR_ALERT_INFO 280
#define T_HDR_ALLOW 281
#define T_HDR_ALLOW_EVENTS 282
#define T_HDR_AUTHENTICATION_INFO 283
#define T_HDR_AUTHORIZATION 284
#define T_HDR_CALL_ID 285
#define T_HDR_CALL_INFO 286
#define T_HDR_CONTACT 287
#define T_HDR_CONTENT_DISP 288
#define T_HDR_CONTENT_ENCODING 289
#define T_HDR_CONTENT_LANGUAGE 290
#define T_HDR_CONTENT_LENGTH 291
#define T_HDR_CONTENT_TYPE 292
#define T_HDR_CSEQ 293
#define T_HDR_DATE 294
#define T_HDR_ERROR_INFO 295
#define T_HDR_EVENT 296
#define T_HDR_EXPIRES 297
#define T_HDR_FROM 298
#define T_HDR_IN_REPLY_TO 299
#define T_HDR_MAX_FORWARDS 300
#define T_HDR_MIN_EXPIRES 301
#define T_HDR_MIME_VERSION 302
#define T_HDR_ORGANIZATION 303
#define T_HDR_P_ASSERTED_IDENTITY 304
#define T_HDR_P_PREFERRED_IDENTITY 305
#define T_HDR_PRIORITY 306
#define T_HDR_PRIVACY 307
#define T_HDR_PROXY_AUTHENTICATE 308
#define T_HDR_PROXY_AUTHORIZATION 309
#define T_HDR_PROXY_REQUIRE 310
#define T_HDR_RACK 311
#define T_HDR_RECORD_ROUTE 312
#define T_HDR_SERVICE_ROUTE 313
#define T_HDR_REFER_SUB 314
#define T_HDR_REFER_TO 315
#define T_HDR_REFERRED_BY 316
#define T_HDR_REPLACES 317
#define T_HDR_REPLY_TO 318
#define T_HDR_REQUIRE 319
#define T_HDR_REQUEST_DISPOSITION 320
#define T_HDR_RETRY_AFTER 321
#define T_HDR_ROUTE 322
#define T_HDR_RSEQ 323
#define T_HDR_SERVER 324
#define T_HDR_SIP_ETAG 325
#define T_HDR_SIP_IF_MATCH 326
#define T_HDR_SUBJECT 327
#define T_HDR_SUBSCRIPTION_STATE 328
#define T_HDR_SUPPORTED 329
#define T_HDR_TIMESTAMP 330
#define T_HDR_TO 331
#define T_HDR_UNSUPPORTED 332
#define T_HDR_USER_AGENT 333
#define T_HDR_VIA 334
#define T_HDR_WARNING 335
#define T_HDR_WWW_AUTHENTICATE 336
#define T_HDR_UNKNOWN 337
#define T_CRLF 338
#define T_ERROR 339
#define T_NULL 340




/* Copy the first part of user declarations.  */
#line 19 "parser.yxx"

#include <cstdio>
#include <cstdlib>
#include <string>
#include "media_type.h"
#include "parameter.h"
#include "parse_ctrl.h"
#include "request.h"
#include "response.h"
#include "util.h"
#include "audits/memman.h"

using namespace std;

extern int yylex(void);
void yyerror(const char *s);


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 49 "parser.yxx"
{
	int			yyt_int;
	ulong			yyt_ulong;
	float			yyt_float;
	string			*yyt_str;
	t_parameter		*yyt_param;
	list<t_parameter>	*yyt_params;
	t_media			*yyt_media;
	t_coding		*yyt_coding;
	t_language		*yyt_language;
	t_alert_param		*yyt_alert_param;
	t_info_param		*yyt_info_param;
	list<t_contact_param>	*yyt_contacts;
	t_contact_param		*yyt_contact;
	t_error_param		*yyt_error_param;
	t_identity		*yyt_from_addr;
	t_route			*yyt_route;
	t_server		*yyt_server;
	t_via			*yyt_via;
	t_warning		*yyt_warning;
	t_digest_response	*yyt_dig_resp;
	t_credentials		*yyt_credentials;
	t_digest_challenge	*yyt_dig_chlg;
	t_challenge		*yyt_challenge;
}
/* Line 193 of yacc.c.  */
#line 310 "parser.cxx"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 323 "parser.cxx"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   734

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  99
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  254
/* YYNRULES -- Number of rules.  */
#define YYNRULES  432
/* YYNRULES -- Number of states.  */
#define YYNSTATES  791

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   340

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      94,    95,     2,     2,    88,     2,    96,    86,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    87,    89,
      91,    90,    92,     2,    93,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    97,     2,    98,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    14,    18,    19,
      20,    27,    28,    33,    37,    38,    39,    40,    48,    49,
      52,    56,    60,    64,    68,    72,    76,    80,    84,    88,
      92,    96,   100,   104,   108,   112,   116,   120,   124,   128,
     132,   136,   140,   144,   148,   152,   156,   160,   164,   168,
     172,   176,   180,   184,   188,   192,   196,   200,   204,   208,
     212,   216,   220,   224,   228,   232,   236,   240,   244,   248,
     252,   256,   260,   264,   268,   272,   276,   280,   284,   288,
     292,   297,   301,   305,   309,   313,   317,   321,   325,   329,
     333,   337,   341,   345,   349,   353,   357,   361,   365,   369,
     373,   377,   381,   385,   389,   393,   397,   401,   405,   409,
     413,   417,   421,   425,   429,   433,   437,   441,   445,   449,
     453,   457,   461,   465,   469,   473,   477,   481,   485,   489,
     493,   497,   501,   505,   509,   513,   517,   521,   525,   529,
     533,   537,   540,   543,   546,   549,   552,   555,   558,   561,
     564,   567,   570,   573,   576,   579,   582,   585,   588,   591,
     594,   597,   600,   603,   606,   609,   612,   615,   618,   621,
     624,   627,   630,   633,   636,   639,   642,   645,   648,   651,
     654,   657,   660,   663,   666,   669,   672,   675,   678,   681,
     684,   687,   690,   693,   696,   699,   702,   705,   708,   711,
     714,   717,   718,   721,   726,   730,   731,   735,   737,   738,
     739,   745,   747,   749,   751,   755,   757,   760,   763,   764,
     767,   768,   773,   775,   776,   780,   782,   786,   787,   788,
     795,   797,   801,   802,   803,   807,   809,   813,   815,   819,
     820,   821,   828,   829,   830,   834,   836,   838,   842,   845,
     846,   847,   851,   852,   853,   854,   862,   863,   865,   867,
     870,   872,   876,   877,   880,   881,   886,   887,   888,   892,
     895,   896,   897,   902,   903,   904,   918,   920,   924,   925,
     926,   933,   934,   935,   939,   940,   944,   945,   948,   949,
     950,   957,   958,   959,   963,   964,   965,   971,   972,   973,
     977,   978,   979,   983,   985,   986,   987,   991,   992,   995,
     999,  1000,  1003,  1007,  1009,  1011,  1015,  1017,  1021,  1023,
    1027,  1028,  1029,  1037,  1039,  1043,  1044,  1045,  1050,  1051,
    1055,  1057,  1061,  1062,  1063,  1069,  1070,  1071,  1072,  1078,
    1080,  1084,  1086,  1089,  1091,  1094,  1099,  1100,  1101,  1105,
    1106,  1108,  1112,  1113,  1116,  1118,  1121,  1123,  1127,  1129,
    1133,  1134,  1138,  1140,  1144,  1146,  1149,  1151,  1155,  1159,
    1165,  1167,  1168,  1169,  1175,  1177,  1178,  1179,  1185,  1186,
    1187,  1193,  1195,  1199,  1200,  1201,  1207,  1208,  1209,  1213,
    1215,  1217,  1221,  1223,  1227,  1229,  1233,  1234,  1238,  1239,
    1243,  1244,  1247,  1249,  1253,  1254,  1258,  1259,  1263,  1264,
    1267,  1268,  1271,  1272,  1275,  1276,  1277,  1281,  1282,  1283,
    1289,  1292,  1294,  1298,  1301,  1302,  1306,  1307,  1311,  1314,
    1316,  1318,  1320
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     100,     0,    -1,    -1,   101,   102,    -1,   103,    -1,   109,
      -1,     1,    85,    -1,   104,   114,    83,    -1,    -1,    -1,
      17,   105,     8,   106,   107,    83,    -1,    -1,    16,   108,
      86,     4,    -1,   110,   114,    83,    -1,    -1,    -1,    -1,
     107,   111,     3,   112,     7,   113,    83,    -1,    -1,   114,
     115,    -1,   116,   176,    83,    -1,   117,   183,    83,    -1,
     118,   186,    83,    -1,   119,   191,    83,    -1,   120,   195,
      83,    -1,   121,   343,    83,    -1,   122,   318,    83,    -1,
     123,   324,    83,    -1,   124,   196,    83,    -1,   125,   200,
      83,    -1,   126,   204,    83,    -1,   127,   216,    83,    -1,
     128,   217,    83,    -1,   129,   218,    83,    -1,   130,   221,
      83,    -1,   131,   224,    83,    -1,   132,   225,    83,    -1,
     133,   228,    83,    -1,   135,   342,    83,    -1,   134,   231,
      83,    -1,   136,   235,    83,    -1,   137,   238,    83,    -1,
     138,   244,    83,    -1,   139,   249,    83,    -1,   140,   252,
      83,    -1,   141,   255,    83,    -1,   142,   256,    83,    -1,
     143,   259,    83,    -1,   144,   261,    83,    -1,   145,   263,
      83,    -1,   146,   264,    83,    -1,   147,   330,    83,    -1,
     148,   332,    83,    -1,   149,   265,    83,    -1,   150,   339,
      83,    -1,   151,   266,    83,    -1,   152,   349,    83,    -1,
     153,   345,    83,    -1,   154,   347,    83,    -1,   155,   271,
      83,    -1,   156,   274,    83,    -1,   157,   276,    83,    -1,
     158,   352,    83,    -1,   159,   277,    83,    -1,   160,   283,
      83,    -1,   161,   336,    83,    -1,   162,   284,    83,    -1,
     163,   270,    83,    -1,   164,   350,    83,    -1,   165,   351,
      83,    -1,   166,   286,    83,    -1,   167,   344,    83,    -1,
     168,   289,    83,    -1,   169,   290,    83,    -1,   170,   295,
      83,    -1,   171,   297,    83,    -1,   172,   298,    83,    -1,
     173,   299,    83,    -1,   174,   310,    83,    -1,   175,   334,
      83,    -1,    82,    87,   314,    83,    -1,   116,     1,    83,
      -1,   117,     1,    83,    -1,   118,     1,    83,    -1,   119,
       1,    83,    -1,   120,     1,    83,    -1,   121,     1,    83,
      -1,   122,     1,    83,    -1,   123,     1,    83,    -1,   124,
       1,    83,    -1,   125,     1,    83,    -1,   126,     1,    83,
      -1,   127,     1,    83,    -1,   128,     1,    83,    -1,   129,
       1,    83,    -1,   130,     1,    83,    -1,   131,     1,    83,
      -1,   132,     1,    83,    -1,   133,     1,    83,    -1,   134,
       1,    83,    -1,   135,     1,    83,    -1,   136,     1,    83,
      -1,   137,     1,    83,    -1,   138,     1,    83,    -1,   139,
       1,    83,    -1,   140,     1,    83,    -1,   141,     1,    83,
      -1,   142,     1,    83,    -1,   143,     1,    83,    -1,   144,
       1,    83,    -1,   145,     1,    83,    -1,   146,     1,    83,
      -1,   147,     1,    83,    -1,   148,     1,    83,    -1,   149,
       1,    83,    -1,   150,     1,    83,    -1,   151,     1,    83,
      -1,   152,     1,    83,    -1,   153,     1,    83,    -1,   154,
       1,    83,    -1,   155,     1,    83,    -1,   156,     1,    83,
      -1,   157,     1,    83,    -1,   158,     1,    83,    -1,   159,
       1,    83,    -1,   160,     1,    83,    -1,   161,     1,    83,
      -1,   162,     1,    83,    -1,   163,     1,    83,    -1,   164,
       1,    83,    -1,   165,     1,    83,    -1,   166,     1,    83,
      -1,   167,     1,    83,    -1,   168,     1,    83,    -1,   169,
       1,    83,    -1,   170,     1,    83,    -1,   171,     1,    83,
      -1,   172,     1,    83,    -1,   173,     1,    83,    -1,   174,
       1,    83,    -1,   175,     1,    83,    -1,    22,    87,    -1,
      23,    87,    -1,    24,    87,    -1,    25,    87,    -1,    26,
      87,    -1,    27,    87,    -1,    28,    87,    -1,    29,    87,
      -1,    30,    87,    -1,    31,    87,    -1,    32,    87,    -1,
      33,    87,    -1,    34,    87,    -1,    35,    87,    -1,    36,
      87,    -1,    37,    87,    -1,    38,    87,    -1,    39,    87,
      -1,    40,    87,    -1,    41,    87,    -1,    42,    87,    -1,
      43,    87,    -1,    44,    87,    -1,    45,    87,    -1,    46,
      87,    -1,    47,    87,    -1,    48,    87,    -1,    49,    87,
      -1,    50,    87,    -1,    51,    87,    -1,    52,    87,    -1,
      53,    87,    -1,    54,    87,    -1,    55,    87,    -1,    56,
      87,    -1,    57,    87,    -1,    59,    87,    -1,    60,    87,
      -1,    61,    87,    -1,    62,    87,    -1,    63,    87,    -1,
      64,    87,    -1,    65,    87,    -1,    66,    87,    -1,    67,
      87,    -1,    68,    87,    -1,    69,    87,    -1,    58,    87,
      -1,    70,    87,    -1,    71,    87,    -1,    72,    87,    -1,
      73,    87,    -1,    74,    87,    -1,    75,    87,    -1,    76,
      87,    -1,    77,    87,    -1,    78,    87,    -1,    79,    87,
      -1,    80,    87,    -1,    81,    87,    -1,    -1,   177,   178,
      -1,   176,    88,   177,   178,    -1,     4,    86,     4,    -1,
      -1,   178,    89,   179,    -1,     4,    -1,    -1,    -1,     4,
      90,   180,   182,   181,    -1,    21,    -1,     5,    -1,   184,
      -1,   183,    88,   184,    -1,     4,    -1,     4,   185,    -1,
      89,   179,    -1,    -1,   187,   189,    -1,    -1,   186,    88,
     188,   189,    -1,    11,    -1,    -1,    11,   190,   185,    -1,
     192,    -1,   191,    88,   192,    -1,    -1,    -1,    91,   193,
       8,   194,    92,   178,    -1,     4,    -1,   195,    88,     4,
      -1,    -1,    -1,   197,   199,   198,    -1,    12,    -1,    12,
      93,    12,    -1,   201,    -1,   200,    88,   201,    -1,    -1,
      -1,    91,   202,     8,   203,    92,   178,    -1,    -1,    -1,
     205,     9,   206,    -1,   207,    -1,   208,    -1,   207,    88,
     208,    -1,   209,   178,    -1,    -1,    -1,   210,     8,   211,
      -1,    -1,    -1,    -1,   212,   215,    91,   213,     8,   214,
      92,    -1,    -1,    10,    -1,     5,    -1,     4,   178,    -1,
     184,    -1,   217,    88,   184,    -1,    -1,   219,   189,    -1,
      -1,   218,    88,   220,   189,    -1,    -1,    -1,   222,     3,
     223,    -1,   177,   178,    -1,    -1,    -1,   226,     3,   227,
       4,    -1,    -1,    -1,   229,    13,    88,     3,    14,     3,
       3,    87,     3,    87,     3,    15,   230,    -1,   232,    -1,
     231,    88,   232,    -1,    -1,    -1,    91,   233,     8,   234,
      92,   178,    -1,    -1,    -1,   236,     3,   237,    -1,    -1,
     239,   240,   178,    -1,    -1,     8,   241,    -1,    -1,    -1,
     215,    91,   242,     8,   243,    92,    -1,    -1,    -1,   245,
     199,   246,    -1,    -1,    -1,   244,    88,   247,   199,   248,
      -1,    -1,    -1,   250,     3,   251,    -1,    -1,    -1,   253,
       3,   254,    -1,     4,    -1,    -1,    -1,   257,     7,   258,
      -1,    -1,   260,   240,    -1,   259,    88,   240,    -1,    -1,
     262,   240,    -1,   261,    88,   240,    -1,     4,    -1,     4,
      -1,   264,    89,     4,    -1,     4,    -1,   265,    88,     4,
      -1,   267,    -1,   266,    88,   267,    -1,    -1,    -1,   268,
     215,    91,     8,   269,    92,   178,    -1,   267,    -1,   270,
      88,   267,    -1,    -1,    -1,   272,   199,   273,   178,    -1,
      -1,   275,   240,   178,    -1,     4,    -1,   265,    88,     4,
      -1,    -1,    -1,   278,     3,   279,   280,   178,    -1,    -1,
      -1,    -1,    94,   281,     6,   282,    95,    -1,   267,    -1,
     283,    88,   267,    -1,   285,    -1,   284,   285,    -1,   280,
      -1,     4,   280,    -1,     4,    86,     4,   280,    -1,    -1,
      -1,   287,     7,   288,    -1,    -1,     4,    -1,   289,    88,
       4,    -1,    -1,   291,   292,    -1,   293,    -1,   293,   294,
      -1,     3,    -1,     3,    96,     3,    -1,     3,    -1,     3,
      96,     3,    -1,    -1,   296,   240,   178,    -1,     4,    -1,
     297,    88,     4,    -1,   285,    -1,   298,   285,    -1,   300,
      -1,   299,    88,   300,    -1,   301,   302,   178,    -1,     4,
      86,     4,    86,     4,    -1,     4,    -1,    -1,    -1,     4,
      87,   303,     3,   304,    -1,   307,    -1,    -1,    -1,   307,
      87,   305,     3,   306,    -1,    -1,    -1,    97,   308,    20,
     309,    98,    -1,   311,    -1,   310,    88,   311,    -1,    -1,
      -1,   312,     3,   313,   302,     5,    -1,    -1,    -1,   315,
       7,   316,    -1,   179,    -1,   317,    -1,   318,    88,   317,
      -1,   179,    -1,   319,    88,   179,    -1,   179,    -1,   320,
      88,   179,    -1,    -1,    18,   322,   319,    -1,    -1,    19,
     323,   320,    -1,    -1,   325,   321,    -1,   179,    -1,   326,
      88,   179,    -1,    -1,    18,   328,   326,    -1,    -1,    19,
     329,   320,    -1,    -1,   331,   327,    -1,    -1,   333,   321,
      -1,    -1,   335,   327,    -1,    -1,    -1,   337,     3,   338,
      -1,    -1,    -1,   340,     3,     3,   341,     4,    -1,     4,
     178,    -1,     4,    -1,   343,    88,     4,    -1,     4,   178,
      -1,    -1,   346,   240,   178,    -1,    -1,   348,   240,   178,
      -1,     4,   178,    -1,     4,    -1,     4,    -1,     4,    -1,
     352,    88,     4,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   244,   244,   244,   247,   248,   249,   269,   278,   278,
     278,   296,   296,   300,   307,   307,   308,   307,   318,   319,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   386,   388,   390,   392,   394,   396,   398,   400,   402,
     404,   406,   408,   410,   412,   414,   416,   418,   420,   422,
     424,   426,   428,   430,   432,   434,   436,   438,   440,   442,
     444,   446,   448,   450,   452,   454,   456,   458,   460,   462,
     464,   466,   468,   470,   472,   474,   476,   478,   480,   482,
     484,   486,   488,   490,   492,   494,   496,   498,   500,   502,
     504,   517,   519,   521,   523,   525,   527,   529,   531,   533,
     535,   537,   539,   541,   543,   545,   547,   549,   551,   553,
     555,   557,   559,   561,   563,   565,   567,   569,   571,   573,
     575,   577,   579,   581,   583,   585,   587,   589,   591,   593,
     595,   597,   599,   601,   603,   605,   607,   609,   611,   613,
     615,   617,   619,   621,   623,   625,   627,   629,   631,   633,
     635,   638,   639,   644,   651,   657,   658,   664,   668,   668,
     668,   675,   677,   681,   684,   689,   693,   700,   707,   707,
     710,   710,   715,   720,   720,   727,   730,   735,   735,   735,
     750,   753,   758,   758,   758,   763,   764,   771,   774,   779,
     779,   779,   794,   794,   794,   796,   801,   806,   812,   828,
     828,   828,   839,   839,   839,   839,   854,   855,   859,   862,
     878,   881,   886,   886,   889,   889,   894,   894,   894,   898,
     905,   905,   905,   911,   914,   911,   925,   928,   933,   933,
     933,   948,   948,   948,   952,   952,   967,   967,   978,   978,
     978,   993,   993,   993,   996,   996,   996,  1001,  1001,  1001,
    1005,  1005,  1005,  1009,  1014,  1014,  1014,  1019,  1019,  1022,
    1027,  1027,  1030,  1035,  1040,  1043,  1048,  1051,  1056,  1059,
    1064,  1064,  1064,  1082,  1085,  1090,  1090,  1090,  1112,  1112,
    1120,  1123,  1128,  1128,  1128,  1144,  1145,  1145,  1145,  1149,
    1152,  1157,  1160,  1165,  1170,  1177,  1188,  1188,  1188,  1193,
    1195,  1198,  1203,  1203,  1206,  1208,  1213,  1214,  1219,  1220,
    1225,  1225,  1240,  1243,  1248,  1251,  1256,  1259,  1264,  1292,
    1303,  1308,  1308,  1308,  1316,  1321,  1321,  1321,  1329,  1329,
    1329,  1336,  1339,  1344,  1344,  1344,  1355,  1355,  1355,  1358,
    1378,  1379,  1382,  1390,  1398,  1403,  1409,  1409,  1415,  1415,
    1424,  1424,  1429,  1437,  1445,  1445,  1451,  1451,  1460,  1460,
    1465,  1465,  1471,  1471,  1476,  1476,  1476,  1480,  1480,  1480,
    1487,  1501,  1504,  1509,  1535,  1535,  1543,  1543,  1558,  1569,
    1574,  1579,  1583
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_NUM", "T_TOKEN", "T_QSTRING",
  "T_COMMENT", "T_LINE", "T_URI", "T_URI_WILDCARD", "T_DISPLAY", "T_LANG",
  "T_WORD", "T_WKDAY", "T_MONTH", "T_GMT", "T_SIP", "T_METHOD",
  "T_AUTH_DIGEST", "T_AUTH_OTHER", "T_IPV6ADDR", "T_PARAMVAL",
  "T_HDR_ACCEPT", "T_HDR_ACCEPT_ENCODING", "T_HDR_ACCEPT_LANGUAGE",
  "T_HDR_ALERT_INFO", "T_HDR_ALLOW", "T_HDR_ALLOW_EVENTS",
  "T_HDR_AUTHENTICATION_INFO", "T_HDR_AUTHORIZATION", "T_HDR_CALL_ID",
  "T_HDR_CALL_INFO", "T_HDR_CONTACT", "T_HDR_CONTENT_DISP",
  "T_HDR_CONTENT_ENCODING", "T_HDR_CONTENT_LANGUAGE",
  "T_HDR_CONTENT_LENGTH", "T_HDR_CONTENT_TYPE", "T_HDR_CSEQ", "T_HDR_DATE",
  "T_HDR_ERROR_INFO", "T_HDR_EVENT", "T_HDR_EXPIRES", "T_HDR_FROM",
  "T_HDR_IN_REPLY_TO", "T_HDR_MAX_FORWARDS", "T_HDR_MIN_EXPIRES",
  "T_HDR_MIME_VERSION", "T_HDR_ORGANIZATION", "T_HDR_P_ASSERTED_IDENTITY",
  "T_HDR_P_PREFERRED_IDENTITY", "T_HDR_PRIORITY", "T_HDR_PRIVACY",
  "T_HDR_PROXY_AUTHENTICATE", "T_HDR_PROXY_AUTHORIZATION",
  "T_HDR_PROXY_REQUIRE", "T_HDR_RACK", "T_HDR_RECORD_ROUTE",
  "T_HDR_SERVICE_ROUTE", "T_HDR_REFER_SUB", "T_HDR_REFER_TO",
  "T_HDR_REFERRED_BY", "T_HDR_REPLACES", "T_HDR_REPLY_TO", "T_HDR_REQUIRE",
  "T_HDR_REQUEST_DISPOSITION", "T_HDR_RETRY_AFTER", "T_HDR_ROUTE",
  "T_HDR_RSEQ", "T_HDR_SERVER", "T_HDR_SIP_ETAG", "T_HDR_SIP_IF_MATCH",
  "T_HDR_SUBJECT", "T_HDR_SUBSCRIPTION_STATE", "T_HDR_SUPPORTED",
  "T_HDR_TIMESTAMP", "T_HDR_TO", "T_HDR_UNSUPPORTED", "T_HDR_USER_AGENT",
  "T_HDR_VIA", "T_HDR_WARNING", "T_HDR_WWW_AUTHENTICATE", "T_HDR_UNKNOWN",
  "T_CRLF", "T_ERROR", "T_NULL", "'/'", "':'", "','", "';'", "'='", "'<'",
  "'>'", "'@'", "'('", "')'", "'.'", "'['", "']'", "$accept",
  "sip_message", "@1", "sip_message2", "request", "request_line", "@2",
  "@3", "sip_version", "@4", "response", "status_line", "@5", "@6", "@7",
  "headers", "header", "hd_accept", "hd_accept_encoding",
  "hd_accept_language", "hd_alert_info", "hd_allow", "hd_allow_events",
  "hd_authentication_info", "hd_authorization", "hd_call_id",
  "hd_call_info", "hd_contact", "hd_content_disp", "hd_content_encoding",
  "hd_content_language", "hd_content_length", "hd_content_type", "hd_cseq",
  "hd_date", "hd_error_info", "hd_event", "hd_expires", "hd_from",
  "hd_in_reply_to", "hd_max_forwards", "hd_min_expires", "hd_mime_version",
  "hd_organization", "hd_p_asserted_identity", "hd_p_preferred_identity",
  "hd_priority", "hd_privacy", "hd_proxy_authenticate",
  "hd_proxy_authorization", "hd_proxy_require", "hd_rack",
  "hd_record_route", "hd_refer_sub", "hd_refer_to", "hd_referred_by",
  "hd_replaces", "hd_reply_to", "hd_require", "hd_request_disposition",
  "hd_retry_after", "hd_route", "hd_rseq", "hd_server", "hd_service_route",
  "hd_sip_etag", "hd_sip_if_match", "hd_subject", "hd_subscription_state",
  "hd_supported", "hd_timestamp", "hd_to", "hd_unsupported",
  "hd_user_agent", "hd_via", "hd_warning", "hd_www_authenticate",
  "hdr_accept", "media_range", "parameters", "parameter", "@8", "@9",
  "parameter_val", "hdr_accept_encoding", "content_coding", "q_factor",
  "hdr_accept_language", "@10", "@11", "language", "@12", "hdr_alert_info",
  "alert_param", "@13", "@14", "hdr_allow", "hdr_call_id", "@15", "@16",
  "call_id", "hdr_call_info", "info_param", "@17", "@18", "hdr_contact",
  "@19", "@20", "contacts", "contact_param", "contact_addr", "@21", "@22",
  "@23", "@24", "@25", "display_name", "hdr_content_disp",
  "hdr_content_encoding", "hdr_content_language", "@26", "@27",
  "hdr_content_length", "@28", "@29", "hdr_content_type", "hdr_cseq",
  "@30", "@31", "hdr_date", "@32", "@33", "hdr_error_info", "error_param",
  "@34", "@35", "hdr_expires", "@36", "@37", "hdr_from", "@38",
  "from_addr", "@39", "@40", "@41", "hdr_in_reply_to", "@42", "@43", "@44",
  "@45", "hdr_max_forwards", "@46", "@47", "hdr_min_expires", "@48", "@49",
  "hdr_mime_version", "hdr_organization", "@50", "@51",
  "hdr_p_asserted_identity", "@52", "hdr_p_preferred_identity", "@53",
  "hdr_priority", "hdr_privacy", "hdr_proxy_require", "hdr_record_route",
  "rec_route", "@54", "@55", "hdr_service_route", "hdr_replaces", "@56",
  "@57", "hdr_reply_to", "@58", "hdr_require", "hdr_retry_after", "@59",
  "@60", "comment", "@61", "@62", "hdr_route", "hdr_server", "server",
  "hdr_subject", "@63", "@64", "hdr_supported", "hdr_timestamp", "@65",
  "hdr_timestamp1", "timestamp", "delay", "hdr_to", "@66",
  "hdr_unsupported", "hdr_user_agent", "hdr_via", "via_parm",
  "sent_protocol", "host", "@67", "@68", "@69", "@70", "ipv6reference",
  "@71", "@72", "hdr_warning", "warning", "@73", "@74", "hdr_unknown",
  "@75", "@76", "ainfo", "hdr_authentication_info", "digest_response",
  "auth_params", "credentials", "@77", "@78", "hdr_authorization", "@79",
  "digest_challenge", "challenge", "@80", "@81", "hdr_proxy_authenticate",
  "@82", "hdr_proxy_authorization", "@83", "hdr_www_authenticate", "@84",
  "hdr_rseq", "@85", "@86", "hdr_rack", "@87", "@88", "hdr_event",
  "hdr_allow_events", "hdr_subscription_state", "hdr_refer_to", "@89",
  "hdr_referred_by", "@90", "hdr_refer_sub", "hdr_sip_etag",
  "hdr_sip_if_match", "hdr_request_disposition", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,    47,    58,    44,    59,
      61,    60,    62,    64,    40,    41,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,    99,   101,   100,   102,   102,   102,   103,   105,   106,
     104,   108,   107,   109,   111,   112,   113,   110,   114,   114,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   176,   176,   177,   178,   178,   179,   180,   181,
     179,   182,   182,   183,   183,   184,   184,   185,   187,   186,
     188,   186,   189,   190,   189,   191,   191,   193,   194,   192,
     195,   195,   197,   198,   196,   199,   199,   200,   200,   202,
     203,   201,   205,   206,   204,   204,   207,   207,   208,   210,
     211,   209,   212,   213,   214,   209,   215,   215,   215,   216,
     217,   217,   219,   218,   220,   218,   222,   223,   221,   224,
     226,   227,   225,   229,   230,   228,   231,   231,   233,   234,
     232,   236,   237,   235,   239,   238,   241,   240,   242,   243,
     240,   245,   246,   244,   247,   248,   244,   250,   251,   249,
     253,   254,   252,   255,   257,   258,   256,   260,   259,   259,
     262,   261,   261,   263,   264,   264,   265,   265,   266,   266,
     268,   269,   267,   270,   270,   272,   273,   271,   275,   274,
     276,   276,   278,   279,   277,   280,   281,   282,   280,   283,
     283,   284,   284,   285,   285,   285,   287,   288,   286,   289,
     289,   289,   291,   290,   292,   292,   293,   293,   294,   294,
     296,   295,   297,   297,   298,   298,   299,   299,   300,   301,
     302,   303,   304,   302,   302,   305,   306,   302,   308,   309,
     307,   310,   310,   312,   313,   311,   315,   316,   314,   317,
     318,   318,   319,   319,   320,   320,   322,   321,   323,   321,
     325,   324,   326,   326,   328,   327,   329,   327,   331,   330,
     333,   332,   335,   334,   337,   338,   336,   340,   341,   339,
     342,   343,   343,   344,   346,   345,   348,   347,   349,   350,
     351,   352,   352
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     2,     3,     0,     0,
       6,     0,     4,     3,     0,     0,     0,     7,     0,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       4,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     0,     2,     4,     3,     0,     3,     1,     0,     0,
       5,     1,     1,     1,     3,     1,     2,     2,     0,     2,
       0,     4,     1,     0,     3,     1,     3,     0,     0,     6,
       1,     3,     0,     0,     3,     1,     3,     1,     3,     0,
       0,     6,     0,     0,     3,     1,     1,     3,     2,     0,
       0,     3,     0,     0,     0,     7,     0,     1,     1,     2,
       1,     3,     0,     2,     0,     4,     0,     0,     3,     2,
       0,     0,     4,     0,     0,    13,     1,     3,     0,     0,
       6,     0,     0,     3,     0,     3,     0,     2,     0,     0,
       6,     0,     0,     3,     0,     0,     5,     0,     0,     3,
       0,     0,     3,     1,     0,     0,     3,     0,     2,     3,
       0,     2,     3,     1,     1,     3,     1,     3,     1,     3,
       0,     0,     7,     1,     3,     0,     0,     4,     0,     3,
       1,     3,     0,     0,     5,     0,     0,     0,     5,     1,
       3,     1,     2,     1,     2,     4,     0,     0,     3,     0,
       1,     3,     0,     2,     1,     2,     1,     3,     1,     3,
       0,     3,     1,     3,     1,     2,     1,     3,     3,     5,
       1,     0,     0,     5,     1,     0,     0,     5,     0,     0,
       5,     1,     3,     0,     0,     5,     0,     0,     3,     1,
       1,     3,     1,     3,     1,     3,     0,     3,     0,     3,
       0,     2,     1,     3,     0,     3,     0,     3,     0,     2,
       0,     2,     0,     2,     0,     0,     3,     0,     0,     5,
       2,     1,     3,     2,     0,     3,     0,     3,     2,     1,
       1,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     0,     1,     0,    11,     8,     3,     4,    18,
      14,     5,    18,     6,     0,     0,     0,     0,     0,     0,
       9,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     7,    19,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    13,    12,     0,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   188,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   386,     0,
       0,     0,   205,     0,   215,     0,   213,     0,     0,     0,
       0,   227,     0,   225,     0,   230,     0,     0,   421,     0,
       0,   207,   389,   390,     0,     0,     0,     0,     0,     0,
       0,     0,   239,     0,   237,     0,     0,     0,   245,   246,
     205,     0,   256,     0,   205,     0,     0,   260,     0,     0,
       0,     0,     0,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,   278,     0,   276,     0,   205,
       0,     0,     0,     0,     0,     0,   256,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   303,     0,     0,
       0,     0,     0,     0,   256,     0,     0,   256,     0,   313,
       0,     0,   314,     0,     0,     0,     0,     0,     0,     0,
       0,   316,     0,     0,     0,     0,     0,     0,   318,   256,
       0,   205,     0,     0,     0,   256,     0,     0,   256,     0,
       0,     0,     0,     0,   256,     0,   316,     0,     0,     0,
     431,     0,     0,     0,     0,     0,   339,     0,     0,     0,
       0,     0,   335,   336,   343,     0,   341,     0,   323,     0,
       0,   429,     0,     0,   430,     0,     0,     0,     0,     0,
     205,     0,     0,   350,     0,     0,     0,     0,     0,     0,
     256,     0,   362,     0,     0,   364,     0,     0,     0,     0,
     366,     0,     0,     0,   381,     0,     0,     0,     0,     0,
       0,     0,     0,    81,     0,    20,     0,   202,    82,     0,
     216,    21,     0,    83,    22,   220,   222,   219,    84,     0,
      23,     0,    85,    24,     0,    86,    25,     0,    87,   208,
      26,     0,    88,    27,   396,   398,   401,    89,    28,   235,
     233,    90,     0,    29,     0,    91,    30,   243,   252,   248,
     250,   258,   257,     0,    92,   259,    31,    93,    32,     0,
      94,    33,   264,   263,    95,    34,   267,    96,   269,    35,
      97,    36,   271,    98,    37,     0,    99,     0,    39,     0,
     100,   420,    38,   101,    40,   282,   102,    41,   286,     0,
     205,   103,    42,   294,   292,   104,    43,   298,   105,    44,
     301,   106,    45,   107,    46,   305,   108,    47,   256,   308,
     109,    48,   256,   311,   110,    49,   111,    50,     0,   112,
      51,   404,   406,   409,   113,    52,   411,   114,    53,     0,
     115,    54,     0,   116,    55,   320,     0,   117,   428,    56,
     118,    57,   205,   119,    58,   205,   120,    59,   326,   121,
      60,   205,   122,     0,    61,   123,    62,     0,   124,    63,
     333,   125,    64,   320,   126,    65,   415,   127,     0,   344,
       0,    66,   342,   128,    67,   320,   129,    68,   130,    69,
     131,    70,   347,   132,   423,    71,   133,    72,     0,   134,
      73,   356,   353,   354,   135,    74,   205,   136,    75,     0,
     137,    76,   365,   138,     0,    77,     0,   370,   378,   205,
     374,   139,    78,   383,   384,   140,    79,   413,    16,    10,
      80,   387,   204,   205,     0,   217,   214,     0,     0,   228,
     226,   231,   422,     0,   391,     0,     0,     0,   234,   240,
     238,   244,   247,   251,   253,   261,     0,   268,     0,     0,
     279,   277,   283,   287,   288,   285,     0,   293,   299,   302,
     306,   309,   312,   315,     0,     0,   317,   418,   319,     0,
     425,   427,   205,   329,   317,   432,   335,   340,   416,   335,
     337,   324,   348,   351,     0,   358,   355,   361,   363,     0,
     367,   371,     0,   368,   375,   382,     0,     0,   388,   203,
     206,   221,   224,     0,   212,   211,   209,   392,   397,   394,
     399,   236,     0,     0,   265,   272,     0,     0,     0,   295,
     402,   405,   407,     0,   321,   327,   205,   345,     0,   357,
       0,     0,     0,   379,     0,     0,    17,   205,   210,     0,
       0,   205,   254,     0,   205,   289,   296,     0,   419,     0,
     334,   338,   359,   369,   372,     0,   376,   385,   229,   393,
     395,   241,     0,     0,   280,     0,   403,   205,   373,   380,
     377,   255,     0,   290,   322,     0,     0,     0,     0,   274,
     275
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     7,     8,     9,    15,   147,    10,    14,
      11,    12,    17,   409,   707,    16,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   211,   212,   417,
     232,   643,   748,   716,   215,   216,   420,   218,   219,   637,
     427,   638,   222,   223,   429,   713,   226,   239,   240,   648,
     450,   243,   244,   452,   722,   246,   247,   651,   248,   249,
     250,   251,   653,   252,   723,   772,   499,   255,   258,   260,
     261,   656,   263,   264,   657,   267,   269,   270,   658,   272,
     273,   790,   276,   277,   487,   727,   282,   283,   662,   285,
     286,   500,   663,   728,   775,   288,   289,   667,   666,   756,
     291,   292,   668,   294,   295,   669,   298,   300,   301,   670,
     303,   304,   306,   307,   310,   313,   322,   327,   328,   329,
     759,   369,   340,   341,   682,   343,   344,   348,   353,   354,
     686,   364,   580,   738,   357,   365,   366,   377,   378,   692,
     384,   386,   387,   602,   603,   696,   389,   390,   393,   396,
     399,   400,   401,   619,   742,   778,   744,   780,   620,   702,
     765,   403,   404,   405,   706,   411,   412,   708,   233,   234,
     718,   720,   446,   645,   646,   236,   237,   731,   533,   674,
     675,   315,   316,   318,   319,   407,   408,   359,   360,   688,
     324,   325,   733,   280,   229,   381,   334,   335,   337,   338,
     332,   372,   375,   351
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -413
static const yytype_int16 yypact[] =
{
    -413,    90,    99,  -413,   -37,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,    18,   119,   506,   133,   568,   152,
    -413,    77,   107,   139,   145,   168,   177,   194,   198,   199,
     200,   201,   202,   203,   204,   206,   208,   209,   210,   211,
     216,   217,   221,   222,   223,   225,   228,   229,   230,   231,
     232,   233,   234,   239,   240,   244,   254,   255,   256,   259,
     260,   261,   262,   263,   266,   270,   273,   274,   277,   278,
     280,   281,   283,   284,   285,   286,   287,   288,   289,   291,
     294,   295,  -413,  -413,    58,   147,   120,    20,   205,   207,
     212,    67,    81,    34,    17,   213,   214,   123,   257,   219,
     258,    64,    37,   220,   264,    23,    95,   267,   268,   226,
      88,    29,    42,   227,   235,    73,   100,   236,   271,    31,
     237,    48,    56,   111,    62,   241,   242,   272,    70,   275,
      11,    78,   243,   247,   128,   248,    83,   276,    68,   252,
      16,   253,   279,   125,  -413,  -413,  -413,    87,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,   136,
     215,    66,  -413,   224,   245,    69,  -413,   246,    72,   167,
     269,  -413,    75,  -413,   301,  -413,    79,   302,  -413,    82,
     303,   238,  -413,  -413,    85,   304,   305,   164,   306,   307,
     282,   308,  -413,    89,  -413,   309,   310,   290,   218,  -413,
    -413,   292,   140,   311,  -413,   312,   314,  -413,    91,   315,
      92,   167,   316,   317,   319,   318,  -413,   320,   321,   322,
     327,   323,   324,   300,   325,  -413,    93,  -413,   326,  -413,
     328,   329,   330,   332,   331,   333,   132,   334,   101,   282,
     335,   337,   336,   338,   339,   341,   340,  -413,   342,   343,
     344,   347,   345,   102,   132,   346,   103,   132,   348,  -413,
     350,   351,  -413,    49,   352,   353,   265,   354,   355,   164,
     356,  -413,   104,   357,   358,   359,   360,   105,  -413,   140,
     361,  -413,   362,   363,   365,   132,   366,   367,   132,   368,
     369,   282,   370,   371,   132,   372,   373,   374,   375,   377,
    -413,   112,   378,   380,   399,   381,  -413,   113,   382,   383,
     407,   384,   -40,  -413,  -413,    15,  -413,   385,  -413,   114,
     386,  -413,   387,   388,  -413,   389,   390,   391,   408,   392,
    -413,   393,   394,  -413,   115,   395,   396,   421,   397,   398,
     132,   400,  -413,   116,   401,  -413,    19,   402,   403,   122,
    -413,     9,   404,   124,  -413,   427,   405,   409,   265,   435,
     410,   411,   450,  -413,   478,  -413,   486,   251,  -413,   487,
    -413,  -413,   491,  -413,  -413,  -413,   413,  -413,  -413,   488,
    -413,   412,  -413,  -413,   494,  -413,  -413,   495,  -413,  -413,
    -413,   487,  -413,  -413,  -413,  -413,  -413,  -413,  -413,   414,
    -413,  -413,   492,  -413,   415,  -413,  -413,  -413,   497,   251,
    -413,  -413,  -413,   417,  -413,   251,  -413,  -413,  -413,   491,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,   251,  -413,
    -413,  -413,  -413,  -413,  -413,   422,  -413,   501,  -413,   420,
    -413,   251,  -413,  -413,  -413,  -413,  -413,  -413,  -413,   423,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,   132,  -413,
    -413,  -413,   132,  -413,  -413,  -413,  -413,  -413,   508,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,   509,
    -413,  -413,   512,  -413,  -413,  -413,   425,  -413,   251,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,   513,  -413,  -413,  -413,   514,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,   515,  -413,
     349,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,   251,  -413,  -413,  -413,   516,  -413,
    -413,   196,  -413,   518,  -413,  -413,  -413,  -413,  -413,   519,
    -413,  -413,  -413,  -413,   520,  -413,   521,   439,  -413,  -413,
     565,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,   487,  -413,  -413,   167,   245,  -413,
    -413,  -413,  -413,    39,  -413,   487,   487,   510,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,   167,  -413,   649,   651,
    -413,  -413,  -413,  -413,  -413,   251,   282,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,   487,   487,  -413,  -413,  -413,   647,
     251,   251,  -413,   251,   573,  -413,   563,  -413,  -413,   563,
    -413,  -413,  -413,  -413,   655,   564,  -413,   251,  -413,   575,
    -413,  -413,   639,   251,  -413,  -413,     9,   579,  -413,   251,
    -413,  -413,  -413,   571,  -413,  -413,  -413,  -413,   576,  -413,
     577,  -413,   574,   659,  -413,  -413,   654,   578,   661,  -413,
    -413,   583,   577,   668,  -413,   251,  -413,  -413,   580,  -413,
     670,   672,   671,  -413,   674,   673,  -413,  -413,  -413,   487,
     487,  -413,  -413,   676,  -413,  -413,  -413,   487,  -413,   588,
     251,  -413,  -413,  -413,  -413,   584,  -413,  -413,   251,  -413,
    -413,   251,   589,   680,   251,   592,  -413,  -413,  -413,  -413,
    -413,  -413,   598,  -413,   251,   683,   600,   685,   299,  -413,
    -413
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,   176,  -413,
    -413,  -413,  -413,  -413,  -413,   677,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,   -91,  -250,
    -412,  -413,  -413,  -413,  -413,   -90,  -314,  -413,  -413,  -413,
    -260,  -413,  -413,   -80,  -413,  -413,  -413,  -413,  -413,  -413,
    -286,  -413,   -96,  -413,  -413,  -413,  -413,  -413,  -413,   -99,
    -413,  -413,  -413,  -413,  -413,  -413,  -212,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -123,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -293,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,   566,  -413,  -126,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -353,  -413,  -413,  -413,  -413,  -130,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -253,  -413,   -16,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,    71,  -413,  -413,  -413,  -413,  -413,   293,  -413,
    -413,    21,   376,  -413,  -413,  -413,  -413,  -413,   296,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -427
static const yytype_int16 yytable[] =
{
     459,   473,   356,   504,   465,   368,   257,   635,   266,   579,
     395,   519,   361,   617,   523,   362,   478,   394,   245,   362,
     362,   220,  -252,   362,   284,  -249,  -242,  -252,  -284,   491,
     302,  -284,   326,  -284,  -307,   241,  -320,  -307,   274,  -307,
     463,  -320,   552,   305,   714,   555,   578,  -310,    13,   333,
    -310,   561,  -310,  -424,   363,   558,  -424,   336,  -424,   209,
     715,  -426,   210,   342,  -426,   271,  -426,  -328,   235,   388,
    -328,   355,  -328,  -360,   314,  -320,  -360,  -273,  -360,   367,
    -320,   548,   238,  -320,   382,  -400,  -400,   383,  -320,   299,
       3,  -408,  -408,  -232,  -335,  -304,   287,   606,   581,  -335,
       4,   317,   611,     5,    19,   363,   618,  -291,  -252,   363,
     363,   221,   339,   363,  -284,     5,     6,   546,  -410,  -410,
    -307,   217,  -320,  -325,   259,   242,   406,    20,   275,   376,
     594,  -218,   527,  -310,  -262,  -346,   144,   461,   528,  -424,
     498,  -201,   462,  -412,  -412,   461,  -201,  -426,   213,   415,
     462,   214,   421,  -328,   416,   424,   146,   422,   430,  -360,
     425,  -320,   433,   431,   148,   436,  -349,   434,   440,  -320,
     437,  -349,   453,   441,   468,   471,   488,   454,   426,   469,
     472,   489,   444,   445,   502,   517,   521,   538,   544,   503,
     518,   522,   539,   545,   149,   566,   572,   584,   597,   608,
     567,   573,   585,   598,   609,   615,   224,   622,   227,   225,
     616,   228,   623,   230,   253,   256,   231,   254,   214,   413,
     265,   278,   710,   210,   279,   671,   150,   296,   308,   672,
     297,   309,   151,   717,   719,   582,   311,   320,   330,   312,
     321,   331,   345,   349,   370,   346,   350,   371,   373,   379,
     665,   374,   380,   391,   397,   152,   392,   398,   262,   268,
    -266,  -270,   730,   719,   153,   281,   612,  -281,   290,   293,
    -297,  -300,   323,   352,  -417,  -332,   358,   385,  -414,  -352,
     402,   154,  -383,   531,   532,   155,   156,   157,   158,   159,
     160,   161,   694,   162,   449,   163,   164,   165,   166,   457,
     460,   414,   680,   167,   168,   681,   458,   418,   169,   170,
     171,   683,   172,   485,   789,   173,   174,   175,   176,   177,
     178,   179,   476,   410,   712,   633,   180,   181,   439,   423,
     482,   182,   636,   736,   419,   495,   737,   769,   770,   507,
     634,   183,   184,   185,   510,   776,   186,   187,   188,   189,
     190,   640,   428,   191,   515,   690,   697,   192,   650,   652,
     193,   194,   542,   700,   195,   196,   661,   197,   198,   703,
     199,   200,   201,   202,   203,   204,   205,   711,   206,   655,
     729,   207,   208,   709,   432,   435,   438,   442,   443,   447,
     448,   451,   455,   456,   464,   466,   724,   467,   470,   474,
     475,   477,   570,   479,   480,   481,   483,   484,   486,   490,
     576,   492,   493,   494,   496,   592,   497,   501,   505,   678,
     506,   508,   509,   511,   601,   512,   513,   514,   516,   520,
     624,   524,   735,   525,   526,   529,   530,   534,   535,   537,
     540,   541,   628,   543,   547,   549,   550,   687,   551,   553,
     554,   556,   557,   559,   560,   562,  -330,   631,   564,   691,
     565,   568,   563,   569,   571,   574,   575,   577,   583,   586,
     587,   588,   589,   590,   591,   593,   595,   596,   599,   600,
     604,   605,   632,   607,   610,   613,   760,   621,   625,   614,
     210,   231,   626,   629,   630,   214,   639,   768,   641,   642,
     649,   771,  -223,   221,   774,  -249,   242,   647,   654,   660,
     659,   275,   673,   676,   664,   677,   679,   684,   685,   689,
     693,   695,   721,   698,   699,   398,   701,   784,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,   145,   704,   725,   726,   734,  -331,   363,   739,   743,
     740,   741,   746,   747,   749,   750,   751,   752,   753,   755,
     754,   757,   758,   762,   764,   761,   763,   766,   767,   773,
     777,   781,   779,   782,   783,   785,   786,   787,   788,    18,
     745,   347,     0,     0,   705,   536,   732,     0,     0,     0,
       0,     0,     0,     0,   627,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   644
};

static const yytype_int16 yycheck[] =
{
     250,   261,   128,   289,   254,   131,    96,   419,    99,   362,
     140,   304,     1,     4,   307,     4,   266,     1,     1,     4,
       4,     1,     5,     4,     1,     8,     9,    10,     5,   279,
       1,     8,     1,    10,     5,     1,     5,     8,     1,    10,
     252,    10,   335,     1,     5,   338,    86,     5,    85,     1,
       8,   344,    10,     5,    94,   341,     8,     1,    10,     1,
      21,     5,     4,     1,     8,     1,    10,     5,     1,     1,
       8,     1,    10,     5,     1,     5,     8,    13,    10,     1,
      10,   331,     1,     5,     1,    18,    19,     4,    10,     1,
       0,    18,    19,    12,    83,     7,     1,   390,    83,    83,
       1,     1,    83,    16,    86,    94,    97,    12,    91,    94,
      94,    91,     1,    94,    91,    16,    17,   329,    18,    19,
      91,     1,    91,    12,     1,    91,     1,     8,    91,     1,
     380,    11,    83,    91,    11,     7,     3,     5,    89,    91,
       8,    83,    10,    18,    19,     5,    88,    91,     1,    83,
      10,     4,    83,    91,    88,    83,     4,    88,    83,    91,
      88,    91,    83,    88,    87,    83,    83,    88,    83,    91,
      88,    88,    83,    88,    83,    83,    83,    88,    11,    88,
      88,    88,    18,    19,    83,    83,    83,    83,    83,    88,
      88,    88,    88,    88,    87,    83,    83,    83,    83,    83,
      88,    88,    88,    88,    88,    83,     1,    83,     1,     4,
      88,     4,    88,     1,     1,     1,     4,     4,     4,    83,
       1,     1,   634,     4,     4,   518,    87,     1,     1,   522,
       4,     4,    87,   645,   646,   365,     1,     1,     1,     4,
       4,     4,     1,     1,     1,     4,     4,     4,     1,     1,
     500,     4,     4,     1,     1,    87,     4,     4,     1,     1,
       3,     3,   674,   675,    87,     1,   396,     3,     1,     1,
       3,     3,     1,     1,     3,     3,     1,     1,     3,     3,
       1,    87,     3,    18,    19,    87,    87,    87,    87,    87,
      87,    87,    96,    87,    12,    87,    87,    87,    87,     9,
       8,    86,   552,    87,    87,   555,    88,    83,    87,    87,
      87,   561,    87,    13,    15,    87,    87,    87,    87,    87,
      87,    87,     3,   147,   638,   416,    87,    87,    90,    83,
       3,    87,   422,   686,    89,     3,   689,   749,   750,     3,
      89,    87,    87,    87,     3,   757,    87,    87,    87,    87,
      87,   431,    83,    87,     7,     6,   606,    87,   454,   458,
      87,    87,     3,   616,    87,    87,   489,    87,    87,   619,
      87,    87,    87,    87,    87,    87,    87,   637,    87,   469,
     666,    87,    87,   633,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,   656,    83,    83,    83,
      83,    83,     3,    83,    83,    83,    83,    83,    83,    83,
       3,    83,    83,    83,    83,     7,    83,    83,    83,   545,
      83,    83,    83,    83,     3,    83,    83,    83,    83,    83,
       3,    83,   682,    83,    83,    83,    83,    83,    83,    83,
      83,    83,     7,    83,    83,    83,    83,   573,    83,    83,
      83,    83,    83,    83,    83,    83,    83,     7,    83,   585,
      83,    83,    88,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,     4,    83,    83,    83,   736,    83,    83,    86,
       4,     4,    83,    83,    83,     4,     8,   747,     4,     4,
       8,   751,    89,    91,   754,     8,    91,    93,    91,     8,
      88,    91,     4,     4,    91,     3,    91,     4,     4,     4,
       4,     3,    12,     4,     4,     4,    87,   777,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    87,     4,     3,     8,    83,    94,     3,    20,
      96,    86,    83,    92,    88,    88,    92,     8,    14,     8,
      92,    88,     4,     3,     3,    95,     4,     3,     5,     3,
      92,    92,    98,     3,    92,    87,     3,    87,     3,    12,
     706,   125,    -1,    -1,   623,   319,   675,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   408,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   441
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   100,   101,     0,     1,    16,    17,   102,   103,   104,
     107,   109,   110,    85,   108,   105,   114,   111,   114,    86,
       8,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,     3,    83,     4,   106,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,     1,
       4,   176,   177,     1,     4,   183,   184,     1,   186,   187,
       1,    91,   191,   192,     1,     4,   195,     1,     4,   343,
       1,     4,   179,   317,   318,     1,   324,   325,     1,   196,
     197,     1,    91,   200,   201,     1,   204,   205,   207,   208,
     209,   210,   212,     1,     4,   216,     1,   184,   217,     1,
     218,   219,     1,   221,   222,     1,   177,   224,     1,   225,
     226,     1,   228,   229,     1,    91,   231,   232,     1,     4,
     342,     1,   235,   236,     1,   238,   239,     1,   244,   245,
       1,   249,   250,     1,   252,   253,     1,     4,   255,     1,
     256,   257,     1,   259,   260,     1,   261,   262,     1,     4,
     263,     1,     4,   264,     1,   330,   331,     1,   332,   333,
       1,     4,   265,     1,   339,   340,     1,   266,   267,   268,
       1,     4,   349,     1,   345,   346,     1,   347,   348,     1,
     271,   272,     1,   274,   275,     1,     4,   265,   276,     1,
       4,   352,     1,   277,   278,     1,   267,   283,     1,   336,
     337,     1,     4,    94,   280,   284,   285,     1,   267,   270,
       1,     4,   350,     1,     4,   351,     1,   286,   287,     1,
       4,   344,     1,     4,   289,     1,   290,   291,     1,   295,
     296,     1,     4,   297,     1,   285,   298,     1,     4,   299,
     300,   301,     1,   310,   311,   312,     1,   334,   335,   112,
     107,   314,   315,    83,    86,    83,    88,   178,    83,    89,
     185,    83,    88,    83,    83,    88,    11,   189,    83,   193,
      83,    88,    83,    83,    88,    83,    83,    88,    83,    90,
      83,    88,    83,    83,    18,    19,   321,    83,    83,    12,
     199,    83,   202,    83,    88,    83,    83,     9,    88,   178,
       8,     5,    10,   215,    83,   178,    83,    83,    83,    88,
      83,    83,    88,   189,    83,    83,     3,    83,   178,    83,
      83,    83,     3,    83,    83,    13,    83,   233,    83,    88,
      83,   178,    83,    83,    83,     3,    83,    83,     8,   215,
     240,    83,    83,    88,   199,    83,    83,     3,    83,    83,
       3,    83,    83,    83,    83,     7,    83,    83,    88,   240,
      83,    83,    88,   240,    83,    83,    83,    83,    89,    83,
      83,    18,    19,   327,    83,    83,   321,    83,    83,    88,
      83,    83,     3,    83,    83,    88,   215,    83,   178,    83,
      83,    83,   240,    83,    83,   240,    83,    83,   199,    83,
      83,   240,    83,    88,    83,    83,    83,    88,    83,    83,
       3,    83,    83,    88,    83,    83,     3,    83,    86,   280,
     281,    83,   285,    83,    83,    88,    83,    83,    83,    83,
      83,    83,     7,    83,   178,    83,    83,    83,    88,    83,
      83,     3,   292,   293,    83,    83,   240,    83,    83,    88,
      83,    83,   285,    83,    86,    83,    88,     4,    97,   302,
     307,    83,    83,    88,     3,    83,    83,   327,     7,    83,
      83,     7,     4,   177,    89,   179,   184,   188,   190,     8,
     192,     4,     4,   180,   317,   322,   323,    93,   198,     8,
     201,   206,   208,   211,    91,   184,   220,   223,   227,    88,
       8,   232,   237,   241,    91,   178,   247,   246,   251,   254,
     258,   240,   240,     4,   328,   329,     4,     3,   267,    91,
     178,   178,   273,   178,     4,     4,   279,   267,   338,     4,
       6,   267,   288,     4,    96,     3,   294,   178,     4,     4,
     300,    87,   308,   178,    87,   311,   313,   113,   316,   178,
     179,   189,   185,   194,     5,    21,   182,   179,   319,   179,
     320,    12,   203,   213,   189,     4,     3,   234,   242,   199,
     179,   326,   320,   341,     8,   178,   280,   280,   282,     3,
      96,    86,   303,    20,   305,   302,    83,    92,   181,    88,
      88,    92,     8,    14,    92,     8,   248,    88,     4,   269,
     178,    95,     3,     4,     3,   309,     3,     5,   178,   179,
     179,   178,   214,     3,   178,   243,   179,    92,   304,    98,
     306,    92,     3,    92,   178,    87,     3,    87,     3,    15,
     230
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 4: /* "T_TOKEN" */
#line 164 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2025 "parser.cxx"
	break;
      case 5: /* "T_QSTRING" */
#line 165 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2030 "parser.cxx"
	break;
      case 6: /* "T_COMMENT" */
#line 166 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2035 "parser.cxx"
	break;
      case 7: /* "T_LINE" */
#line 167 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2040 "parser.cxx"
	break;
      case 8: /* "T_URI" */
#line 168 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2045 "parser.cxx"
	break;
      case 10: /* "T_DISPLAY" */
#line 169 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2050 "parser.cxx"
	break;
      case 11: /* "T_LANG" */
#line 170 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2055 "parser.cxx"
	break;
      case 12: /* "T_WORD" */
#line 171 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2060 "parser.cxx"
	break;
      case 17: /* "T_METHOD" */
#line 172 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2065 "parser.cxx"
	break;
      case 19: /* "T_AUTH_OTHER" */
#line 173 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2070 "parser.cxx"
	break;
      case 20: /* "T_IPV6ADDR" */
#line 174 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2075 "parser.cxx"
	break;
      case 21: /* "T_PARAMVAL" */
#line 175 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2080 "parser.cxx"
	break;
      case 82: /* "T_HDR_UNKNOWN" */
#line 176 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2085 "parser.cxx"
	break;
      case 107: /* "sip_version" */
#line 239 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2090 "parser.cxx"
	break;
      case 177: /* "media_range" */
#line 232 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_media)); delete (yyvaluep->yyt_media); };
#line 2095 "parser.cxx"
	break;
      case 178: /* "parameters" */
#line 235 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_params)); delete (yyvaluep->yyt_params); };
#line 2100 "parser.cxx"
	break;
      case 179: /* "parameter" */
#line 233 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_param)); delete (yyvaluep->yyt_param); };
#line 2105 "parser.cxx"
	break;
      case 182: /* "parameter_val" */
#line 234 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2110 "parser.cxx"
	break;
      case 184: /* "content_coding" */
#line 220 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_coding)); delete (yyvaluep->yyt_coding); };
#line 2115 "parser.cxx"
	break;
      case 189: /* "language" */
#line 231 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_language)); delete (yyvaluep->yyt_language); };
#line 2120 "parser.cxx"
	break;
      case 192: /* "alert_param" */
#line 212 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_alert_param)); delete (yyvaluep->yyt_alert_param); };
#line 2125 "parser.cxx"
	break;
      case 199: /* "call_id" */
#line 214 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2130 "parser.cxx"
	break;
      case 201: /* "info_param" */
#line 230 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_info_param)); delete (yyvaluep->yyt_info_param); };
#line 2135 "parser.cxx"
	break;
      case 207: /* "contacts" */
#line 219 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_contacts)); delete (yyvaluep->yyt_contacts); };
#line 2140 "parser.cxx"
	break;
      case 208: /* "contact_param" */
#line 218 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_contact)); delete (yyvaluep->yyt_contact); };
#line 2145 "parser.cxx"
	break;
      case 209: /* "contact_addr" */
#line 217 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_contact)); delete (yyvaluep->yyt_contact); };
#line 2150 "parser.cxx"
	break;
      case 215: /* "display_name" */
#line 224 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2155 "parser.cxx"
	break;
      case 232: /* "error_param" */
#line 225 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_error_param)); delete (yyvaluep->yyt_error_param); };
#line 2160 "parser.cxx"
	break;
      case 240: /* "from_addr" */
#line 226 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_from_addr)); delete (yyvaluep->yyt_from_addr); };
#line 2165 "parser.cxx"
	break;
      case 267: /* "rec_route" */
#line 236 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_route)); delete (yyvaluep->yyt_route); };
#line 2170 "parser.cxx"
	break;
      case 280: /* "comment" */
#line 216 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2175 "parser.cxx"
	break;
      case 285: /* "server" */
#line 238 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_server)); delete (yyvaluep->yyt_server); };
#line 2180 "parser.cxx"
	break;
      case 300: /* "via_parm" */
#line 240 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_via)); delete (yyvaluep->yyt_via); };
#line 2185 "parser.cxx"
	break;
      case 301: /* "sent_protocol" */
#line 237 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_via)); delete (yyvaluep->yyt_via); };
#line 2190 "parser.cxx"
	break;
      case 302: /* "host" */
#line 228 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_via)); delete (yyvaluep->yyt_via); };
#line 2195 "parser.cxx"
	break;
      case 307: /* "ipv6reference" */
#line 229 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2200 "parser.cxx"
	break;
      case 311: /* "warning" */
#line 241 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_warning)); delete (yyvaluep->yyt_warning); };
#line 2205 "parser.cxx"
	break;
      case 314: /* "hdr_unknown" */
#line 227 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_str)); delete (yyvaluep->yyt_str); };
#line 2210 "parser.cxx"
	break;
      case 319: /* "digest_response" */
#line 223 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_dig_resp)); delete (yyvaluep->yyt_dig_resp); };
#line 2215 "parser.cxx"
	break;
      case 320: /* "auth_params" */
#line 213 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_params)); delete (yyvaluep->yyt_params); };
#line 2220 "parser.cxx"
	break;
      case 321: /* "credentials" */
#line 221 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_credentials)); delete (yyvaluep->yyt_credentials); };
#line 2225 "parser.cxx"
	break;
      case 326: /* "digest_challenge" */
#line 222 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_dig_chlg)); delete (yyvaluep->yyt_dig_chlg); };
#line 2230 "parser.cxx"
	break;
      case 327: /* "challenge" */
#line 215 "parser.yxx"
	{ MEMMAN_DELETE((yyvaluep->yyt_challenge)); delete (yyvaluep->yyt_challenge); };
#line 2235 "parser.cxx"
	break;

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 244 "parser.yxx"
    { CTXT_NEW; }
    break;

  case 6:
#line 249 "parser.yxx"
    {
			/* KLUDGE to work around a memory leak in bison.
			 * T_NULL does never match, so the parser never
			 * gets here. The error keyword causes bison
			 * to eat all input and destroy all tokens returned
			 * by the parser.
			 * Without this workaround the following input causes
			 * the parser to leak:
			 *
			 *   INVITE INVITE ....
			 *
			 * In request_line a T_METHOD is returned as look ahead
			 * token when bison tries to match sip_version.
			 * This does not match, but the look ahead token is
			 * never destructed by Bison.
			 */
			YYABORT;
		}
    break;

  case 7:
#line 269 "parser.yxx"
    {
		  	/* Parsing stops here. Remaining text is
			 * not parsed.
			 */
		  	YYACCEPT; }
    break;

  case 8:
#line 278 "parser.yxx"
    { CTXT_URI; }
    break;

  case 9:
#line 278 "parser.yxx"
    { CTXT_NEW; }
    break;

  case 10:
#line 279 "parser.yxx"
    {
		  	MSG = new t_request();
			MEMMAN_NEW(MSG);
			((t_request *)MSG)->set_method(*(yyvsp[(1) - (6)].yyt_str));
			((t_request *)MSG)->uri.set_url(*(yyvsp[(3) - (6)].yyt_str));
			MSG->version = *(yyvsp[(5) - (6)].yyt_str);
			MEMMAN_DELETE((yyvsp[(1) - (6)].yyt_str)); delete (yyvsp[(1) - (6)].yyt_str);
			MEMMAN_DELETE((yyvsp[(3) - (6)].yyt_str)); delete (yyvsp[(3) - (6)].yyt_str);
			MEMMAN_DELETE((yyvsp[(5) - (6)].yyt_str)); delete (yyvsp[(5) - (6)].yyt_str);

			if (!((t_request *)MSG)->uri.is_valid()) {
				MEMMAN_DELETE(MSG); delete MSG;
				MSG = NULL;
				YYABORT;
			} }
    break;

  case 11:
#line 296 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 12:
#line 296 "parser.yxx"
    {
			(yyval.yyt_str) = (yyvsp[(4) - (4)].yyt_str); }
    break;

  case 13:
#line 300 "parser.yxx"
    {
		  	/* Parsing stops here. Remaining text is
			 * not parsed.
			 */
		  	YYACCEPT; }
    break;

  case 14:
#line 307 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 15:
#line 307 "parser.yxx"
    { CTXT_LINE; }
    break;

  case 16:
#line 308 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 17:
#line 308 "parser.yxx"
    {
			MSG = new t_response();
			MEMMAN_NEW(MSG);
		  	MSG->version = *(yyvsp[(1) - (7)].yyt_str);
			((t_response *)MSG)->code = (yyvsp[(3) - (7)].yyt_ulong);
			((t_response *)MSG)->reason = trim(*(yyvsp[(5) - (7)].yyt_str));
			MEMMAN_DELETE((yyvsp[(1) - (7)].yyt_str)); delete (yyvsp[(1) - (7)].yyt_str);
			MEMMAN_DELETE((yyvsp[(5) - (7)].yyt_str)); delete (yyvsp[(5) - (7)].yyt_str); }
    break;

  case 80:
#line 382 "parser.yxx"
    {
			MSG->add_unknown_header(*(yyvsp[(1) - (4)].yyt_str), trim(*(yyvsp[(3) - (4)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(1) - (4)].yyt_str)); delete (yyvsp[(1) - (4)].yyt_str);
			MEMMAN_DELETE((yyvsp[(3) - (4)].yyt_str)); delete (yyvsp[(3) - (4)].yyt_str); }
    break;

  case 81:
#line 387 "parser.yxx"
    { PARSE_ERROR("Accept"); }
    break;

  case 82:
#line 389 "parser.yxx"
    { PARSE_ERROR("Accept-Encoding"); }
    break;

  case 83:
#line 391 "parser.yxx"
    { PARSE_ERROR("Accept-Language"); }
    break;

  case 84:
#line 393 "parser.yxx"
    { PARSE_ERROR("Alert-Info"); }
    break;

  case 85:
#line 395 "parser.yxx"
    { PARSE_ERROR("Allow"); }
    break;

  case 86:
#line 397 "parser.yxx"
    { PARSE_ERROR("Allow-Events"); }
    break;

  case 87:
#line 399 "parser.yxx"
    { PARSE_ERROR("Authentication-Info"); }
    break;

  case 88:
#line 401 "parser.yxx"
    { PARSE_ERROR("Authorization"); }
    break;

  case 89:
#line 403 "parser.yxx"
    { PARSE_ERROR("Call-ID"); }
    break;

  case 90:
#line 405 "parser.yxx"
    { PARSE_ERROR("Call-Info"); }
    break;

  case 91:
#line 407 "parser.yxx"
    { PARSE_ERROR("Contact"); }
    break;

  case 92:
#line 409 "parser.yxx"
    { PARSE_ERROR("Content-Disposition"); }
    break;

  case 93:
#line 411 "parser.yxx"
    { PARSE_ERROR("Content-Encoding"); }
    break;

  case 94:
#line 413 "parser.yxx"
    { PARSE_ERROR("Content-Language"); }
    break;

  case 95:
#line 415 "parser.yxx"
    { PARSE_ERROR("Content-Length"); }
    break;

  case 96:
#line 417 "parser.yxx"
    { PARSE_ERROR("Content-Type"); }
    break;

  case 97:
#line 419 "parser.yxx"
    { PARSE_ERROR("CSeq"); }
    break;

  case 98:
#line 421 "parser.yxx"
    { PARSE_ERROR("Date"); }
    break;

  case 99:
#line 423 "parser.yxx"
    { PARSE_ERROR("Error-Info"); }
    break;

  case 100:
#line 425 "parser.yxx"
    { PARSE_ERROR("Event"); }
    break;

  case 101:
#line 427 "parser.yxx"
    { PARSE_ERROR("Expires"); }
    break;

  case 102:
#line 429 "parser.yxx"
    { PARSE_ERROR("From"); }
    break;

  case 103:
#line 431 "parser.yxx"
    { PARSE_ERROR("In-Reply-To"); }
    break;

  case 104:
#line 433 "parser.yxx"
    { PARSE_ERROR("Max-Forwards"); }
    break;

  case 105:
#line 435 "parser.yxx"
    { PARSE_ERROR("Min-Expires"); }
    break;

  case 106:
#line 437 "parser.yxx"
    { PARSE_ERROR("MIME-Version"); }
    break;

  case 107:
#line 439 "parser.yxx"
    { PARSE_ERROR("Organization"); }
    break;

  case 108:
#line 441 "parser.yxx"
    { PARSE_ERROR("P-Asserted-Identity"); }
    break;

  case 109:
#line 443 "parser.yxx"
    { PARSE_ERROR("P-Preferred-Identity"); }
    break;

  case 110:
#line 445 "parser.yxx"
    { PARSE_ERROR("Priority"); }
    break;

  case 111:
#line 447 "parser.yxx"
    { PARSE_ERROR("Privacy"); }
    break;

  case 112:
#line 449 "parser.yxx"
    { PARSE_ERROR("Proxy-Authenticate"); }
    break;

  case 113:
#line 451 "parser.yxx"
    { PARSE_ERROR("Proxy-Authorization"); }
    break;

  case 114:
#line 453 "parser.yxx"
    { PARSE_ERROR("Proxy-Require"); }
    break;

  case 115:
#line 455 "parser.yxx"
    { PARSE_ERROR("RAck"); }
    break;

  case 116:
#line 457 "parser.yxx"
    { PARSE_ERROR("Record-Route"); }
    break;

  case 117:
#line 459 "parser.yxx"
    { PARSE_ERROR("Refer-Sub"); }
    break;

  case 118:
#line 461 "parser.yxx"
    { PARSE_ERROR("Refer-To"); }
    break;

  case 119:
#line 463 "parser.yxx"
    { PARSE_ERROR("Referred-By"); }
    break;

  case 120:
#line 465 "parser.yxx"
    { PARSE_ERROR("Replaces"); }
    break;

  case 121:
#line 467 "parser.yxx"
    { PARSE_ERROR("Reply-To"); }
    break;

  case 122:
#line 469 "parser.yxx"
    { PARSE_ERROR("Require"); }
    break;

  case 123:
#line 471 "parser.yxx"
    { PARSE_ERROR("Request-Disposition"); }
    break;

  case 124:
#line 473 "parser.yxx"
    { PARSE_ERROR("Retry-After"); }
    break;

  case 125:
#line 475 "parser.yxx"
    { PARSE_ERROR("Route"); }
    break;

  case 126:
#line 477 "parser.yxx"
    { PARSE_ERROR("RSeq"); }
    break;

  case 127:
#line 479 "parser.yxx"
    { PARSE_ERROR("Server"); }
    break;

  case 128:
#line 481 "parser.yxx"
    { PARSE_ERROR("Service-Route"); }
    break;

  case 129:
#line 483 "parser.yxx"
    { PARSE_ERROR("SIP-ETag"); }
    break;

  case 130:
#line 485 "parser.yxx"
    { PARSE_ERROR("SIP-If-Match"); }
    break;

  case 131:
#line 487 "parser.yxx"
    { PARSE_ERROR("Subject"); }
    break;

  case 132:
#line 489 "parser.yxx"
    { PARSE_ERROR("Subscription-State"); }
    break;

  case 133:
#line 491 "parser.yxx"
    { PARSE_ERROR("Supported"); }
    break;

  case 134:
#line 493 "parser.yxx"
    { PARSE_ERROR("Timestamp"); }
    break;

  case 135:
#line 495 "parser.yxx"
    { PARSE_ERROR("To"); }
    break;

  case 136:
#line 497 "parser.yxx"
    { PARSE_ERROR("Unsupported"); }
    break;

  case 137:
#line 499 "parser.yxx"
    { PARSE_ERROR("User-Agent"); }
    break;

  case 138:
#line 501 "parser.yxx"
    { PARSE_ERROR("Via"); }
    break;

  case 139:
#line 503 "parser.yxx"
    { PARSE_ERROR("Warning"); }
    break;

  case 140:
#line 505 "parser.yxx"
    { PARSE_ERROR("WWW-Authenticate"); }
    break;

  case 143:
#line 521 "parser.yxx"
    { CTXT_LANG; }
    break;

  case 148:
#line 531 "parser.yxx"
    { CTXT_AUTH_SCHEME; }
    break;

  case 149:
#line 533 "parser.yxx"
    { CTXT_WORD; }
    break;

  case 151:
#line 537 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 154:
#line 543 "parser.yxx"
    { CTXT_LANG; }
    break;

  case 155:
#line 545 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 157:
#line 549 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 158:
#line 551 "parser.yxx"
    { CTXT_DATE;}
    break;

  case 161:
#line 557 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 162:
#line 559 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 163:
#line 561 "parser.yxx"
    { CTXT_WORD; }
    break;

  case 164:
#line 563 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 165:
#line 565 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 167:
#line 569 "parser.yxx"
    { CTXT_LINE; }
    break;

  case 168:
#line 571 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 169:
#line 573 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 172:
#line 579 "parser.yxx"
    { CTXT_AUTH_SCHEME; }
    break;

  case 173:
#line 581 "parser.yxx"
    { CTXT_AUTH_SCHEME; }
    break;

  case 175:
#line 585 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 176:
#line 587 "parser.yxx"
    { CTXT_URI; }
    break;

  case 178:
#line 591 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 179:
#line 593 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 180:
#line 595 "parser.yxx"
    { CTXT_WORD; }
    break;

  case 181:
#line 597 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 184:
#line 603 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 185:
#line 605 "parser.yxx"
    { CTXT_URI; }
    break;

  case 186:
#line 607 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 188:
#line 611 "parser.yxx"
    { CTXT_URI; }
    break;

  case 191:
#line 617 "parser.yxx"
    { CTXT_LINE; }
    break;

  case 194:
#line 623 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 195:
#line 625 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 199:
#line 633 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 200:
#line 635 "parser.yxx"
    { CTXT_AUTH_SCHEME; }
    break;

  case 202:
#line 639 "parser.yxx"
    {
			(yyvsp[(1) - (2)].yyt_media)->add_params(*(yyvsp[(2) - (2)].yyt_params));
			MSG->hdr_accept.add_media(*(yyvsp[(1) - (2)].yyt_media));
			MEMMAN_DELETE((yyvsp[(1) - (2)].yyt_media)); delete (yyvsp[(1) - (2)].yyt_media);
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_params)); delete (yyvsp[(2) - (2)].yyt_params); }
    break;

  case 203:
#line 644 "parser.yxx"
    {
			(yyvsp[(3) - (4)].yyt_media)->add_params(*(yyvsp[(4) - (4)].yyt_params));
			MSG->hdr_accept.add_media(*(yyvsp[(3) - (4)].yyt_media));
			MEMMAN_DELETE((yyvsp[(3) - (4)].yyt_media)); delete (yyvsp[(3) - (4)].yyt_media);
			MEMMAN_DELETE((yyvsp[(4) - (4)].yyt_params)); delete (yyvsp[(4) - (4)].yyt_params); }
    break;

  case 204:
#line 651 "parser.yxx"
    { (yyval.yyt_media) = new t_media(tolower(*(yyvsp[(1) - (3)].yyt_str)), tolower(*(yyvsp[(3) - (3)].yyt_str)));
					MEMMAN_NEW((yyval.yyt_media));
					MEMMAN_DELETE((yyvsp[(1) - (3)].yyt_str)); delete (yyvsp[(1) - (3)].yyt_str);
					MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_str)); delete (yyvsp[(3) - (3)].yyt_str); }
    break;

  case 205:
#line 657 "parser.yxx"
    { (yyval.yyt_params) = new list<t_parameter>; MEMMAN_NEW((yyval.yyt_params)); }
    break;

  case 206:
#line 658 "parser.yxx"
    {
			(yyvsp[(1) - (3)].yyt_params)->push_back(*(yyvsp[(3) - (3)].yyt_param));
			(yyval.yyt_params) = (yyvsp[(1) - (3)].yyt_params);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_param)); delete (yyvsp[(3) - (3)].yyt_param); }
    break;

  case 207:
#line 664 "parser.yxx"
    {
			(yyval.yyt_param) = new t_parameter(tolower(*(yyvsp[(1) - (1)].yyt_str)));
			MEMMAN_NEW((yyval.yyt_param));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 208:
#line 668 "parser.yxx"
    { CTXT_PARAMVAL; }
    break;

  case 209:
#line 668 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 210:
#line 668 "parser.yxx"
    {
			(yyval.yyt_param) = new t_parameter(tolower(*(yyvsp[(1) - (5)].yyt_str)), *(yyvsp[(4) - (5)].yyt_str));
			MEMMAN_NEW((yyval.yyt_param));
			MEMMAN_DELETE((yyvsp[(1) - (5)].yyt_str)); delete (yyvsp[(1) - (5)].yyt_str);
			MEMMAN_DELETE((yyvsp[(4) - (5)].yyt_str)); delete (yyvsp[(4) - (5)].yyt_str); }
    break;

  case 211:
#line 675 "parser.yxx"
    {
			(yyval.yyt_str) = (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 212:
#line 677 "parser.yxx"
    {
			(yyval.yyt_str) = (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 213:
#line 681 "parser.yxx"
    {
			MSG->hdr_accept_encoding.add_coding(*(yyvsp[(1) - (1)].yyt_coding));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_coding)); delete (yyvsp[(1) - (1)].yyt_coding); }
    break;

  case 214:
#line 684 "parser.yxx"
    {
			MSG->hdr_accept_encoding.add_coding(*(yyvsp[(3) - (3)].yyt_coding));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_coding)); delete (yyvsp[(3) - (3)].yyt_coding); }
    break;

  case 215:
#line 689 "parser.yxx"
    {
			(yyval.yyt_coding) = new t_coding(tolower(*(yyvsp[(1) - (1)].yyt_str)));
			MEMMAN_NEW((yyval.yyt_coding));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 216:
#line 693 "parser.yxx"
    {
			(yyval.yyt_coding) = new t_coding(tolower(*(yyvsp[(1) - (2)].yyt_str)));
			MEMMAN_NEW((yyval.yyt_coding));
			(yyval.yyt_coding)->q = (yyvsp[(2) - (2)].yyt_float);
			MEMMAN_DELETE((yyvsp[(1) - (2)].yyt_str)); delete (yyvsp[(1) - (2)].yyt_str); }
    break;

  case 217:
#line 700 "parser.yxx"
    {
			if ((yyvsp[(2) - (2)].yyt_param)->name != "q") YYERROR;
			(yyval.yyt_float) = atof((yyvsp[(2) - (2)].yyt_param)->value.c_str());
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_param)); delete (yyvsp[(2) - (2)].yyt_param);
			}
    break;

  case 218:
#line 707 "parser.yxx"
    { CTXT_LANG; }
    break;

  case 219:
#line 707 "parser.yxx"
    {
			MSG->hdr_accept_language.add_language(*(yyvsp[(2) - (2)].yyt_language));
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_language)); delete (yyvsp[(2) - (2)].yyt_language); }
    break;

  case 220:
#line 710 "parser.yxx"
    { CTXT_LANG; }
    break;

  case 221:
#line 710 "parser.yxx"
    {
			MSG->hdr_accept_language.add_language(*(yyvsp[(4) - (4)].yyt_language));
			MEMMAN_DELETE((yyvsp[(4) - (4)].yyt_language)); delete (yyvsp[(4) - (4)].yyt_language); }
    break;

  case 222:
#line 715 "parser.yxx"
    {
			CTXT_INITIAL;
		  	(yyval.yyt_language) = new t_language(tolower(*(yyvsp[(1) - (1)].yyt_str)));
			MEMMAN_NEW((yyval.yyt_language));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 223:
#line 720 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 224:
#line 720 "parser.yxx"
    {
			(yyval.yyt_language) = new t_language(tolower(*(yyvsp[(1) - (3)].yyt_str)));
			MEMMAN_NEW((yyval.yyt_language));
			(yyval.yyt_language)->q = (yyvsp[(3) - (3)].yyt_float);
			MEMMAN_DELETE((yyvsp[(1) - (3)].yyt_str)); delete (yyvsp[(1) - (3)].yyt_str); }
    break;

  case 225:
#line 727 "parser.yxx"
    {
			MSG->hdr_alert_info.add_param(*(yyvsp[(1) - (1)].yyt_alert_param));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_alert_param)); delete (yyvsp[(1) - (1)].yyt_alert_param); }
    break;

  case 226:
#line 730 "parser.yxx"
    {
			MSG->hdr_alert_info.add_param(*(yyvsp[(3) - (3)].yyt_alert_param));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_alert_param)); delete (yyvsp[(3) - (3)].yyt_alert_param); }
    break;

  case 227:
#line 735 "parser.yxx"
    { CTXT_URI; }
    break;

  case 228:
#line 735 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 229:
#line 735 "parser.yxx"
    {
		  	(yyval.yyt_alert_param) = new t_alert_param();
			MEMMAN_NEW((yyval.yyt_alert_param));
			(yyval.yyt_alert_param)->uri.set_url(*(yyvsp[(3) - (6)].yyt_str));
			(yyval.yyt_alert_param)->parameter_list = *(yyvsp[(6) - (6)].yyt_params);

			if (!(yyval.yyt_alert_param)->uri.is_valid()) {
				MEMMAN_DELETE((yyval.yyt_alert_param)); delete (yyval.yyt_alert_param);
				YYERROR;
			}
			 
			MEMMAN_DELETE((yyvsp[(3) - (6)].yyt_str)); delete (yyvsp[(3) - (6)].yyt_str);
			MEMMAN_DELETE((yyvsp[(6) - (6)].yyt_params)); delete (yyvsp[(6) - (6)].yyt_params); }
    break;

  case 230:
#line 750 "parser.yxx"
    {
			MSG->hdr_allow.add_method(*(yyvsp[(1) - (1)].yyt_str));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 231:
#line 753 "parser.yxx"
    {
			MSG->hdr_allow.add_method(*(yyvsp[(3) - (3)].yyt_str));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_str)); delete (yyvsp[(3) - (3)].yyt_str); }
    break;

  case 232:
#line 758 "parser.yxx"
    { CTXT_WORD; }
    break;

  case 233:
#line 758 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 234:
#line 758 "parser.yxx"
    {
			MSG->hdr_call_id.set_call_id(*(yyvsp[(2) - (3)].yyt_str));
			MEMMAN_DELETE((yyvsp[(2) - (3)].yyt_str)); delete (yyvsp[(2) - (3)].yyt_str); }
    break;

  case 235:
#line 763 "parser.yxx"
    { (yyval.yyt_str) = (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 236:
#line 764 "parser.yxx"
    {
			(yyval.yyt_str) = new string(*(yyvsp[(1) - (3)].yyt_str) + '@' + *(yyvsp[(3) - (3)].yyt_str));
			MEMMAN_NEW((yyval.yyt_str));
			MEMMAN_DELETE((yyvsp[(1) - (3)].yyt_str)); delete (yyvsp[(1) - (3)].yyt_str);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_str)); delete (yyvsp[(3) - (3)].yyt_str); }
    break;

  case 237:
#line 771 "parser.yxx"
    {
			MSG->hdr_call_info.add_param(*(yyvsp[(1) - (1)].yyt_info_param));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_info_param)); delete (yyvsp[(1) - (1)].yyt_info_param); }
    break;

  case 238:
#line 774 "parser.yxx"
    {
			MSG->hdr_call_info.add_param(*(yyvsp[(3) - (3)].yyt_info_param));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_info_param)); delete (yyvsp[(3) - (3)].yyt_info_param); }
    break;

  case 239:
#line 779 "parser.yxx"
    { CTXT_URI; }
    break;

  case 240:
#line 779 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 241:
#line 779 "parser.yxx"
    {
		  	(yyval.yyt_info_param) = new t_info_param();
			MEMMAN_NEW((yyval.yyt_info_param));
			(yyval.yyt_info_param)->uri.set_url(*(yyvsp[(3) - (6)].yyt_str));
			(yyval.yyt_info_param)->parameter_list = *(yyvsp[(6) - (6)].yyt_params);

			if (!(yyval.yyt_info_param)->uri.is_valid()) {
				MEMMAN_DELETE((yyval.yyt_info_param)); delete (yyval.yyt_info_param);
				YYERROR;
			}
			
			MEMMAN_DELETE((yyvsp[(3) - (6)].yyt_str)); delete (yyvsp[(3) - (6)].yyt_str);
			MEMMAN_DELETE((yyvsp[(6) - (6)].yyt_params)); delete (yyvsp[(6) - (6)].yyt_params); }
    break;

  case 242:
#line 794 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 243:
#line 794 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 244:
#line 794 "parser.yxx"
    {
			MSG->hdr_contact.set_any(); }
    break;

  case 245:
#line 796 "parser.yxx"
    {
			MSG->hdr_contact.add_contacts(*(yyvsp[(1) - (1)].yyt_contacts));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_contacts)); delete (yyvsp[(1) - (1)].yyt_contacts); }
    break;

  case 246:
#line 801 "parser.yxx"
    {
			(yyval.yyt_contacts) = new list<t_contact_param>;
			MEMMAN_NEW((yyval.yyt_contacts));
			(yyval.yyt_contacts)->push_back(*(yyvsp[(1) - (1)].yyt_contact));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_contact)); delete (yyvsp[(1) - (1)].yyt_contact); }
    break;

  case 247:
#line 806 "parser.yxx"
    {
			(yyvsp[(1) - (3)].yyt_contacts)->push_back(*(yyvsp[(3) - (3)].yyt_contact));
			(yyval.yyt_contacts) = (yyvsp[(1) - (3)].yyt_contacts);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_contact)); delete (yyvsp[(3) - (3)].yyt_contact); }
    break;

  case 248:
#line 812 "parser.yxx"
    {
			(yyval.yyt_contact) = (yyvsp[(1) - (2)].yyt_contact);
			list<t_parameter>::const_iterator i;
			for (i = (yyvsp[(2) - (2)].yyt_params)->begin(); i != (yyvsp[(2) - (2)].yyt_params)->end(); i++) {
				if (i->name == "q") {
					(yyval.yyt_contact)->set_qvalue(atof(i->value.c_str()));
				} else if (i->name == "expires") {
					(yyval.yyt_contact)->set_expires(strtoul(
						i->value.c_str(), NULL, 10));
				} else {
					(yyval.yyt_contact)->add_extension(*i);
				}
			}
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_params)); delete (yyvsp[(2) - (2)].yyt_params); }
    break;

  case 249:
#line 828 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 250:
#line 828 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 251:
#line 828 "parser.yxx"
    {
			(yyval.yyt_contact) = new t_contact_param();
			MEMMAN_NEW((yyval.yyt_contact));
			(yyval.yyt_contact)->uri.set_url(*(yyvsp[(2) - (3)].yyt_str));

			if (!(yyval.yyt_contact)->uri.is_valid()) {
				MEMMAN_DELETE((yyval.yyt_contact)); delete (yyval.yyt_contact);
				YYERROR;
			}
			
			MEMMAN_DELETE((yyvsp[(2) - (3)].yyt_str)); delete (yyvsp[(2) - (3)].yyt_str); }
    break;

  case 252:
#line 839 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 253:
#line 839 "parser.yxx"
    { CTXT_URI; }
    break;

  case 254:
#line 839 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 255:
#line 839 "parser.yxx"
    {
			(yyval.yyt_contact) = new t_contact_param();
			MEMMAN_NEW((yyval.yyt_contact));
			(yyval.yyt_contact)->display = *(yyvsp[(2) - (7)].yyt_str);
			(yyval.yyt_contact)->uri.set_url(*(yyvsp[(5) - (7)].yyt_str));

			if (!(yyval.yyt_contact)->uri.is_valid()) {
				MEMMAN_DELETE((yyval.yyt_contact)); delete (yyval.yyt_contact);
				YYERROR;
			}
			 
			MEMMAN_DELETE((yyvsp[(2) - (7)].yyt_str)); delete (yyvsp[(2) - (7)].yyt_str);
			MEMMAN_DELETE((yyvsp[(5) - (7)].yyt_str)); delete (yyvsp[(5) - (7)].yyt_str); }
    break;

  case 256:
#line 854 "parser.yxx"
    { (yyval.yyt_str) = new string(); MEMMAN_NEW((yyval.yyt_str)); }
    break;

  case 257:
#line 855 "parser.yxx"
    {
			(yyval.yyt_str) = new string(rtrim(*(yyvsp[(1) - (1)].yyt_str)));
			MEMMAN_NEW((yyval.yyt_str));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 258:
#line 859 "parser.yxx"
    { (yyval.yyt_str) = (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 259:
#line 862 "parser.yxx"
    {
			MSG->hdr_content_disp.set_type(tolower(*(yyvsp[(1) - (2)].yyt_str)));
			
			list<t_parameter>::const_iterator i;
			for (i = (yyvsp[(2) - (2)].yyt_params)->begin(); i != (yyvsp[(2) - (2)].yyt_params)->end(); i++) {
				if (i->name == "filename") {
					MSG->hdr_content_disp.set_filename(i->value);
				} else {
					MSG->hdr_content_disp.add_param(*i);
				}
			}

			MEMMAN_DELETE((yyvsp[(1) - (2)].yyt_str)); delete (yyvsp[(1) - (2)].yyt_str);
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_params)); delete (yyvsp[(2) - (2)].yyt_params); }
    break;

  case 260:
#line 878 "parser.yxx"
    {
			MSG->hdr_content_encoding.add_coding(*(yyvsp[(1) - (1)].yyt_coding));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_coding)); delete (yyvsp[(1) - (1)].yyt_coding); }
    break;

  case 261:
#line 881 "parser.yxx"
    {
			MSG->hdr_content_encoding.add_coding(*(yyvsp[(3) - (3)].yyt_coding));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_coding)); delete (yyvsp[(3) - (3)].yyt_coding); }
    break;

  case 262:
#line 886 "parser.yxx"
    { CTXT_LANG; }
    break;

  case 263:
#line 886 "parser.yxx"
    {
			MSG->hdr_content_language.add_language(*(yyvsp[(2) - (2)].yyt_language));
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_language)); delete (yyvsp[(2) - (2)].yyt_language); }
    break;

  case 264:
#line 889 "parser.yxx"
    { CTXT_LANG; }
    break;

  case 265:
#line 889 "parser.yxx"
    {
			MSG->hdr_content_language.add_language(*(yyvsp[(4) - (4)].yyt_language));
			MEMMAN_DELETE((yyvsp[(4) - (4)].yyt_language)); delete (yyvsp[(4) - (4)].yyt_language); }
    break;

  case 266:
#line 894 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 267:
#line 894 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 268:
#line 894 "parser.yxx"
    {
			MSG->hdr_content_length.set_length((yyvsp[(2) - (3)].yyt_ulong)); }
    break;

  case 269:
#line 898 "parser.yxx"
    {
			(yyvsp[(1) - (2)].yyt_media)->add_params(*(yyvsp[(2) - (2)].yyt_params));
			MSG->hdr_content_type.set_media(*(yyvsp[(1) - (2)].yyt_media));
			MEMMAN_DELETE((yyvsp[(1) - (2)].yyt_media)); delete (yyvsp[(1) - (2)].yyt_media);
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_params)); delete (yyvsp[(2) - (2)].yyt_params); }
    break;

  case 270:
#line 905 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 271:
#line 905 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 272:
#line 905 "parser.yxx"
    {
			MSG->hdr_cseq.set_seqnr((yyvsp[(2) - (4)].yyt_ulong));
			MSG->hdr_cseq.set_method(*(yyvsp[(4) - (4)].yyt_str));
			MEMMAN_DELETE((yyvsp[(4) - (4)].yyt_str)); delete (yyvsp[(4) - (4)].yyt_str); }
    break;

  case 273:
#line 911 "parser.yxx"
    { CTXT_DATE;}
    break;

  case 274:
#line 914 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 275:
#line 914 "parser.yxx"
    {
			struct tm t;
			t.tm_mday = (yyvsp[(4) - (13)].yyt_ulong);
			t.tm_mon = (yyvsp[(5) - (13)].yyt_int);
			t.tm_year = (yyvsp[(6) - (13)].yyt_ulong) - 1900;
			t.tm_hour = (yyvsp[(7) - (13)].yyt_ulong);
			t.tm_min = (yyvsp[(9) - (13)].yyt_ulong);
			t.tm_sec = (yyvsp[(11) - (13)].yyt_ulong);
			MSG->hdr_date.set_date_gm(&t); }
    break;

  case 276:
#line 925 "parser.yxx"
    {
			MSG->hdr_error_info.add_param(*(yyvsp[(1) - (1)].yyt_error_param));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_error_param)); delete (yyvsp[(1) - (1)].yyt_error_param); }
    break;

  case 277:
#line 928 "parser.yxx"
    {
			MSG->hdr_error_info.add_param(*(yyvsp[(3) - (3)].yyt_error_param));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_error_param)); delete (yyvsp[(3) - (3)].yyt_error_param); }
    break;

  case 278:
#line 933 "parser.yxx"
    { CTXT_URI; }
    break;

  case 279:
#line 933 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 280:
#line 933 "parser.yxx"
    {
		  	(yyval.yyt_error_param) = new t_error_param();
			MEMMAN_NEW((yyval.yyt_error_param));
			(yyval.yyt_error_param)->uri.set_url(*(yyvsp[(3) - (6)].yyt_str));
			(yyval.yyt_error_param)->parameter_list = *(yyvsp[(6) - (6)].yyt_params);

			if (!(yyval.yyt_error_param)->uri.is_valid()) {
				MEMMAN_DELETE((yyval.yyt_error_param)); delete (yyval.yyt_error_param);
				YYERROR;
			}
			
			MEMMAN_DELETE((yyvsp[(3) - (6)].yyt_str)); delete (yyvsp[(3) - (6)].yyt_str);
			MEMMAN_DELETE((yyvsp[(6) - (6)].yyt_params)); delete (yyvsp[(6) - (6)].yyt_params); }
    break;

  case 281:
#line 948 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 282:
#line 948 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 283:
#line 948 "parser.yxx"
    {
			MSG->hdr_expires.set_time((yyvsp[(2) - (3)].yyt_ulong)); }
    break;

  case 284:
#line 952 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 285:
#line 952 "parser.yxx"
    {
			MSG->hdr_from.set_display((yyvsp[(2) - (3)].yyt_from_addr)->display);
			MSG->hdr_from.set_uri((yyvsp[(2) - (3)].yyt_from_addr)->uri);
			list<t_parameter>::const_iterator i;
			for (i = (yyvsp[(3) - (3)].yyt_params)->begin(); i != (yyvsp[(3) - (3)].yyt_params)->end(); i++) {
				if (i->name == "tag") {
					MSG->hdr_from.set_tag(i->value);
				} else {
					MSG->hdr_from.add_param(*i);
				}
			}
			MEMMAN_DELETE((yyvsp[(2) - (3)].yyt_from_addr)); delete (yyvsp[(2) - (3)].yyt_from_addr);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_params)); delete (yyvsp[(3) - (3)].yyt_params); }
    break;

  case 286:
#line 967 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 287:
#line 967 "parser.yxx"
    {
			(yyval.yyt_from_addr) = new t_identity();
			MEMMAN_NEW((yyval.yyt_from_addr));
			(yyval.yyt_from_addr)->set_uri(*(yyvsp[(1) - (2)].yyt_str));

			if (!(yyval.yyt_from_addr)->uri.is_valid()) {
				MEMMAN_DELETE((yyval.yyt_from_addr)); delete (yyval.yyt_from_addr);
				YYERROR;
			}
			
			MEMMAN_DELETE((yyvsp[(1) - (2)].yyt_str)); delete (yyvsp[(1) - (2)].yyt_str); }
    break;

  case 288:
#line 978 "parser.yxx"
    { CTXT_URI; }
    break;

  case 289:
#line 978 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 290:
#line 978 "parser.yxx"
    {
			(yyval.yyt_from_addr) = new t_identity();
			MEMMAN_NEW((yyval.yyt_from_addr));
			(yyval.yyt_from_addr)->set_display(*(yyvsp[(1) - (6)].yyt_str));
			(yyval.yyt_from_addr)->set_uri(*(yyvsp[(4) - (6)].yyt_str));

			if (!(yyval.yyt_from_addr)->uri.is_valid()) {
				MEMMAN_DELETE((yyval.yyt_from_addr)); delete (yyval.yyt_from_addr);
				YYERROR;
			}
			
			MEMMAN_DELETE((yyvsp[(1) - (6)].yyt_str)); delete (yyvsp[(1) - (6)].yyt_str);
			MEMMAN_DELETE((yyvsp[(4) - (6)].yyt_str)); delete (yyvsp[(4) - (6)].yyt_str); }
    break;

  case 291:
#line 993 "parser.yxx"
    { CTXT_WORD; }
    break;

  case 292:
#line 993 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 293:
#line 993 "parser.yxx"
    {
			MSG->hdr_in_reply_to.add_call_id(*(yyvsp[(2) - (3)].yyt_str));
			MEMMAN_DELETE((yyvsp[(2) - (3)].yyt_str)); delete (yyvsp[(2) - (3)].yyt_str); }
    break;

  case 294:
#line 996 "parser.yxx"
    { CTXT_WORD; }
    break;

  case 295:
#line 996 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 296:
#line 996 "parser.yxx"
    {
			MSG->hdr_in_reply_to.add_call_id(*(yyvsp[(4) - (5)].yyt_str));
			MEMMAN_DELETE((yyvsp[(4) - (5)].yyt_str)); delete (yyvsp[(4) - (5)].yyt_str); }
    break;

  case 297:
#line 1001 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 298:
#line 1001 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 299:
#line 1001 "parser.yxx"
    {
			MSG->hdr_max_forwards.set_max_forwards((yyvsp[(2) - (3)].yyt_ulong)); }
    break;

  case 300:
#line 1005 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 301:
#line 1005 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 302:
#line 1005 "parser.yxx"
    {
			MSG->hdr_min_expires.set_time((yyvsp[(2) - (3)].yyt_ulong)); }
    break;

  case 303:
#line 1009 "parser.yxx"
    {
			MSG->hdr_mime_version.set_version(*(yyvsp[(1) - (1)].yyt_str));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 304:
#line 1014 "parser.yxx"
    { CTXT_LINE; }
    break;

  case 305:
#line 1014 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 306:
#line 1014 "parser.yxx"
    {
			MSG->hdr_organization.set_name(trim(*(yyvsp[(2) - (3)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(2) - (3)].yyt_str)); delete (yyvsp[(2) - (3)].yyt_str); }
    break;

  case 307:
#line 1019 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 308:
#line 1019 "parser.yxx"
    {
				MSG->hdr_p_asserted_identity.add_identity(*(yyvsp[(2) - (2)].yyt_from_addr));
				MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_from_addr)); delete (yyvsp[(2) - (2)].yyt_from_addr); }
    break;

  case 309:
#line 1022 "parser.yxx"
    {
				MSG->hdr_p_asserted_identity.add_identity(*(yyvsp[(3) - (3)].yyt_from_addr));
				MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_from_addr)); delete (yyvsp[(3) - (3)].yyt_from_addr); }
    break;

  case 310:
#line 1027 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 311:
#line 1027 "parser.yxx"
    {
				MSG->hdr_p_preferred_identity.add_identity(*(yyvsp[(2) - (2)].yyt_from_addr));
				MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_from_addr)); delete (yyvsp[(2) - (2)].yyt_from_addr); }
    break;

  case 312:
#line 1030 "parser.yxx"
    {
				MSG->hdr_p_preferred_identity.add_identity(*(yyvsp[(3) - (3)].yyt_from_addr));
				MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_from_addr)); delete (yyvsp[(3) - (3)].yyt_from_addr); }
    break;

  case 313:
#line 1035 "parser.yxx"
    {
			MSG->hdr_priority.set_priority(tolower(*(yyvsp[(1) - (1)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 314:
#line 1040 "parser.yxx"
    {
			MSG->hdr_privacy.add_privacy(tolower(*(yyvsp[(1) - (1)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 315:
#line 1043 "parser.yxx"
    {
			MSG->hdr_privacy.add_privacy(tolower(*(yyvsp[(3) - (3)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_str)); delete (yyvsp[(3) - (3)].yyt_str); }
    break;

  case 316:
#line 1048 "parser.yxx"
    {
			MSG->hdr_proxy_require.add_feature(tolower(*(yyvsp[(1) - (1)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 317:
#line 1051 "parser.yxx"
    {
			MSG->hdr_proxy_require.add_feature(tolower(*(yyvsp[(3) - (3)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_str)); delete (yyvsp[(3) - (3)].yyt_str); }
    break;

  case 318:
#line 1056 "parser.yxx"
    {
			MSG->hdr_record_route.add_route(*(yyvsp[(1) - (1)].yyt_route));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_route)); delete (yyvsp[(1) - (1)].yyt_route); }
    break;

  case 319:
#line 1059 "parser.yxx"
    {
		  	MSG->hdr_record_route.add_route(*(yyvsp[(3) - (3)].yyt_route));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_route)); delete (yyvsp[(3) - (3)].yyt_route); }
    break;

  case 320:
#line 1064 "parser.yxx"
    { CTXT_URI; }
    break;

  case 321:
#line 1064 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 322:
#line 1065 "parser.yxx"
    {
			(yyval.yyt_route) = new t_route;
			MEMMAN_NEW((yyval.yyt_route));
			(yyval.yyt_route)->display = *(yyvsp[(2) - (7)].yyt_str);
			(yyval.yyt_route)->uri.set_url(*(yyvsp[(4) - (7)].yyt_str));
			(yyval.yyt_route)->set_params(*(yyvsp[(7) - (7)].yyt_params));

			if (!(yyval.yyt_route)->uri.is_valid()) {
				MEMMAN_DELETE((yyval.yyt_route)); delete (yyval.yyt_route);
				YYERROR;
			}
			 
			MEMMAN_DELETE((yyvsp[(2) - (7)].yyt_str)); delete (yyvsp[(2) - (7)].yyt_str);
			MEMMAN_DELETE((yyvsp[(4) - (7)].yyt_str)); delete (yyvsp[(4) - (7)].yyt_str);
			MEMMAN_DELETE((yyvsp[(7) - (7)].yyt_params)); delete (yyvsp[(7) - (7)].yyt_params); }
    break;

  case 323:
#line 1082 "parser.yxx"
    {
			MSG->hdr_service_route.add_route(*(yyvsp[(1) - (1)].yyt_route));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_route)); delete (yyvsp[(1) - (1)].yyt_route); }
    break;

  case 324:
#line 1085 "parser.yxx"
    {
		  	MSG->hdr_service_route.add_route(*(yyvsp[(3) - (3)].yyt_route));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_route)); delete (yyvsp[(3) - (3)].yyt_route); }
    break;

  case 325:
#line 1090 "parser.yxx"
    { CTXT_WORD; }
    break;

  case 326:
#line 1090 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 327:
#line 1090 "parser.yxx"
    {
			MSG->hdr_replaces.set_call_id(*(yyvsp[(2) - (4)].yyt_str));
			
			list<t_parameter>::const_iterator i;
			for (i = (yyvsp[(4) - (4)].yyt_params)->begin(); i != (yyvsp[(4) - (4)].yyt_params)->end(); i++) {
				if (i->name == "to-tag") {
					MSG->hdr_replaces.set_to_tag(i->value);
				} else if (i->name == "from-tag") {
					MSG->hdr_replaces.set_from_tag(i->value);
				} else if (i->name == "early-only") {
					MSG->hdr_replaces.set_early_only(true);
				} else {
					MSG->hdr_replaces.add_param(*i);
				}
			}
			
			if (!MSG->hdr_replaces.is_valid()) YYERROR;
			
			MEMMAN_DELETE((yyvsp[(2) - (4)].yyt_str)); delete (yyvsp[(2) - (4)].yyt_str);
			MEMMAN_DELETE((yyvsp[(4) - (4)].yyt_params)); delete (yyvsp[(4) - (4)].yyt_params); }
    break;

  case 328:
#line 1112 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 329:
#line 1112 "parser.yxx"
    {
			MSG->hdr_reply_to.set_display((yyvsp[(2) - (3)].yyt_from_addr)->display);
			MSG->hdr_reply_to.set_uri((yyvsp[(2) - (3)].yyt_from_addr)->uri);
			MSG->hdr_reply_to.set_params(*(yyvsp[(3) - (3)].yyt_params));
			MEMMAN_DELETE((yyvsp[(2) - (3)].yyt_from_addr)); delete (yyvsp[(2) - (3)].yyt_from_addr);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_params)); delete (yyvsp[(3) - (3)].yyt_params); }
    break;

  case 330:
#line 1120 "parser.yxx"
    {
			MSG->hdr_require.add_feature(tolower(*(yyvsp[(1) - (1)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 331:
#line 1123 "parser.yxx"
    {
			MSG->hdr_require.add_feature(tolower(*(yyvsp[(3) - (3)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_str)); delete (yyvsp[(3) - (3)].yyt_str); }
    break;

  case 332:
#line 1128 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 333:
#line 1128 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 334:
#line 1128 "parser.yxx"
    {
			MSG->hdr_retry_after.set_time((yyvsp[(2) - (5)].yyt_ulong));
			MSG->hdr_retry_after.set_comment(*(yyvsp[(4) - (5)].yyt_str));
			list<t_parameter>::const_iterator i;
			for (i = (yyvsp[(5) - (5)].yyt_params)->begin(); i != (yyvsp[(5) - (5)].yyt_params)->end(); i++) {
				if (i->name == "duration") {
					int d = strtoul(i->value.c_str(), NULL, 10);
					MSG->hdr_retry_after.set_duration(d);
				} else {
					MSG->hdr_retry_after.add_param(*i);
				}
			}
			MEMMAN_DELETE((yyvsp[(4) - (5)].yyt_str)); delete (yyvsp[(4) - (5)].yyt_str);
			MEMMAN_DELETE((yyvsp[(5) - (5)].yyt_params)); delete (yyvsp[(5) - (5)].yyt_params); }
    break;

  case 335:
#line 1144 "parser.yxx"
    { (yyval.yyt_str) = new string(); MEMMAN_NEW((yyval.yyt_str)); }
    break;

  case 336:
#line 1145 "parser.yxx"
    { CTXT_COMMENT; }
    break;

  case 337:
#line 1145 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 338:
#line 1145 "parser.yxx"
    {
			(yyval.yyt_str) = (yyvsp[(3) - (5)].yyt_str); }
    break;

  case 339:
#line 1149 "parser.yxx"
    {
			MSG->hdr_route.add_route(*(yyvsp[(1) - (1)].yyt_route));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_route)); delete (yyvsp[(1) - (1)].yyt_route); }
    break;

  case 340:
#line 1152 "parser.yxx"
    {
		  	MSG->hdr_route.add_route(*(yyvsp[(3) - (3)].yyt_route));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_route)); delete (yyvsp[(3) - (3)].yyt_route); }
    break;

  case 341:
#line 1157 "parser.yxx"
    {
			MSG->hdr_server.add_server(*(yyvsp[(1) - (1)].yyt_server));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_server)); delete (yyvsp[(1) - (1)].yyt_server); }
    break;

  case 342:
#line 1160 "parser.yxx"
    {
			MSG->hdr_server.add_server(*(yyvsp[(2) - (2)].yyt_server));
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_server)); delete (yyvsp[(2) - (2)].yyt_server); }
    break;

  case 343:
#line 1165 "parser.yxx"
    {
			(yyval.yyt_server) = new t_server();
			MEMMAN_NEW((yyval.yyt_server));
			(yyval.yyt_server)->comment = *(yyvsp[(1) - (1)].yyt_str);
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 344:
#line 1170 "parser.yxx"
    {
			(yyval.yyt_server) = new t_server();
			MEMMAN_NEW((yyval.yyt_server));
			(yyval.yyt_server)->product = *(yyvsp[(1) - (2)].yyt_str);
			(yyval.yyt_server)->comment = *(yyvsp[(2) - (2)].yyt_str);
			MEMMAN_DELETE((yyvsp[(1) - (2)].yyt_str)); delete (yyvsp[(1) - (2)].yyt_str);
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_str)); delete (yyvsp[(2) - (2)].yyt_str); }
    break;

  case 345:
#line 1177 "parser.yxx"
    {
			(yyval.yyt_server) = new t_server();
			MEMMAN_NEW((yyval.yyt_server));
			(yyval.yyt_server)->product = *(yyvsp[(1) - (4)].yyt_str);
			(yyval.yyt_server)->version = *(yyvsp[(3) - (4)].yyt_str);
			(yyval.yyt_server)->comment = *(yyvsp[(4) - (4)].yyt_str);
			MEMMAN_DELETE((yyvsp[(1) - (4)].yyt_str)); delete (yyvsp[(1) - (4)].yyt_str);
			MEMMAN_DELETE((yyvsp[(3) - (4)].yyt_str)); delete (yyvsp[(3) - (4)].yyt_str);
			MEMMAN_DELETE((yyvsp[(4) - (4)].yyt_str)); delete (yyvsp[(4) - (4)].yyt_str); }
    break;

  case 346:
#line 1188 "parser.yxx"
    { CTXT_LINE; }
    break;

  case 347:
#line 1188 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 348:
#line 1188 "parser.yxx"
    {
			MSG->hdr_subject.set_subject(trim(*(yyvsp[(2) - (3)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(2) - (3)].yyt_str)); delete (yyvsp[(2) - (3)].yyt_str); }
    break;

  case 349:
#line 1193 "parser.yxx"
    {
			MSG->hdr_supported.set_empty(); }
    break;

  case 350:
#line 1195 "parser.yxx"
    {
			MSG->hdr_supported.add_feature(tolower(*(yyvsp[(1) - (1)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 351:
#line 1198 "parser.yxx"
    {
			MSG->hdr_supported.add_feature(tolower(*(yyvsp[(3) - (3)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_str)); delete (yyvsp[(3) - (3)].yyt_str); }
    break;

  case 352:
#line 1203 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 353:
#line 1203 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 354:
#line 1206 "parser.yxx"
    {
			MSG->hdr_timestamp.set_timestamp((yyvsp[(1) - (1)].yyt_float)); }
    break;

  case 355:
#line 1208 "parser.yxx"
    {
			MSG->hdr_timestamp.set_timestamp((yyvsp[(1) - (2)].yyt_float));
			MSG->hdr_timestamp.set_delay((yyvsp[(2) - (2)].yyt_float)); }
    break;

  case 356:
#line 1213 "parser.yxx"
    { (yyval.yyt_float) = (yyvsp[(1) - (1)].yyt_ulong); }
    break;

  case 357:
#line 1214 "parser.yxx"
    {
			string s = int2str((yyvsp[(1) - (3)].yyt_ulong)) + '.' + int2str((yyvsp[(3) - (3)].yyt_ulong));
			(yyval.yyt_float) = atof(s.c_str()); }
    break;

  case 358:
#line 1219 "parser.yxx"
    { (yyval.yyt_float) = (yyvsp[(1) - (1)].yyt_ulong); }
    break;

  case 359:
#line 1220 "parser.yxx"
    {
			string s = int2str((yyvsp[(1) - (3)].yyt_ulong)) + '.' + int2str((yyvsp[(3) - (3)].yyt_ulong));
			(yyval.yyt_float) = atof(s.c_str()); }
    break;

  case 360:
#line 1225 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 361:
#line 1225 "parser.yxx"
    {
			MSG->hdr_to.set_display((yyvsp[(2) - (3)].yyt_from_addr)->display);
			MSG->hdr_to.set_uri((yyvsp[(2) - (3)].yyt_from_addr)->uri);
			list<t_parameter>::const_iterator i;
			for (i = (yyvsp[(3) - (3)].yyt_params)->begin(); i != (yyvsp[(3) - (3)].yyt_params)->end(); i++) {
				if (i->name == "tag") {
					MSG->hdr_to.set_tag(i->value);
				} else {
					MSG->hdr_to.add_param(*i);
				}
			}
			MEMMAN_DELETE((yyvsp[(2) - (3)].yyt_from_addr)); delete (yyvsp[(2) - (3)].yyt_from_addr);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_params)); delete (yyvsp[(3) - (3)].yyt_params); }
    break;

  case 362:
#line 1240 "parser.yxx"
    {
			MSG->hdr_unsupported.add_feature(tolower(*(yyvsp[(1) - (1)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 363:
#line 1243 "parser.yxx"
    {
			MSG->hdr_unsupported.add_feature(tolower(*(yyvsp[(3) - (3)].yyt_str)));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_str)); delete (yyvsp[(3) - (3)].yyt_str); }
    break;

  case 364:
#line 1248 "parser.yxx"
    {
			MSG->hdr_user_agent.add_server(*(yyvsp[(1) - (1)].yyt_server));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_server)); delete (yyvsp[(1) - (1)].yyt_server); }
    break;

  case 365:
#line 1251 "parser.yxx"
    {
			MSG->hdr_user_agent.add_server(*(yyvsp[(2) - (2)].yyt_server));
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_server)); delete (yyvsp[(2) - (2)].yyt_server); }
    break;

  case 366:
#line 1256 "parser.yxx"
    {
			MSG->hdr_via.add_via(*(yyvsp[(1) - (1)].yyt_via));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_via)); delete (yyvsp[(1) - (1)].yyt_via); }
    break;

  case 367:
#line 1259 "parser.yxx"
    {
			MSG->hdr_via.add_via(*(yyvsp[(3) - (3)].yyt_via));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_via)); delete (yyvsp[(3) - (3)].yyt_via); }
    break;

  case 368:
#line 1264 "parser.yxx"
    {
			(yyval.yyt_via) = (yyvsp[(1) - (3)].yyt_via);
			(yyval.yyt_via)->host = (yyvsp[(2) - (3)].yyt_via)->host;
			(yyval.yyt_via)->port = (yyvsp[(2) - (3)].yyt_via)->port;
			list<t_parameter>::const_iterator i;
			for (i = (yyvsp[(3) - (3)].yyt_params)->begin(); i != (yyvsp[(3) - (3)].yyt_params)->end(); i++) {
				if (i->name == "ttl") {
					(yyval.yyt_via)->ttl = atoi(i->value.c_str());
				} else if (i->name == "maddr") {
					(yyval.yyt_via)->maddr = i->value;
				} else if (i->name == "received") {
					(yyval.yyt_via)->received = i->value;
				} else if (i->name == "branch") {
					(yyval.yyt_via)->branch = i->value;
				} else if (i->name == "rport") {
					(yyval.yyt_via)->rport_present = true;
					if (i->type == t_parameter::VALUE) {
						(yyval.yyt_via)->rport =
							atoi(i->value.c_str());
					}
				} else {
					(yyval.yyt_via)->add_extension(*i);
				}
			}
			MEMMAN_DELETE((yyvsp[(2) - (3)].yyt_via)); delete (yyvsp[(2) - (3)].yyt_via);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_params)); delete (yyvsp[(3) - (3)].yyt_params); }
    break;

  case 369:
#line 1292 "parser.yxx"
    {
			(yyval.yyt_via) = new t_via();
			MEMMAN_NEW((yyval.yyt_via));
			(yyval.yyt_via)->protocol_name = toupper(*(yyvsp[(1) - (5)].yyt_str));
			(yyval.yyt_via)->protocol_version = *(yyvsp[(3) - (5)].yyt_str);
			(yyval.yyt_via)->transport = toupper(*(yyvsp[(5) - (5)].yyt_str));
			MEMMAN_DELETE((yyvsp[(1) - (5)].yyt_str)); delete (yyvsp[(1) - (5)].yyt_str);
			MEMMAN_DELETE((yyvsp[(3) - (5)].yyt_str)); delete (yyvsp[(3) - (5)].yyt_str);
			MEMMAN_DELETE((yyvsp[(5) - (5)].yyt_str)); delete (yyvsp[(5) - (5)].yyt_str); }
    break;

  case 370:
#line 1303 "parser.yxx"
    {
			(yyval.yyt_via) = new t_via();
			MEMMAN_NEW((yyval.yyt_via));
			(yyval.yyt_via)->host = *(yyvsp[(1) - (1)].yyt_str);
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 371:
#line 1308 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 372:
#line 1308 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 373:
#line 1308 "parser.yxx"
    {
			if ((yyvsp[(4) - (5)].yyt_ulong) > 65535) YYERROR;
			
			(yyval.yyt_via) = new t_via();
			MEMMAN_NEW((yyval.yyt_via));
			(yyval.yyt_via)->host = *(yyvsp[(1) - (5)].yyt_str);
			(yyval.yyt_via)->port = (yyvsp[(4) - (5)].yyt_ulong);
			MEMMAN_DELETE((yyvsp[(1) - (5)].yyt_str)); delete (yyvsp[(1) - (5)].yyt_str); }
    break;

  case 374:
#line 1316 "parser.yxx"
    {
			(yyval.yyt_via) = new t_via();
			MEMMAN_NEW((yyval.yyt_via));
			(yyval.yyt_via)->host = *(yyvsp[(1) - (1)].yyt_str);
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 375:
#line 1321 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 376:
#line 1321 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 377:
#line 1321 "parser.yxx"
    {
			(yyval.yyt_via) = new t_via();
			MEMMAN_NEW((yyval.yyt_via));
			(yyval.yyt_via)->host = *(yyvsp[(1) - (5)].yyt_str);
			(yyval.yyt_via)->port = (yyvsp[(4) - (5)].yyt_ulong);
			MEMMAN_DELETE((yyvsp[(1) - (5)].yyt_str)); delete (yyvsp[(1) - (5)].yyt_str); }
    break;

  case 378:
#line 1329 "parser.yxx"
    { CTXT_IPV6ADDR; }
    break;

  case 379:
#line 1329 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 380:
#line 1329 "parser.yxx"
    {
			// TODO: check correct format of IPv6 address
			(yyval.yyt_str) = new string('[' + *(yyvsp[(3) - (5)].yyt_str) + ']');
			MEMMAN_NEW((yyval.yyt_str));
			MEMMAN_DELETE((yyvsp[(3) - (5)].yyt_str)); }
    break;

  case 381:
#line 1336 "parser.yxx"
    {
			MSG->hdr_warning.add_warning(*(yyvsp[(1) - (1)].yyt_warning));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_warning)); delete (yyvsp[(1) - (1)].yyt_warning); }
    break;

  case 382:
#line 1339 "parser.yxx"
    {
			MSG->hdr_warning.add_warning(*(yyvsp[(3) - (3)].yyt_warning));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_warning)); delete (yyvsp[(3) - (3)].yyt_warning); }
    break;

  case 383:
#line 1344 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 384:
#line 1344 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 385:
#line 1344 "parser.yxx"
    {
			(yyval.yyt_warning) = new t_warning();
			MEMMAN_NEW((yyval.yyt_warning));
			(yyval.yyt_warning)->code = (yyvsp[(2) - (5)].yyt_ulong);
			(yyval.yyt_warning)->host = (yyvsp[(4) - (5)].yyt_via)->host;
			(yyval.yyt_warning)->port = (yyvsp[(4) - (5)].yyt_via)->port;
			(yyval.yyt_warning)->text = *(yyvsp[(5) - (5)].yyt_str);
			MEMMAN_DELETE((yyvsp[(4) - (5)].yyt_via)); delete (yyvsp[(4) - (5)].yyt_via);
			MEMMAN_DELETE((yyvsp[(5) - (5)].yyt_str)); delete (yyvsp[(5) - (5)].yyt_str); }
    break;

  case 386:
#line 1355 "parser.yxx"
    { CTXT_LINE; }
    break;

  case 387:
#line 1355 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 388:
#line 1355 "parser.yxx"
    { (yyval.yyt_str) = (yyvsp[(2) - (3)].yyt_str); }
    break;

  case 389:
#line 1358 "parser.yxx"
    {
			if ((yyvsp[(1) - (1)].yyt_param)->name == "nextnonce")
				MSG->hdr_auth_info.set_next_nonce((yyvsp[(1) - (1)].yyt_param)->value);
		 	else if ((yyvsp[(1) - (1)].yyt_param)->name == "qop")
				MSG->hdr_auth_info.set_message_qop((yyvsp[(1) - (1)].yyt_param)->value);
			else if ((yyvsp[(1) - (1)].yyt_param)->name == "rspauth")
				MSG->hdr_auth_info.set_response_auth((yyvsp[(1) - (1)].yyt_param)->value);
			else if ((yyvsp[(1) - (1)].yyt_param)->name == "cnonce")
				MSG->hdr_auth_info.set_cnonce((yyvsp[(1) - (1)].yyt_param)->value);
			else if ((yyvsp[(1) - (1)].yyt_param)->name == "nc") {
				MSG->hdr_auth_info.set_nonce_count(
							hex2int((yyvsp[(1) - (1)].yyt_param)->value));
			}
			else {
				YYERROR;
			}

			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_param)); delete (yyvsp[(1) - (1)].yyt_param); }
    break;

  case 392:
#line 1382 "parser.yxx"
    {
			(yyval.yyt_dig_resp) = new t_digest_response();
			MEMMAN_NEW((yyval.yyt_dig_resp));
			if (!(yyval.yyt_dig_resp)->set_attr(*(yyvsp[(1) - (1)].yyt_param))) {
				MEMMAN_DELETE((yyval.yyt_dig_resp)); delete (yyval.yyt_dig_resp);
				YYERROR;
			}
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_param)); delete (yyvsp[(1) - (1)].yyt_param); }
    break;

  case 393:
#line 1390 "parser.yxx"
    {
			(yyval.yyt_dig_resp) = (yyvsp[(1) - (3)].yyt_dig_resp);
			if (!(yyval.yyt_dig_resp)->set_attr(*(yyvsp[(3) - (3)].yyt_param))) {
				YYERROR;
			}
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_param)); delete (yyvsp[(3) - (3)].yyt_param); }
    break;

  case 394:
#line 1398 "parser.yxx"
    {
			(yyval.yyt_params) = new list<t_parameter>;
			MEMMAN_NEW((yyval.yyt_params));
			(yyval.yyt_params)->push_back(*(yyvsp[(1) - (1)].yyt_param));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_param)); delete (yyvsp[(1) - (1)].yyt_param); }
    break;

  case 395:
#line 1403 "parser.yxx"
    {
			(yyval.yyt_params) = (yyvsp[(1) - (3)].yyt_params);
			(yyval.yyt_params)->push_back(*(yyvsp[(3) - (3)].yyt_param));
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_param)); delete (yyvsp[(3) - (3)].yyt_param); }
    break;

  case 396:
#line 1409 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 397:
#line 1409 "parser.yxx"
    {
			(yyval.yyt_credentials) = new t_credentials;
			MEMMAN_NEW((yyval.yyt_credentials));
			(yyval.yyt_credentials)->auth_scheme = AUTH_DIGEST;
			(yyval.yyt_credentials)->digest_response = *(yyvsp[(3) - (3)].yyt_dig_resp);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_dig_resp)); delete (yyvsp[(3) - (3)].yyt_dig_resp); }
    break;

  case 398:
#line 1415 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 399:
#line 1415 "parser.yxx"
    {
			(yyval.yyt_credentials) = new t_credentials;
			MEMMAN_NEW((yyval.yyt_credentials));
			(yyval.yyt_credentials)->auth_scheme = *(yyvsp[(1) - (3)].yyt_str);
			(yyval.yyt_credentials)->auth_params = *(yyvsp[(3) - (3)].yyt_params);
			MEMMAN_DELETE((yyvsp[(1) - (3)].yyt_str)); delete (yyvsp[(1) - (3)].yyt_str);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_params)); delete (yyvsp[(3) - (3)].yyt_params); }
    break;

  case 400:
#line 1424 "parser.yxx"
    { CTXT_AUTH_SCHEME; }
    break;

  case 401:
#line 1424 "parser.yxx"
    {
			MSG->hdr_authorization.add_credentials(*(yyvsp[(2) - (2)].yyt_credentials));
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_credentials)); delete (yyvsp[(2) - (2)].yyt_credentials); }
    break;

  case 402:
#line 1429 "parser.yxx"
    {
			(yyval.yyt_dig_chlg) = new t_digest_challenge();
			MEMMAN_NEW((yyval.yyt_dig_chlg));
			if (!(yyval.yyt_dig_chlg)->set_attr(*(yyvsp[(1) - (1)].yyt_param))) {
				MEMMAN_DELETE((yyval.yyt_dig_chlg)); delete (yyval.yyt_dig_chlg);
				YYERROR;
			}
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_param)); delete (yyvsp[(1) - (1)].yyt_param); }
    break;

  case 403:
#line 1437 "parser.yxx"
    {
			(yyval.yyt_dig_chlg) = (yyvsp[(1) - (3)].yyt_dig_chlg);
			if (!(yyval.yyt_dig_chlg)->set_attr(*(yyvsp[(3) - (3)].yyt_param))) {
				YYERROR;
			}
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_param)); delete (yyvsp[(3) - (3)].yyt_param); }
    break;

  case 404:
#line 1445 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 405:
#line 1445 "parser.yxx"
    {
			(yyval.yyt_challenge) = new t_challenge;
			MEMMAN_NEW((yyval.yyt_challenge));
			(yyval.yyt_challenge)->auth_scheme = AUTH_DIGEST;
			(yyval.yyt_challenge)->digest_challenge = *(yyvsp[(3) - (3)].yyt_dig_chlg);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_dig_chlg)); delete (yyvsp[(3) - (3)].yyt_dig_chlg); }
    break;

  case 406:
#line 1451 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 407:
#line 1451 "parser.yxx"
    {
			(yyval.yyt_challenge) = new t_challenge;
			MEMMAN_NEW((yyval.yyt_challenge));
			(yyval.yyt_challenge)->auth_scheme = *(yyvsp[(1) - (3)].yyt_str);
			(yyval.yyt_challenge)->auth_params = *(yyvsp[(3) - (3)].yyt_params);
			MEMMAN_DELETE((yyvsp[(1) - (3)].yyt_str)); delete (yyvsp[(1) - (3)].yyt_str);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_params)); delete (yyvsp[(3) - (3)].yyt_params); }
    break;

  case 408:
#line 1460 "parser.yxx"
    { CTXT_AUTH_SCHEME; }
    break;

  case 409:
#line 1460 "parser.yxx"
    {
				MSG->hdr_proxy_authenticate.set_challenge(*(yyvsp[(2) - (2)].yyt_challenge));
				MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_challenge)); delete (yyvsp[(2) - (2)].yyt_challenge); }
    break;

  case 410:
#line 1465 "parser.yxx"
    { CTXT_AUTH_SCHEME; }
    break;

  case 411:
#line 1465 "parser.yxx"
    {
				MSG->hdr_proxy_authorization.
							add_credentials(*(yyvsp[(2) - (2)].yyt_credentials));
				MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_credentials)); delete (yyvsp[(2) - (2)].yyt_credentials); }
    break;

  case 412:
#line 1471 "parser.yxx"
    { CTXT_AUTH_SCHEME; }
    break;

  case 413:
#line 1471 "parser.yxx"
    {
				MSG->hdr_www_authenticate.set_challenge(*(yyvsp[(2) - (2)].yyt_challenge));
				MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_challenge)); delete (yyvsp[(2) - (2)].yyt_challenge); }
    break;

  case 414:
#line 1476 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 415:
#line 1476 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 416:
#line 1476 "parser.yxx"
    {
			MSG->hdr_rseq.set_resp_nr((yyvsp[(2) - (3)].yyt_ulong)); }
    break;

  case 417:
#line 1480 "parser.yxx"
    { CTXT_NUM; }
    break;

  case 418:
#line 1480 "parser.yxx"
    { CTXT_INITIAL; }
    break;

  case 419:
#line 1480 "parser.yxx"
    {
			MSG->hdr_rack.set_resp_nr((yyvsp[(2) - (5)].yyt_ulong));
			MSG->hdr_rack.set_cseq_nr((yyvsp[(3) - (5)].yyt_ulong));
			MSG->hdr_rack.set_method(*(yyvsp[(5) - (5)].yyt_str));
			MEMMAN_DELETE((yyvsp[(5) - (5)].yyt_str)); delete (yyvsp[(5) - (5)].yyt_str); }
    break;

  case 420:
#line 1487 "parser.yxx"
    {
			MSG->hdr_event.set_event_type(tolower(*(yyvsp[(1) - (2)].yyt_str)));
			list<t_parameter>::const_iterator i;
			for (i = (yyvsp[(2) - (2)].yyt_params)->begin(); i != (yyvsp[(2) - (2)].yyt_params)->end(); i++) {
				if (i->name == "id") {
					MSG->hdr_event.set_id(i->value);
				} else {
					MSG->hdr_event.add_event_param(*i);
				}
			}
			MEMMAN_DELETE((yyvsp[(1) - (2)].yyt_str)); delete (yyvsp[(1) - (2)].yyt_str);
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_params)); delete (yyvsp[(2) - (2)].yyt_params); }
    break;

  case 421:
#line 1501 "parser.yxx"
    {
				MSG->hdr_allow_events.add_event_type(tolower(*(yyvsp[(1) - (1)].yyt_str)));
				MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 422:
#line 1504 "parser.yxx"
    {
		      		MSG->hdr_allow_events.add_event_type(tolower(*(yyvsp[(3) - (3)].yyt_str)));
				MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_str)); delete (yyvsp[(3) - (3)].yyt_str); }
    break;

  case 423:
#line 1509 "parser.yxx"
    {
				MSG->hdr_subscription_state.set_substate(tolower(*(yyvsp[(1) - (2)].yyt_str)));
				list<t_parameter>::const_iterator i;
				for (i = (yyvsp[(2) - (2)].yyt_params)->begin(); i != (yyvsp[(2) - (2)].yyt_params)->end(); i++) {
					if (i->name == "reason") {
						MSG->hdr_subscription_state.
							set_reason(tolower(i->value));
					} else if (i->name == "expires") {
						MSG->hdr_subscription_state.
							set_expires(strtoul(
							  i->value.c_str(),
							  NULL, 10));
					} else if (i->name == "retry-after") {
						MSG->hdr_subscription_state.
							set_retry_after(strtoul(
							  i->value.c_str(),
							  NULL, 10));
					} else {
						MSG->hdr_subscription_state.
							add_extension(*i);
					}
				}
				MEMMAN_DELETE((yyvsp[(1) - (2)].yyt_str)); delete (yyvsp[(1) - (2)].yyt_str);
				MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_params)); delete (yyvsp[(2) - (2)].yyt_params); }
    break;

  case 424:
#line 1535 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 425:
#line 1535 "parser.yxx"
    {
			MSG->hdr_refer_to.set_display((yyvsp[(2) - (3)].yyt_from_addr)->display);
			MSG->hdr_refer_to.set_uri((yyvsp[(2) - (3)].yyt_from_addr)->uri);
			MSG->hdr_refer_to.set_params(*(yyvsp[(3) - (3)].yyt_params));
			MEMMAN_DELETE((yyvsp[(2) - (3)].yyt_from_addr)); delete (yyvsp[(2) - (3)].yyt_from_addr);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_params)); delete (yyvsp[(3) - (3)].yyt_params); }
    break;

  case 426:
#line 1543 "parser.yxx"
    { CTXT_URI_SPECIAL; }
    break;

  case 427:
#line 1543 "parser.yxx"
    {
			MSG->hdr_referred_by.set_display((yyvsp[(2) - (3)].yyt_from_addr)->display);
			MSG->hdr_referred_by.set_uri((yyvsp[(2) - (3)].yyt_from_addr)->uri);
			list<t_parameter>::const_iterator i;
			for (i = (yyvsp[(3) - (3)].yyt_params)->begin(); i != (yyvsp[(3) - (3)].yyt_params)->end(); i++) {
				if (i->name == "cid") {
					MSG->hdr_referred_by.set_cid(i->value);
				} else {
					MSG->hdr_referred_by.add_param(*i);
				}
			}
			MEMMAN_DELETE((yyvsp[(2) - (3)].yyt_from_addr)); delete (yyvsp[(2) - (3)].yyt_from_addr);
			MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_params)); delete (yyvsp[(3) - (3)].yyt_params); }
    break;

  case 428:
#line 1558 "parser.yxx"
    {
			string value(tolower(*(yyvsp[(1) - (2)].yyt_str)));
			if (value != "true" && value != "false") {
				YYERROR;
			}
			MSG->hdr_refer_sub.set_create_refer_sub(value == "true");
			MSG->hdr_refer_sub.set_extensions(*(yyvsp[(2) - (2)].yyt_params));
			MEMMAN_DELETE((yyvsp[(1) - (2)].yyt_str)); delete (yyvsp[(1) - (2)].yyt_str);
			MEMMAN_DELETE((yyvsp[(2) - (2)].yyt_params)); delete (yyvsp[(2) - (2)].yyt_params); }
    break;

  case 429:
#line 1569 "parser.yxx"
    {
			MSG->hdr_sip_etag.set_etag(*(yyvsp[(1) - (1)].yyt_str));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 430:
#line 1574 "parser.yxx"
    {
			MSG->hdr_sip_if_match.set_etag(*(yyvsp[(1) - (1)].yyt_str));
			MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 431:
#line 1579 "parser.yxx"
    {
				bool ret = MSG->hdr_request_disposition.set_directive(*(yyvsp[(1) - (1)].yyt_str));
				if (!ret) YYERROR;
				MEMMAN_DELETE((yyvsp[(1) - (1)].yyt_str)); delete (yyvsp[(1) - (1)].yyt_str); }
    break;

  case 432:
#line 1583 "parser.yxx"
    {
				bool ret = MSG->hdr_request_disposition.set_directive(*(yyvsp[(3) - (3)].yyt_str));
				if (!ret) YYERROR;
				MEMMAN_DELETE((yyvsp[(3) - (3)].yyt_str)); delete (yyvsp[(3) - (3)].yyt_str); }
    break;


/* Line 1267 of yacc.c.  */
#line 4885 "parser.cxx"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 1588 "parser.yxx"


void
yyerror (const char *s)  /* Called by yyparse on error */
{
  // printf ("%s\n", s);
}

