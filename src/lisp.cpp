/** @file
 * LispDSL - main logic
 */

/*
 *  LispDSL is Copyleft (C) 2016, The 1st Middle School in Yongsheng Lijiang China
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

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include "lispdsl.h"

namespace DSL {

#define DEBUG_LEXER (1)
#define DEBUG_PARSER (1)

////////////////////////////////////////////////////////////////////////////////

Token Lisp::tokens[] =
{
    {"set!", &Lisp::symbolSet},
    {"set-car!", &Lisp::symbolSetCar},
    {"set-cdr!", &Lisp::symbolSetCdr},
    {"define", &Lisp::symbolDefine},
    {"lambda", &Lisp::symbolLambda},
    {"if", &Lisp::symbolIf},
    {"begin", &Lisp::symbolBegin},
    {"cond", &Lisp::symbolCond},
    {"cons", &Lisp::symbolCons},
    {"car", &Lisp::symbolCar},
    {"cdr", &Lisp::symbolCdr},
    {"quote", &Lisp::symbolQuote},
    {"display", &Lisp::symbolDisplay},
    {"print", &Lisp::symbolPrint},
    {"eval", &Lisp::symbolEval},
    {"append", &Lisp::symbolAppend},
    {"boolean?", &Lisp::symbolBooleanP},
    {"number?", &Lisp::symbolNumberP},
    {"char?", &Lisp::symbolCharP},
    {"string?", &Lisp::symbolStringP},
    {"+", &Lisp::symbolAdd},
    {"-", &Lisp::symbolSub},
    {"*", &Lisp::symbolMul},
    {"/", &Lisp::symbolDiv},
    {"=", &Lisp::symbolEqual},
    {">", &Lisp::symbolGreater},
    {"<", &Lisp::symbolLess},
    {">=", &Lisp::symbolEGreater},
    {"<=", &Lisp::symbolELess},
    {0, 0}
};

Lisp::Lisp()
  : m_stream(0),
    m_parser(&m_gc),
    m_envstack(&m_gc),
    m_parsed(false),
    m_ast(0),
    m_printAtom(0)
{
}

/**
 * Report a error.
 * @param line The number of source line.
 * @param pos Position of source line.
 * @return status code.
 */
/* static */
int
Lisp::throwError(file_off line, file_off pos, const char *msg, ...)
{
  //!todo format string
  LOG(ERROR) << "error: line:" << line << ":" << pos
      << " " << msg << "\n";
  return LERR_THROW_ERROR;
}

/**
 * Parser the lisp code from stream.
 * @param stream Pointer to the IStream interface.
 * @return status code.
 */
int
Lisp::parser(IStream *stream)
{
  int rc;
  m_parsed = false;

  /*
   * parser the lexicons
   */
  rc = m_lexer.lex(stream);
  if (LP_SUCCESS(rc))
    {
#if DEBUG_LEXER
      m_lexer.dumplist();
#endif
      /*
       * generate the AST
       */
      rc = m_parser.parse(m_lexer.getListRoot(), m_lexer.getListTail());
      if (LP_SUCCESS(rc))
        {
#if DEBUG_PARSER
          m_parser.dumpast();
#endif
          m_stream = stream;
          m_ast = m_parser.getSynRoot();
          m_parsed = true;

          return rc;
        }
    }
  return rc;
}


/**
 * Inner, evaluating the variable.
 * @param leaf Pointer to the source node.
 * @param envsp Index of local environment stack.
 * @param rc Reference to the status code.
 */
SynNode*
Lisp::evalVariable(SynNode *leaf, EnvSP envsp, __OUT int &rc)
{
  SynNode *var;
  rc = m_envstack.lookupVariable(envsp, leaf, &var);
  if (LP_FAILURE(rc))
    {
      rc = throwError(leaf->line, 0, "variable was not found.");
      return 0;
    }
  return var;
}

/**
 * Inner, evaluating the calling.
 * @param leaf Pointer to the source node.
 * @param envsp Index of local environment stack.
 * @param rc Reference to the status code.
 * @return pointer to the node that stores the result.
 */
