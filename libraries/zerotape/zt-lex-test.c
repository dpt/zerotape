/* zt-lex-test.c */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "zt-lex.h"
#include "zt-gram.h"

#include "zt-lex-impl.h"

/* ----------------------------------------------------------------------- */

const char *ztlex_tokname(ztlextok_t t)
{
  switch (t)
  {
  /* "(" */ case ZTTOKEN_LPAREN: return "LPAREN";
  /* ")" */ case ZTTOKEN_RPAREN: return "RPAREN";
  /* "*" */ case ZTTOKEN_TIMES: return "TIMES";
  /* "+" */ case ZTTOKEN_PLUS: return "PLUS";
  /* "," */ case ZTTOKEN_COMMA: return "COMMA";
  /* "-" */ case ZTTOKEN_MINUS: return "MINUS";
  /* "/" */ case ZTTOKEN_DIVIDE: return "DIVIDE";
  /* ";" */ case ZTTOKEN_SEMICOLON: return "SEMICOLON";
  /* "=" */ case ZTTOKEN_EQUALS: return "EQUALS";
  /* "[" */ case ZTTOKEN_LSQBRA: return "LSQBRA";
  /* "]" */ case ZTTOKEN_RSQBRA: return "RSQBRA";
  /* "{" */ case ZTTOKEN_LBRACE: return "LBRACE";
  /* "}" */ case ZTTOKEN_RBRACE: return "RBRACE";

  /*     */ case ZTTOKEN_DECIMAL: return "DECIMAL";
  /*     */ case ZTTOKEN_DOLLARHEX: return "DOLLARHEX";
  /*     */ case ZTTOKEN_HEX: return "HEX";
  /*     */ case ZTTOKEN_INT: return "INT";
  /*     */ case ZTTOKEN_NAME: return "NAME";
  /*     */ case ZTTOKEN_NIL: return "NIL";

  default:
    return "Unknown token";
  }
}

/* ----------------------------------------------------------------------- */

size_t ztlex_get_cursor(const ztlex_t *lex)
{
  assert(lex->string);
  return lex->index;
}

/* ----------------------------------------------------------------------- */

int ztlex_stringtest(const char *string)
{
  ztlex_t          *lex;
  ztlextok_t        token;
  const ztlexinf_t *info;
  int               rc;

  printf("==> Tokenise \"%s\"\n", string);

  lex = ztlex_from_string(malloc, free, string);
  if (lex == NULL)
    return 1;

  for (;;)
  {
    rc = ztlex_next_token(lex, &token, &info);
    if (rc == 0)
      break;
    if (token == ZTTOKEN_NAME)
      printf("- %s [%d] (\"%s\")\n", ztlex_tokname(token), token, info->lexeme);
    else
      printf("- %s [%d]\n", ztlex_tokname(token), token);
  }
  printf("- (end)\n\n");

  ztlex_destroy(lex);

  return 0;
}

/* ----------------------------------------------------------------------- */

