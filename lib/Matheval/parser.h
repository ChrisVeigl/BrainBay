#ifndef BISON_PARSER_H
# define BISON_PARSER_H

#ifndef YYSTYPE
typedef union {
  Node *node;
  Record *record;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	NUMBER	257
# define	CONSTANT	258
# define	VARIABLE	259
# define	FUNCTION	260
# define	NEG	261
# define	END	262


extern YYSTYPE yylval;

#endif /* not BISON_PARSER_H */