SynNode*
Lisp::evalCall(SynNode *leaf, EnvSP envsp, __OUT int &rc)
{
  Token *tk;
  bool matched = false;
  SynNode *sym = OBJ_LEAF(leaf);

  /*
   * Match the symbol name
   */
  if (OBJTYPE_SYMBOL == sym->object.type)
    {
      int cmp;
      for (tk = tokens; tk->symbol; tk++)
        {
          cmp = OBJ_VALUE(OBJTYPE_SYMBOL, sym)->compare(tk->symbol);
          if (cmp == 0)
            {
              matched = true;
              break;
            }
        }
    }

  /*
   * Call the primitive function if matched.
   */
  if (matched)
    {
      /* call the internal function here. */
      SynNode * val = (this->*(tk->eval))(leaf, envsp, rc);
      if (LP_SUCCESS(rc))
        {
          return val;
        }
      return 0; // failed
    }

  SynNode *result;

  /*
   * the node is not a primary function,
   * try to evaluate it as a procedure or lambda.
   */
  if (OBJTYPE_SYMBOL == OBJ_LEAF(leaf)->object.type)
    {
      /* not a lambda */
      SynNode *value;
      rc = m_envstack.lookupVariable(envsp, OBJ_LEAF(leaf), &value);
      if (LP_FAILURE(rc))
        {
          rc = throwError(leaf->line, 0, "target function was not found");
          return 0;
        }
      else
        {
          if (OBJTYPE_FUNC != value->object.type)
            {
              rc = throwError(leaf->line, 0, "invalid calling, target is not a function.");
              return 0;
            }

          SynNode *args = evalList(value->object.u.OBJTYPE_FUNC.params, OBJ_NEXT(leaf), envsp, rc);
          if (LP_SUCCESS(rc))
            {
              /*
               * Call the procedure.
               */
              result = evalProcedure(
                  value->object.u.OBJTYPE_FUNC.body,
                  value->object.u.OBJTYPE_FUNC.params,
                  args,
                  value->object.u.OBJTYPE_FUNC.envsp,
                  rc);
              //!todo collection
              if (LP_SUCCESS(rc))
                {
                  return result;
                }
            }
        }
    }
  else
    {
      /* is lambda */
      if (OBJTYPE_PAIR != OBJ_LEAF(leaf)->object.type)
        {
          rc = throwError(leaf->line, 0, "expected a function.");
          return 0;
        }
      SynNode *lambda = eval(OBJ_LEAF(leaf), envsp, rc);
      if (LP_SUCCESS(rc))
        {
          if (OBJTYPE_FUNC != lambda->object.type)
            {
              rc = throwError(leaf->line, 0, "expected a function.");
              return 0;
            }
          SynNode *args = evalList(lambda->object.u.OBJTYPE_FUNC.params, OBJ_NEXT(leaf), envsp, rc);
          if (LP_SUCCESS(rc))
            {
              /*
               * Call the procedure.
               */
              result = evalProcedure(
                  lambda->object.u.OBJTYPE_FUNC.body,
                  lambda->object.u.OBJTYPE_FUNC.params,
                  args,
                  lambda->object.u.OBJTYPE_FUNC.envsp,
                  rc);
              //!todo colleaction
              if (LP_SUCCESS(rc))
                {
                  return result;
                }
            }
        }
  }
  return 0; // failed
}


/**
 * Inner,Working for evalCall(), evaluating the list.
 * @param var Pointer to the source variable.
 * @param vals Pointer to the values of vars.
 * @param envsp Index of local environment stack.
 * @param rc Reference to the status code.
 * @return pointer to the node that stores the result.
 *          (also a list).
 */