int ztlex_selftest(void)
{
  static const struct
  {
    const int  id;
    const char *input;
    int       (*fn)(ztlex_t *);
    int         count;
    const char *expectedLexeme;
    size_t      newCursorPos;
  }
  cases[] =
  {
    {   1, "",      ztlex_isdollarhex, EOF, ""      , 0 },
    {   2, " ",     ztlex_isdollarhex,   0, ""      , 0 },
    {   3, "$",     ztlex_isdollarhex,   0, ""      , 0 },
    {   4, "$$",    ztlex_isdollarhex,   0, ""      , 0 },
    {   5, "$d",    ztlex_isdollarhex,   2, "$d"    , 2 },
    {   6, "$d$",   ztlex_isdollarhex,   2, "$d"    , 2 },
    {   7, "$dae",  ztlex_isdollarhex,   4, "$dae"  , 4 },
    {   8, "$DAVE", ztlex_isdollarhex,   3, "$DA"   , 3 },

    {  20, "",      ztlex_ishex,       EOF, ""      , 0 },
    {  21, " ",     ztlex_ishex,         0, ""      , 0 },
    {  22, "0",     ztlex_ishex,         0, ""      , 0 },
    {  23, "x",     ztlex_ishex,         0, ""      , 0 },
    {  24, "0x",    ztlex_ishex,         0, ""      , 0 },
    {  25, "0xx",   ztlex_ishex,         0, ""      , 0 },
    {  26, "0x0x",  ztlex_ishex,         3, "0x0"   , 3 },
    {  27, "0xd",   ztlex_ishex,         3, "0xd"   , 3 },
    {  28, "0xDg",  ztlex_ishex,         3, "0xD"   , 3 },
    {  29, "0xDEF", ztlex_ishex,         5, "0xDEF" , 5 },

    {  40, "",      ztlex_isdecimal,   EOF, ""      , 0 },
    {  41, " ",     ztlex_isdecimal,     0, ""      , 0 },
    {  42, ".",     ztlex_isdecimal,     0, ""      , 0 },
    {  43, "..",    ztlex_isdecimal,     0, ""      , 0 },
    {  44, ".1",    ztlex_isdecimal,     0, ""      , 0 },
    {  45, "1",     ztlex_isdecimal,     0, ""      , 0 },
    {  46, "1.2",   ztlex_isdecimal,     0, ""      , 0 },
    {  47, "1.23",  ztlex_isdecimal,     4, "1.23"  , 4 },
    {  48, "1.23.", ztlex_isdecimal,     4, "1.23"  , 4 },
    {  49, "1.234", ztlex_isdecimal,     4, "1.23"  , 4 },

    {  60, "",      ztlex_isinteger,   EOF, ""      , 0 },
    {  61, " ",     ztlex_isinteger,     0, ""      , 0 },
    {  62, "-1",    ztlex_isinteger,     0, ""      , 0 },
    {  63, "0",     ztlex_isinteger,     1, "0"     , 1 },
    {  64, "0A",    ztlex_isinteger,     1, "0"     , 1 },
    {  65, "1",     ztlex_isinteger,     1, "1"     , 1 },
    {  66, "23",    ztlex_isinteger,     2, "23"    , 2 },
    {  67, "23 ",   ztlex_isinteger,     2, "23"    , 2 },
    {  68, "45678", ztlex_isinteger,     5, "45678" , 5 },

    {  80, "",      ztlex_isname,      EOF, ""      , 0 },
    {  81, " ",     ztlex_isname,        0, ""      , 0 },
    {  82, "0",     ztlex_isname,        0, ""      , 0 },
    {  83, "A",     ztlex_isname,        1, "A"     , 1 },
    {  84, "_",     ztlex_isname,        1, "_"     , 1 },
    {  85, "a0",    ztlex_isname,        2, "a0"    , 2 },
    {  86, "A0$",   ztlex_isname,        2, "A0"    , 2 },
    {  87, "a0_",   ztlex_isname,        3, "a0_"   , 3 },

    { 100, "",      ztlex_isnil,       EOF, ""      , 0 },
    { 101, "n",     ztlex_isnil,       EOF, ""      , 0 },
    { 102, "ni",    ztlex_isnil,       EOF, ""      , 0 },
    { 103, " ",     ztlex_isnil,         0, ""      , 0 },
    { 104, "NIL",   ztlex_isnil,         0, ""      , 0 },
    { 105, "nil",   ztlex_isnil,         3, "nil"   , 3 },
    { 106, "nill",  ztlex_isnil,         3, "nil"   , 3 },
  };

  int totaltests;
  int totalpassed;
  int i;

  printf("==> Lexer self-test\n");

  totaltests    = 0;
  totalpassed = 0;
  for (i = 0; i < (int) NELEMS(cases); i++)
  {
    int      passed;
    ztlex_t *lex;
    int      count;

    printf("--> Test id %d\n", cases[i].id);

    passed = 1;
    lex    = ztlex_from_string(malloc, free, cases[i].input);
    count  = cases[i].fn(lex);
    if (count != cases[i].count)
    {
      printf("diff in test id %d ('%s'): count was %u but we expected %u\n",
             cases[i].id, cases[i].input, count, cases[i].count);
      passed = 0;
    }

    if (cases[i].count > 0 && strcmp(lex->lexeme, cases[i].expectedLexeme) != 0)
    {
      printf("diff in test id %d ('%s'): lexeme was '%s' but we expected '%s'\n",
             cases[i].id, cases[i].input, lex->lexeme, cases[i].expectedLexeme);
      passed = 0;
    }

    if (cases[i].newCursorPos != ztlex_get_cursor(lex))
    {
      printf("diff in test id %d ('%s'): cursor was '%lu' but we expected '%lu'\n",
             cases[i].id, cases[i].input, ztlex_get_cursor(lex), cases[i].newCursorPos);
      passed = 0;
    }

    ztlex_destroy(lex);

    totalpassed += passed;
    totaltests++;
  }

  printf("%d/%d tests passed\n", totalpassed, totaltests);
  printf("(end)\n\n");

  return 0;
}

/* ----------------------------------------------------------------------- */

int ztlex_dump_filename_to_tokens(const char *filename)
{
  ztlex_t          *lex;
  ztlextok_t        token;
  const ztlexinf_t *info;

  lex = ztlex_from_file(malloc, free, filename);
  if (lex == NULL)
    return 1;

  while (ztlex_next_token(lex, &token, &info))
  {
    printf("%s[%d] [%d,%d] ", ztlex_tokname(token), token, info->line, info->column);
    if (info->length > 0)
      printf("%s", info->lexeme);
    printf("\n");
  }
  ztlex_destroy(lex);

  return 0;
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
