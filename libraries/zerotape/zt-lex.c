/* zt-lex.c */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "fortify/fortify.h"

#include "zt-gram.h"

#include "zt-lex.h"

#include "zt-lex-impl.h"

/* ----------------------------------------------------------------------- */

static int ztlex_fgetc(ztlex_t *lex)
{
  int c = fgetc(lex->file);

  if (c == '\n')
  {
    lex->line++;
    lex->prevcolumn = lex->column;
    lex->column     = 1;
  }
  else
  {
    lex->column++;
  }

  return c;
}

static void ztlex_fungetc(int c, ztlex_t *lex)
{
  if (c == '\n')
  {
    assert(lex->line >= 0);
    lex->line--;
    assert(lex->prevcolumn >= 0);
    lex->column     = lex->prevcolumn;
    lex->prevcolumn = -1; /* invalidate */
  }
  else
  {
    lex->column--;
  }

  ungetc(c, lex->file);
}

ztlex_t *ztlex_from_file(ztlex_mallocfn_t *mallocfn,
                         ztlex_freefn_t   *freefn,
                         const char       *filename)
{
  ztlex_t *lex = NULL;

  lex = mallocfn(sizeof(*lex));
  if (lex == NULL)
    return NULL;

  lex->file = fopen(filename, "r");
  if (lex->file == NULL)
  {
    freefn(lex);
    return NULL;
  }

  lex->string     = NULL;
  lex->length     = 0;
  lex->index      = 0;

  lex->line       = 1;
  lex->column     = 1;
  lex->prevcolumn = -1;

  lex->getC       = ztlex_fgetc;
  lex->ungetC     = ztlex_fungetc;

  lex->freefn     = freefn;

  return lex;
}

static int ztlex_sgetc(ztlex_t *lex)
{
  int c;

  assert(lex->string);
  assert(lex->index <= lex->length);

  if (lex->index == lex->length)
    return EOF;

  c = lex->string[lex->index++];
  if (c == '\n')
  {
    lex->line++;
    lex->prevcolumn = lex->column;
    lex->column = 1;
  }
  else
  {
    lex->column++;
  }

  return c;
}

static void ztlex_sungetc(int c, ztlex_t *lex)
{
  assert(lex->string);
  assert(lex->index >= 1);
  assert(lex->index <= lex->length);

  if (c == '\n')
  {
    assert(lex->line >= 0);
    lex->line--;
    assert(lex->prevcolumn >= 0);
    lex->column = lex->prevcolumn;
    lex->prevcolumn = -1; /* invalidate */
  }
  else
  {
    lex->column--;
  }

  lex->index--;

  assert(lex->string[lex->index] == c);
}

ztlex_t *ztlex_from_string(ztlex_mallocfn_t *mallocfn,
                           ztlex_freefn_t   *freefn,
                           const char       *string)
{
  ztlex_t *lex = NULL;

  lex = mallocfn(sizeof(*lex));
  if (lex == NULL)
    return NULL;

  lex->file       = NULL;

  lex->string     = string; /* FIXME: Copy string */
  lex->length     = strlen(string);
  lex->index      = 0;

  lex->line       = 1;
  lex->column     = 1;
  lex->prevcolumn = -1;

  lex->getC       = ztlex_sgetc;
  lex->ungetC     = ztlex_sungetc;

  lex->freefn     = freefn;

  return lex;
}

void ztlex_destroy(ztlex_t *lex)
{
  if (lex == NULL)
    return;

  if (lex->file)
  {
    if (!feof(lex->file))
      fprintf(stderr, "warning: lexer closed with bytes pending\n");

    fclose(lex->file);
  }
  else
  {
    int remaining = (int)(lex->length - lex->index);
    if (remaining)
      fprintf(stderr, "warning: lexer closed with %d bytes pending\n",
              remaining);
  }

  lex->freefn(lex);
}

/* The is*() functions return the length of token, 0 if not valid, or EOF. */