SynNode*
Lisp::evalList(SynNode *vars, SynNode *vals, EnvSP envsp, __OUT int &rc)
{
  SynNode *old_vars = vars;
  if ((vars == NULL && vals != NULL) ||
      (vars != NULL && vals == NULL))
    {
      rc = throwError(vars->line, 0, "invalid number of actual parameters of target function.");
      return 0;
    }

  if (!vars)
    {
      //!fixme
      LP_ASSERT(0);
      return 0;
    }

  SynNode *res;
  SynNode *first;
  res = eval(OBJ_LEAF(vals), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  rc = gc().createPair(res, 0, 0, &first);
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  SynNode *index = first;
  vars = OBJ_NEXT(vars);
  vals = OBJ_NEXT(vals);
  while (vars)
    {
      SynNode *each;
      res = eval(OBJ_LEAF(vals), envsp, rc);
      if (LP_SUCCESS(rc))
        {
          rc = gc().createPair(res, 0, 0, &each);
          if (LP_SUCCESS(rc))
            {
              OBJ_NEXT(index) = each;
              index = each;
              vars = OBJ_NEXT(vars);
              vals = OBJ_NEXT(vals);
            }
        }
      else
        return 0;
    }
  if (vals) {
    rc = throwError(old_vars->line, 0, "invalid number of actual parameters of target function.");
    return 0;
  }
  return first;
}

/**
 * Inner, evaluate the procedure.
 * @param bdoy Pointer to the root node of procedure body.
 * @param var Pointer to the source variable.
 * @param vals Pointer to the values of vars.
 * @param envsp Index of local environment stack.
 * @param rc Reference of status code.
 * @return pointer to the node that stores the result.
 */
SynNode*
Lisp::evalProcedure(SynNode *body, SynNode *vars, SynNode *vals, EnvSP envsp, __OUT int &rc)
{
  /*
   * create a new local environment for the procedure.
   */
  EnvSP newsp;
  rc = m_envstack.push(vars, vals, envsp, &newsp);
  if (LP_SUCCESS(rc))
    {
      SynNode *res = dispatchEvaling(body, newsp, rc);
      if (LP_SUCCESS(rc))
        {
          //!todo garbage collection.
          m_envstack.pop();
          return res;
        }
    }
  return 0;
}


/**
 * Inner, Interpret the leaves in list.
 * @param node Pointer to the target list
 * @param envsp Index of local environment stack.
 * @return status code.
 */
SynNode *
Lisp::eval(SynNode *node, EnvSP envsp, __OUT int &rc)
{
  if (targetEval(node))
    {
      return node;
    }
  else if (targetSymbol(node))
    {
      return evalVariable(node, envsp, rc);
    }
  else if (targetCall(node))
    {
      return evalCall(node, envsp, rc);
    }
  rc = throwError(node->line, 0, "invalid syntax.");
  return 0;
}

/**
 * Inner, dispatch all the operation in the list.
 * @param root Pointer to the root node of SynTree.
 * @param envsp Index of local environment stack.
 * @return Pointer to the node that stores the result.
 */
SynNode *
Lisp::dispatchEvaling(SynNode *root, EnvSP envsp, int &rc)
{
  SynNode *result = 0;
  SynNode *node = root;
  /* execute */
  while (node)
    {
      if (UNLIKELY(node->object.type != OBJTYPE_PAIR))
        {
          /* this never happens */
          LP_ASSERT(0);
          rc = throwError(node->line, 0, "invalid syntax.");
          return 0;
        }

      result = eval(OBJ_LEAF(node), envsp, rc);
      if (LP_SUCCESS(rc))
        {
          node = OBJ_NEXT(node);
        }
      else
        return result; // failed.
    }

  //todo: gc

  return result;
}

/**
 * Inner, resolve whether the target is a symbol.
 * @param leaf Pointer to the node.
 * @return true if yes.
 */
bool
Lisp::targetSymbol(SynNode *leaf)
{
  return (leaf->object.type == OBJTYPE_SYMBOL);
}

/**
 * Inner, resolve whether the target is a calling.
 * @param leaf Pointer to the node.
 * @return true if yes.
 */
bool
Lisp::targetCall(SynNode *leaf)
{
  if (OBJTYPE_PAIR == leaf->object.type)
    {
      if (OBJTYPE_SYMBOL == OBJ_LEAF(leaf)->object.type)
        {
          return true;
        }
    }
  return false;
}

/**
 * Inner, resolve whether the target is pair.
 * @param leaf Pointer to the node.
 * @return true if yes.
 */
bool
Lisp::targetPair(SynNode *leaf)
{
  return (leaf->object.type == OBJTYPE_PAIR);
}

/**
 * Inner, resolve whether the target is a eval expression.
 * @param leaf Pointer to the node.
 * @return true if yes.
 */
bool
Lisp::targetEval(SynNode *leaf)
{
  switch (leaf->object.type)
  {
    case OBJTYPE_BOOLEAN:
    case OBJTYPE_NUMBER:
    case OBJTYPE_STRING:
    case OBJTYPE_CHARACTER:
      return true;
    default:
      return false;
  }
}

/**
 * Evaluate the script
 * @param out Optional, Where to store the result.
 * @reutrn status code.
 */
int
Lisp::run(__OUT SynNode **out)
{
  if (!m_parsed)
    {
      return LERR_FAILED;
    }

  int rc;
  SynNode *result = 0;

  rc = m_envstack.newenv();
  if (LP_SUCCESS(rc))
    {
      result = dispatchEvaling(m_ast, 0/*envsp*/, rc);
      if (LP_SUCCESS(rc))
        {
          if (out)
            {
              *out = result;
            }
        }
    }
  return rc;
}

/**
 * Set the callback for atom data output.
 * @param pfn Pointer to the callback function,
 */
void
Lisp::setPrintAtomCallback(pfnPrintAtom pfn)
{
  m_printAtom = pfn;
}

} // namespace DSL
