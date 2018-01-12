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
     IS = 258,
     CLASS = 259,
     VAR = 260,
     EXTENDS = 261,
     DEF = 262,
     OVERRIDE = 263,
     RETURN = 264,
     AS = 265,
     IF = 266,
     THEN = 267,
     ELSE = 268,
     AFF = 269,
     NEWC = 270,
     VOIDC = 271,
     INTC = 272,
     STRINGC = 273,
     THIS = 274,
     SUPER = 275,
     RESULT = 276,
     ADD = 277,
     SUB = 278,
     MUL = 279,
     DIV = 280,
     CONCAT = 281,
     OBJECT = 282,
     Id = 283,
     Classname = 284,
     Cste = 285,
     RelOp = 286,
     UNARY = 287
   };
#endif
/* Tokens.  */
#define IS 258
#define CLASS 259
#define VAR 260
#define EXTENDS 261
#define DEF 262
#define OVERRIDE 263
#define RETURN 264
#define AS 265
#define IF 266
#define THEN 267
#define ELSE 268
#define AFF 269
#define NEWC 270
#define VOIDC 271
#define INTC 272
#define STRINGC 273
#define THIS 274
#define SUPER 275
#define RESULT 276
#define ADD 277
#define SUB 278
#define MUL 279
#define DIV 280
#define CONCAT 281
#define OBJECT 282
#define Id 283
#define Classname 284
#define Cste 285
#define RelOp 286
#define UNARY 287




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