/* '$'[:hexdigit:]+ */
int ztlex_isdollarhex(ztlex_t *lex)
{
  int i = 0;
  int c;

  c = lex->getC(lex);
  if (c == EOF)
    return EOF;

  if (c != '$')
  {
    lex->ungetC(c, lex);
    return 0;
  }

  do
  {
    lex->lexeme[i++] = c;
    c = lex->getC(lex);
  }
  while (isxdigit(c) && i < MAXLEXEME - 1);

  if (c != EOF && !isxdigit(c))
    lex->ungetC(c, lex);

  if (i < 2)
  {
    while (--i >= 0)
      lex->ungetC(lex->lexeme[i], lex);
    return 0;
  }

  lex->lexeme[i] = '\0';
  return i;
}

/* '0x'[:hexdigit:]+ */
int ztlex_ishex(ztlex_t *lex)
{
  int i = 0;
  int c;

  c = lex->getC(lex);
  if (c == EOF)
    return EOF;

  if (c != '0')
  {
    lex->ungetC(c, lex);
    return 0;
  }

  lex->lexeme[i++] = c;

  c = lex->getC(lex);
  if (c != 'x') /* including EOF */
  {
    if (c != EOF)
      lex->ungetC(c, lex);
    goto undo;
  }

  do
  {
    lex->lexeme[i++] = c;
    c = lex->getC(lex);
  }
  while (isxdigit(c) && i < MAXLEXEME - 1);

  if (c != EOF && !isxdigit(c))
    lex->ungetC(c, lex);

  if (i < 3)
  {
undo:
    while (--i >= 0)
      lex->ungetC(lex->lexeme[i], lex);
    return 0;
  }

  lex->lexeme[i] = '\0';
  return i;
}

/* [:digit:]'.'[:digit:][:digit:] */
int ztlex_isdecimal(ztlex_t *lex)
{
  int i = 0;
  int c;

  c = lex->getC(lex);
  if (c == EOF)
    return EOF;

  if (!isdigit(c))
  {
    lex->ungetC(c, lex);
    return 0;
  }

  lex->lexeme[i++] = c;

  c = lex->getC(lex);
  if (c != '.') /* including EOF */
  {
    if (c != EOF)
      lex->ungetC(c, lex);
    goto undo;
  }

  lex->lexeme[i++] = c;

  c = lex->getC(lex);
  if (!isdigit(c)) /* including EOF */
  {
    if (c != EOF)
      lex->ungetC(c, lex);
    goto undo;
  }

  lex->lexeme[i++] = c;

  c = lex->getC(lex);
  if (!isdigit(c)) /* including EOF */
  {
    if (c != EOF)
      lex->ungetC(c, lex);
    goto undo;
  }

  lex->lexeme[i++] = c;

  lex->lexeme[i] = '\0';
  return i;

undo:
  while (--i >= 0)
    lex->ungetC(lex->lexeme[i], lex);
  return 0;
}

/* [:digit:]+ */
int ztlex_isinteger(ztlex_t *lex)
{
  int i = 0;
  int c;

  c = lex->getC(lex);
  if (c == EOF)
    return EOF;

  if (!isdigit(c))
  {
    lex->ungetC(c, lex);
    return 0;
  }

  do
  {
    lex->lexeme[i++] = c;
    c = lex->getC(lex);
  }
  while (isdigit(c) && i < MAXLEXEME - 1);

  if (c != EOF && !isdigit(c))
    lex->ungetC(c, lex);

  lex->lexeme[i] = '\0';
  return i;
}

/* "nil" */
int ztlex_isnil(ztlex_t *lex)
{
  int i = 0;
  int c;

  c = lex->getC(lex);
  if (c == EOF)
    return EOF;
  if (c != 'n')
  {
    lex->ungetC(c, lex);
    return 0;
  }
  lex->lexeme[i++] = c;

  c = lex->getC(lex);
  if (c != 'i')
  {
    if (c != EOF)
      lex->ungetC(c, lex);
    goto undo;
  }
  lex->lexeme[i++] = c;

  c = lex->getC(lex);
  if (c != 'l')
  {
    if (c != EOF)
      lex->ungetC(c, lex);
    goto undo;
  }
  lex->lexeme[i++] = c;

  lex->lexeme[i] = '\0';
  return 3;

undo:
  while (--i >= 0)
    lex->ungetC(lex->lexeme[i], lex);
  return (c == EOF) ? c : 0;
}

