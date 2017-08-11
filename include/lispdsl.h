/*
 *  Lisp-DSL is Copyleft (C) 2016, The 1st Middle School in Yongsheng Lijiang China
 *  please contact with <diyer175@hotmail.com> if you have any problems.
 *
 *  This project is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License(GPL)
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This project is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 */

#ifndef LISPDSL_H_
#define LISPDSL_H_

#include <assert.h>
#include <cstddef>
#include <iostream>

namespace DSL {

/*
 * Switchs
 */

#define USES(obj)    (defined USES_##obj && USES_##obj)
/** @def HAVE()
 * indicates whether the host machine supports 'obj'
 */
#define HAVE(obj)   (defined HAVE_##obj && HAVE_##obj)
/** @def ENABLE()
 * indicates whether the 'obj' is enabled
 */
#define ENABLE(obj) (defined ENABLE_##obj && ENABLE_##obj)

////////////////////////////////////////////////////////////////////////////////

/*
 * cdefs
 */

/** @def UNLIKELY
 * Branch prediction. Unlikely
 * @returns the value of expression.
 * @param   expr  The expression.
 */

#ifdef __GNUC__
# if __GNUC__ >= 3 && !defined(FORTIFY_RUNNING)
#  ifndef LIKELY
#  define LIKELY(expr)       __builtin_expect(!!(expr), 1)
#  endif
#  ifndef UNLIKELY
#  define UNLIKELY(expr)     __builtin_expect(!!(expr), 0)
#  endif
# else
#  ifndef LIKELY
#  define LIKELY(expr)       (expr)
#  endif
#  ifndef UNLIKELY
#  define UNLIKELY(expr)     (expr)
#  endif
# endif
#else
# ifndef LIKELY
# define LIKELY(expr)       (expr)
# endif
# ifndef UNLIKELY
# define UNLIKELY(expr)     (expr)
# endif
#endif

#if __GNUC__
#define LP_CURRENT_FUNCTION __PRETTY_FUNCTION__
#else
#define LP_CURRENT_FUNCTION __FUNCTION__
#endif
#define LP_CURRENT_FILE __FILE__
#define LP_CURRENT_LINE __LINE__

/** @def NULL
 * Null pointer
 */
#ifndef NULL
# ifdef __cplusplus
# define NULL 0
#else
# define NULL (void*)0
#endif
#endif

/** @def UNUSED
 * Avoid the 'unused parameter' warning.
 */
#define UNUSED(var) (void)var

////////////////////////////////////////////////////////////////////////////////


/*
 * Assertions
 */
#if ENABLE(ASSERTIONS)

static void lpAssertFailure(const char* file, int line, const char* func, const char*expr) {
    UNUSED(file);
    UNUSED(line);
    UNUSED(func);
    UNUSED(expr);

}
static void lpAssertFailureLog(const char* format,...) {
    UNUSED(format);
}

#define LP_ASSERT_PANIC() assert(0)

/** @todo: implement the variable parameter by __VA_ARGS__ */

/** @def LP_ASSERT
 * ASSERT that an expression is true. If it's not emit the breakpoint.
 * @param   assertion  The Expression.
 */
#define LP_ASSERT(assertion) \
    (UNLIKELY(!(assertion)) ? \
        (lpAssertFailure(__FILE__, __LINE__, LP_CURRENT_FUNCTION, #assertion), \
         LP_ASSERT_PANIC()) : \
        (void)0)


/** @def LP_ASSERT_LOG
 * If the expression isn't true, will report the txt-message and emit the breakpoint.
 * @param   assertion  The Expression.
 * @param   msg        Message format,args,... (must be in brackets).
 *                     eg. LP_ASSERT_LOG(expr, ("msg format", arg1, arg2, ...) );
 */
#define LP_ASSERT_LOG(assertion, msg) \
    (UNLIKELY(!(assertion)) ? \
        (lpAssertFailure(__FILE__, __LINE__, LP_CURRENT_FUNCTION, #assertion), \
         lpAssertFailureMsg msg , \
         LP_ASSERT_PANIC()) : \
        (void)0)


/** @def ASSERT_STATIC_INT
 * static_assert emulated.(non standard)
 * This differs from AssertCompile in that it accepts some more expressions
 * than what C++0x allows
 * @param   assertion    The expression.
 */
#define LP_ASSERT_STATIC_INT(assertion)  typedef int known[(assertion) ? 1 : -1]

/** @def ASSERT_STATIC
 * Asserts that a C++0x compile-time expression is true. If it's not break the
 * build.
 * @param   assertion    The Expression.
 */
#ifdef HAVE_STATIC_ASSERT
# define LP_ASSERT_STATIC(assertion) static_assert(!!(assertion), #assertion)
#else
# define LP_ASSERT_STATIC(assertion) LP_ASSERT_STATIC_INT(assertion)
#endif


#else

/*
 dummy
 */
#define LP_ASSERT(assertion) ((void)0)
#define LP_ASSERT_LOG(assertion, msg) ((void)0)
#define LP_ASSERT_STATIC(assertion) ((void)0)

#endif //ENABLE(ASSERTIONS)


////////////////////////////////////////////////////////////////////////////////

/*
 * Errors
 */

/** Operation was succeeded. */
#define LINF_SUCCEEDED (1)
/** Operation was failed. */
#define LERR_FAILED (0)
/** Failed to allocate the memory. */
#define LERR_ALLOC_MEMORY (-1)
/** The stream has been opened */
#define LERR_STREAM_HAS_BEEN_OPENED (-2)
/** Invalid lexicon. */
#define LERR_INVALID_LEX (-3)
/** Report a error */
#define LERR_THROW_ERROR (-4)
/** Syntax error */
#define LERR_SYNTAX_ERROR (-5)
/** The symbol was not found */
#define LERR_SYMBOL_NOT_FOUND (-6)
/** Target is not matched */
#define LERR_NOT_MATCHED (-7)
/** Stack overflows */
#define LERR_STACK_OVERFLOWS (-8)

#define LP_SUCCESS(rc) (rc>0)
#define LP_FAILURE(rc) (rc<1)

#define UPDATE_RC(rc) do {if LP_FAILURE(rc) return rc;} while(0)

#define AssertRC(rc) do {LP_ASSERT(LP_SUCCESS(rc)); } while(0)


////////////////////////////////////////////////////////////////////////////////

/*
 * Logging
 */

enum LogLevel {
  INFO = 0,
  WARNING,
  ERROR,
  VERBOSE,
  DEBUG0,
  DEBUG1,
  DEBUG2,
};


#if ENABLE(LOG)
# define logLevelThreshold (DEBUG2)
#else
# define logLevelThreshold (ERROR)
#endif

class logstream{
public:
  logstream(LogLevel level)
  {
    m_level = level;
  }

 template <class T>
  std::ostream & operator<<(T value) const
  {
    if (m_level <= logLevelThreshold)
      {
#if 0
        return std::cout.operator <<(value);
#endif
        return std::cout << value;
      }
    return std::cout;
  }

 ~logstream()
 {
   std::cout.flush(); /* flush the stream */
 }

private:
  LogLevel m_level;
};

# define LOG(level) logstream(level)


////////////////////////////////////////////////////////////////////////////////

/*
 * Marks
 */
#define __OUT
#define __IN

////////////////////////////////////////////////////////////////////////////////

/*
 * Export
 */
#ifdef IN_SHARED
# ifdef __GNUC__
#  define LP_EXPORT __attribute__((visibility("default")))
# else
#  define LP_EXPORT __declspec(dllexport)
# endif
#else
# define LP_EXPORT
#endif

////////////////////////////////////////////////////////////////////////////////

/*
 * String
 */

#define IsSpace(c) (c =='\t'|| c =='\n'|| c ==' ')
#define IsDigit(c) ((c) >= '0' && (c) <= '9')

////////////////////////////////////////////////////////////////////////////////


/*
 * Config
 */
#define MAX_LEX_LEN 80





////////////////////////////////////////////////////////////////////////////////

/*
 * Stream
 */

/**
 * File stream error code
 */
enum StreamError
{
  STREAM_ERR_INVALID = 0,
  STREAM_ERR_NO,
  STREAM_ERR_OPEN,
  STREAM_ERR_CLOSE,
  STREAM_ERR_READ,
  STREAM_ERR_WRITE,
  STREAM_ERR_SEEK,
  STREAM_ERR_TELL,
  STREAM_ERR_GETSIZE,
  STREAM_ERR_FLUSH
};

/**
 File stream Seek mode
 */
enum StreamSeekMode
{
  STREAM_SEEK_SET, /**< Seek from the beginning of the file */
  STREAM_SEEK_CUR, /**< Seek from current position */
  STREAM_SEEK_END  /**< Seek from the end of the file */
};

/**
 file offset internal
 */
typedef unsigned long long file_off;

#define STREAM_EOF (-1)

/***************************************************
  *****            IStream object              *****
  ***************************************************/

class IStream {
public:
  virtual ~IStream() {}

  virtual int Open(const char *filename, const char *mode) =0;
  virtual int Close() =0;

  /* in/out */
  /* ................................................ */
  virtual file_off Read(void *buffer, file_off size, file_off count) =0;
  virtual file_off Write(const void *buffer, file_off size, file_off count) =0;
  virtual char Getchar() =0;
  virtual int UnGetchar(char c) =0;
  virtual char Peek() =0;
  virtual int Seek(file_off pos, StreamSeekMode mode) =0;
  virtual file_off Tell() =0;
  virtual file_off GetSize() =0;
  virtual int Flush() =0;
};

/***************************************************
  *****             Stream object              *****
  ***************************************************/

LP_EXPORT class Stream {
public:
  static IStream *CreateStream();
};


////////////////////////////////////////////////////////////////////////////////

/*
 * String
 */
#define _MAX_BUFF_SIZE (16) /* power of 2 */

/***************************************************
  *****          StringPool object             *****
  ***************************************************/

LP_EXPORT class StringPool {
public:
  StringPool();
  ~StringPool();

  int append(const char *src, size_t len);
  int append(const char *src);
  int copy(const char *src, size_t len);
  int copy(const char *src);
  int copy(const StringPool &src);
  int compare(const char *src);
  int compare(const StringPool &src);

  size_t length();
  char *buffer() const;
  
  inline char & operator [](const size_t &i)
  {
    LP_ASSERT(i >=0 && i < len);
    return buffer()[i];
  }

  inline StringPool &operator ++()
  {
    LP_ASSERT((curpos + 1) < len);
    curpos++;
    return *this;
  }

  inline StringPool &operator --()
  {
    LP_ASSERT((curpos + 1) < len);
    curpos--;
    return *this;
  }

private:
  int resizeBuffer(size_t newsize);
  int reallocBuffer(size_t size);
private:
  char buff[_MAX_BUFF_SIZE];
  char *heap_buff;
  size_t len;
  size_t buffsize;
  bool inheap;
  size_t curpos;
};

int parserNumberStr(const char *src, __OUT double *out);

/*
 * Lexicon type
 */
enum LexType {
  LEX_INVALID = 0,
  LEX_OPEN_PAREN,
  LEX_CLOSE_PAREN,
  LEX_STRING,
  LEX_MISC
};

/*
 * Lexicon node.
 */
class LexNode {
public:
  LexType      m_type;
  StringPool   m_word;
  file_off     m_line;
  LexNode     *m_prev;
  LexNode     *m_next;
};

/***************************************************
  *****             Lexer object               *****
  ***************************************************/

LP_EXPORT class Lexer {
public:
  Lexer();
  int lex(IStream *stream);
  void dumplist();

  /**
   * Get the root of list.
   * @return pointer to the root node.
   */
  inline LexNode *
  getListRoot()
  {
    return lexlist; // "reverse" the list.
  }

  /**
   * Get the tail of list.
   * @return pointer to the tail node.
   */
  inline LexNode *
  getListTail()
  {
    return lexlistTail; // "reverse" the list.
  }

private:
  inline void skipComment(char *c);
  static int createLexNode(LexType type, const char *word, file_off line, __OUT LexNode **out);
  void insertLexNode(LexNode *node);
  int lexMisc(LexNode *lex, char c);

private:
  IStream *stream;
  LexNode *lexlist;
  LexNode *lexlistTail;
  file_off currentLine;
};

/*
 * Types of object
 */
enum objType
{
  OBJTYPE_INVALID = 0,
  OBJTYPE_BOOLEAN,
  OBJTYPE_NUMBER,
  OBJTYPE_CHARACTER,
  OBJTYPE_STRING,
  OBJTYPE_SYMBOL,
  OBJTYPE_PAIR,
  OBJTYPE_FUNC
};

/*
 * Object data
 */
struct objData {
  objType type;
  union {
    struct {
      bool v;
    } OBJTYPE_BOOLEAN;

    struct {
      double v;
    } OBJTYPE_NUMBER;

    struct {
      char v;
    } OBJTYPE_CHARACTER;

    struct {
      StringPool *v;
    } OBJTYPE_STRING;

    struct {
      struct SynNode *leaf;
      struct SynNode *next;
    } OBJTYPE_PAIR;

    struct {
      StringPool *v;
    } OBJTYPE_SYMBOL;

    struct {
      struct SynNode *params;
      struct SynNode *body;
      int envsp;
    } OBJTYPE_FUNC;
  } u;
};

/**
 * NOTE: the following is extremely DANGEROUS for the performance.
 * Ensure that we have disabled the assertions when compiling a release version.
 */
#if ENABLE(ASSERTIONS)
# define OBJ_VALUE(t, s) (LP_ASSERT(t == s->object.type), (s->object.u.t).v )
# define OBJ_LEAF(s) (LP_ASSERT(OBJTYPE_PAIR == s->object.type), (s->object.u.OBJTYPE_PAIR.leaf) )
# define OBJ_NEXT(s) (LP_ASSERT(OBJTYPE_PAIR == s->object.type), (s->object.u.OBJTYPE_PAIR.next) )
#else
# define OBJ_VALUE(t, s) (s->object.u.t).v
# define OBJ_LEAF(s) (s->object.u.OBJTYPE_PAIR.leaf)
# define OBJ_NEXT(s) (s->object.u.OBJTYPE_PAIR.next)
#endif
#define OBJ_LEAF2(s) OBJ_LEAF(OBJ_LEAF(s))
#define OBJ_LEAF3(s) OBJ_LEAF2(OBJ_LEAF(s))
#define OBJ_NEXT2(s) OBJ_NEXT(OBJ_NEXT(s))
#define OBJ_NEXT3(s) OBJ_NEXT2(OBJ_NEXT(s))


/*
 * Syntax node
 */
struct SynNode
{
  objData object;
  file_off line;
};

class Lisp;

/*
 * Environment stack index.
 */
typedef int EnvSP;


/*
 * Inner, create a atom-type syntax node.
 * @param gc GC object reference.
 * @param t Type of data object.
 * @param out Where to store the pointer of node.
 * @parem v Value want to set.
 * @param l The number of line.
 * @param Where to store the status code.
 */
#define createAtom(gc, t, out, v, l, rcref) \
  do \
    { \
      rcref = gc._createAtom(&out); \
      if (LP_SUCCESS(rcref)) \
        { \
          out->object.type = t; \
          out->line = l; \
          OBJ_VALUE(t, out) = v; \
        } \
    } \
  while(0)

/***************************************************
  *****      Garbage Collection object         *****
  ***************************************************/
LP_EXPORT class GC {
public:
  GC();

  int createSynNode(SynNode *leaf, SynNode *next, __OUT SynNode **out);
  int createPair(SynNode *leaf, SynNode *next, file_off line, __OUT SynNode **out);
  int createFunc(SynNode *params, SynNode *body, EnvSP sp, file_off line, __OUT SynNode **out);
  int _createAtom(__OUT SynNode **out);
};


/***************************************************
  *****             Parser object              *****
  ***************************************************/
LP_EXPORT class Parser {
public:
  Parser(GC *gc);
  int parse(LexNode *root, LexNode *tail);

  void dumpast();

  /*
   * Get the root of AST (abstract syntax tree)
   * @return pointer to the target.
   */
  inline SynNode *getSynRoot()
  {
    return m_ast;
  }

private:
  SynNode *generate(LexNode *& lexnode, __OUT int &rc);
  SynNode *generateList(LexNode *& lexnode, __OUT int &rc);
  SynNode *generateNumber(LexNode *& lexnode, __OUT int &rc);
  SynNode *generateString(LexNode *& lexnode, __OUT int &rc);
  SynNode *generateBoolean(LexNode *& lexnode, __OUT int &rc);
  SynNode *generateCharacter(LexNode *& lexnode, __OUT int &rc);
  SynNode *generateSymbol(LexNode *& lexnode, __OUT int &rc);

  /* inner */
  inline GC & gc()
  {
    return *m_gc;
  }

private:
  GC      *m_gc;
  LexNode *m_lexlist;
  LexNode *m_lexlistTail;
  SynNode *m_ast;
};

#define _MAX_STACK_DEEPTH (2048)

/***************************************************
  *****     Environment Stack object           *****
  ***************************************************/

LP_EXPORT class EnvStack {
public:
  EnvStack(GC *gc);

  int newenv();

  int push(SynNode *vars, SynNode *vals, EnvSP sp, __OUT EnvSP *out);
  void pop();

  int lookupVariableList(EnvSP sp, SynNode *node, __OUT SynNode **out);
  int lookupVariable(EnvSP sp, SynNode *node, __OUT SynNode **out);
  int defineVariable(EnvSP sp, SynNode *node, SynNode *val);
  int setVariable(EnvSP sp, SynNode *node, SynNode *val);
  void releaseVarNode(SynNode *node);

  /**
   * Get the root node of environment in the STACK.
   * @return pointer to the target.
   */
  inline SynNode* node(EnvSP sp)
  {
    return m_stack[sp];
  }

  /* unused */
  inline SynNode* curnode()
  {
    return m_stack[m_sp];
  }

private:
  GC      *m_gc;
  SynNode *m_vars;
  SynNode *m_stack[_MAX_STACK_DEEPTH];
  EnvSP    m_sp;

  /* inner */
  inline GC &gc()
  {
    return *m_gc;
  }
};

/**
 * callback. output the atom data to terminal.
 * @param node Pointer to the target node.
 * @param ln Whether print a new line.
 * @return status code.
 */
typedef int (*pfnPrintAtom)(SynNode *node, bool ln);

#define _MAX_TOKEN_NUM 32

struct Token
{
  const char *symbol;
  SynNode * (Lisp::*eval)(SynNode *leaf, EnvSP envsp, __OUT int &rc);
};


/***************************************************
  *****             Lisp object                *****
  ***************************************************/

LP_EXPORT class Lisp {
public:
  Lisp();

  int parser(IStream *stream);
  int run(__OUT SynNode **out);
  void setPrintAtomCallback(pfnPrintAtom pfn);

  static int throwError(file_off line, file_off pos, const char *msg, ...);
  /*
   * Get the reference of envstack instance.
   * @return envstack.
   */
  inline EnvStack & envstack()
  {
    return m_envstack;
  }

  /*
   * Get the pointer of gc instance.
   * @return gc.
   */
  inline GC &gc()
  {
    return m_gc;
  }

private:
  SynNode* dispatchEvaling(SynNode *root, EnvSP envsp, __OUT int &rc);
  SynNode* eval(SynNode *node, EnvSP envsp, __OUT int &rc);
  SynNode* evalVariable(SynNode *leaf, EnvSP envsp, __OUT int &rc);
  SynNode* evalCall(SynNode *leaf, EnvSP envsp, __OUT int &rc);
  SynNode* evalList(SynNode *vars, SynNode *vals, EnvSP envsp, __OUT int &rc);
  SynNode* evalProcedure(SynNode *body, SynNode *vars, SynNode *vals, EnvSP envsp, __OUT int &rc);

  bool targetSymbol(SynNode *leaf);
  bool targetCall(SynNode *leaf);
  bool targetPair(SynNode *leaf);
  bool targetEval(SynNode *leaf);

  int validateSyntax(SynNode *leaf, int paramCount, const char *name);

  SynNode* symbolSet(SynNode *leaf, EnvSP envsp, __OUT int &rc);
  SynNode* symbolSetCar(SynNode *leaf, EnvSP envsp, __OUT int &rc);
  SynNode* symbolSetCdr(SynNode *leaf, EnvSP envsp, __OUT int &rc);
  SynNode* symbolDefine(SynNode *leaf, EnvSP envsp, __OUT int &rc);
  SynNode* symbolLambda(SynNode *leaf, EnvSP envsp, __OUT int &rc);
  SynNode* symbolIf(SynNode *leaf, EnvSP envsp, __OUT int &rc);
  SynNode* symbolBegin(SynNode *leaf, EnvSP envsp, __OUT int &rc);
  SynNode* symbolCond(SynNode *leaf, EnvSP envsp, __OUT int &rc);
  SynNode* symbolCons(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolCar(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolCdr(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolQuote(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolDisplay(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolPrint(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolEval(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolAppend(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolBooleanP(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolNumberP(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolCharP(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolStringP(SynNode *args, EnvSP envsp, __OUT int &rc);

  SynNode* symbolAdd(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolSub(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolMul(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolDiv(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolEqual(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolGreater(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolLess(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolEGreater(SynNode *args, EnvSP envsp, __OUT int &rc);
  SynNode* symbolELess(SynNode *args, EnvSP envsp, __OUT int &rc);

private:
  SynNode* displayInner(SynNode *args, EnvSP envsp, bool ln, __OUT int &rc);
  SynNode* cmpInner(SynNode *args, EnvSP envsp, int op, __OUT int &rc);
  SynNode* predTypeInner(SynNode *args, EnvSP envsp, objType type, __OUT int &rc);

private:
  IStream     *m_stream;
  Lexer        m_lexer;
  GC           m_gc;
  Parser       m_parser;
  EnvStack     m_envstack;
  bool         m_parsed;
  SynNode     *m_ast;
  static Token  tokens[];
  pfnPrintAtom m_printAtom;
};


} // namespace DSL

#endif //!defined(LISPDSL_H_)