/* [:alpha:_][:alnum:_]* */
int ztlex_isname(ztlex_t *lex)
{
  int i = 0;
  int c;

  c = lex->getC(lex);
  if (c == EOF)
    return EOF;

  if (!(isalpha(c) || c == '_'))
  {
    lex->ungetC(c, lex);
    return 0;
  }

  do
  {
    lex->lexeme[i++] = c;
    c = lex->getC(lex);
  }
  while ((isalnum(c) || c == '_') && i < MAXLEXEME - 1);

  if (c != EOF && !(isalnum(c) || c == '_'))
    lex->ungetC(c, lex);

  lex->lexeme[i] = '\0';
  return i;
}

int ztlex_next_token(ztlex_t     *lex,
                     ztlextok_t  *token,
               const ztlexinf_t **info)
{
  static const struct
  {
    int        (*fn)(ztlex_t *);
    ztlextok_t token;
  }
  map[] =
  {
    { ztlex_isdollarhex, ZTTOKEN_DOLLARHEX },
    { ztlex_ishex,       ZTTOKEN_HEX       },
    { ztlex_isdecimal,   ZTTOKEN_DECIMAL   },
    { ztlex_isinteger,   ZTTOKEN_INT       },
    { ztlex_isnil,       ZTTOKEN_NIL       },
    { ztlex_isname,      ZTTOKEN_NAME      },
  };

  int c;
  int i;

  assert(lex);
  assert(token);
  assert(info);

  *info = NULL;

again:
  /* absorb all leading whitespace */
  do
  {
    c = lex->getC(lex);
    if (c == EOF)
      return 0;
  }
  while (isspace(c));

  lex->ungetC(c, lex);

  /* absorb comments */
  {
    c = lex->getC(lex);
    if (c == EOF)
      return 0;

    if (c != '/')
    {
      lex->ungetC(c, lex);
      goto not;
    }

    c = lex->getC(lex);
    if (c != '/') /* including EOF */
    {
      if (c != EOF)
        lex->ungetC(c, lex);
      lex->ungetC('/', lex);
      goto not;
    }

    /* we've seen // - absorb all the rest */
    do
      c = lex->getC(lex);
    while (c != '\n');

    if (c != EOF)
      lex->ungetC(c, lex);

    goto again;
  }

not:
  /* try the complex tokens first */
  for (i = 0; i < (int) NELEMS(map); i++)
  {
    int len;

    len = map[i].fn(lex);
    if (len == EOF)
    {
      return 0;
    }
    else if (len)
    {
      *token = map[i].token;

      lex->info.line   = lex->line;
      lex->info.column = lex->column - len; /* report start of token */
      lex->info.length = len;
      memcpy(&lex->info.lexeme[0], lex->lexeme, len + 1);
      *info = &lex->info;
      return 1;
    }
  }

  /* deal with basic tokens */
  c = lex->getC(lex);
  switch (c)
  {
    case '(': *token = ZTTOKEN_LPAREN;    break;
    case ')': *token = ZTTOKEN_RPAREN;    break;
    case '*': *token = ZTTOKEN_TIMES;     break;
    case '+': *token = ZTTOKEN_PLUS;      break;
    case ',': *token = ZTTOKEN_COMMA;     break;
    case '-': *token = ZTTOKEN_MINUS;     break;
    case '/': *token = ZTTOKEN_DIVIDE;    break;
    case ':': *token = ZTTOKEN_COLON;     break;
    case ';': *token = ZTTOKEN_SEMICOLON; break;
    case '=': *token = ZTTOKEN_EQUALS;    break;
    case '[': *token = ZTTOKEN_LSQBRA;    break;
    case ']': *token = ZTTOKEN_RSQBRA;    break;
    case '{': *token = ZTTOKEN_LBRACE;    break;
    case '}': *token = ZTTOKEN_RBRACE;    break;
    default:
      if (c != EOF) {
        lex->ungetC(c, lex);
        fprintf(stderr, "Unknown token '%c' at line %d column %d\n", c, lex->line, lex->column);
      }
      return 0;
  }

  lex->info.line   = lex->line;
  lex->info.column = lex->column - 1; /* report start of token */
  lex->info.length = 0; /* FIXME: Should basic tokens have any length? */
  *info = &lex->info;
  return 1;
}

/* ----------------------------------------------------------------------- */

/* vim: set ts=8 sts=2 sw=2 et: */
