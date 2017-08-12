/** @file
 * LispDSL - built-in primitive functions.
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

////////////////////////////////////////////////////////////////////////////////

/**
 * Validate the syntax
 * @param leaf Pointer to the target node,
 * @param paramCount The number of parameters.
 * @param name the name of target.
 * @retuen status code.
 */
int
Lisp::validateSyntax(SynNode *leaf, int paramCount, const char *name)
{
  /* a burst... */
  if (paramCount == -1)
    return LINF_SUCCEEDED;

  file_off line = leaf->line;
  int count = 0;
  while (leaf)
    {
      count++;
      leaf = OBJ_NEXT(leaf);
    }
  if (paramCount != count)
    {
      Lisp::throwError(line, 0, "'%s' syntax error.", name);
      return LERR_SYNTAX_ERROR;
    }
  return LINF_SUCCEEDED;
}


/**
 * Set the value.
 * (set! [target] [value])
 */
SynNode*
Lisp::symbolSet(SynNode *leaf, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(leaf, 3, "set!");
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  SynNode *var = OBJ_LEAF(OBJ_NEXT(leaf));
  SynNode *val = eval(OBJ_LEAF(OBJ_NEXT2(leaf)), envsp, rc);

  if (LP_SUCCESS(rc))
    {
      if (var->object.type != OBJTYPE_SYMBOL)
        {
          rc = Lisp::throwError(var->line, 0, "set: target variable has invalid format.");
          return 0;
        }
      rc = envstack().setVariable(envsp, var, val);
      if (LP_FAILURE(rc))
        {
          rc = Lisp::throwError(leaf->line, 0, "set: target variable was not found.");
          return 0;
        }
      rc = LINF_SUCCEEDED;
    }
  return 0;
}

/**
 * Set car.
 * (set-car! [target] [value])
 */
SynNode*
Lisp::symbolSetCar(SynNode *leaf, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(leaf, 3, "set-car!");
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  SynNode *list = eval(OBJ_LEAF(OBJ_NEXT(leaf)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  SynNode *val = eval(OBJ_LEAF(OBJ_NEXT2(leaf)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  if (OBJTYPE_PAIR != list->object.type)
    {
      rc = Lisp::throwError(leaf->line, 0, "set-car! - expected a pair.");
      return 0;
    }
  OBJ_LEAF(list) = val; //set

  SynNode *res;
  createAtom(gc(), OBJTYPE_BOOLEAN, res, true, leaf->line, rc);
  return res;
}

/**
 * Set cdr.
 * (set-cdr! [target] [value])
 */
SynNode*
Lisp::symbolSetCdr(SynNode *leaf, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(leaf, 3, "set-cdr!");
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  SynNode *list = eval(OBJ_LEAF(OBJ_NEXT(leaf)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  SynNode *val = eval(OBJ_LEAF(OBJ_NEXT2(leaf)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  if (OBJTYPE_PAIR != list->object.type)
    {
      rc = Lisp::throwError(leaf->line, 0, "set-car! - expected a pair.");
      return 0;
    }
  OBJ_NEXT(list) = val; //set

  SynNode *res;
  createAtom(gc(), OBJTYPE_BOOLEAN, res, true, leaf->line, rc);
  return res;
}

/**
 * Define a new variable.
 */
SynNode*
Lisp::symbolDefine(SynNode *leaf, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(leaf, 3, "define");
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  SynNode *var = OBJ_LEAF(OBJ_NEXT(leaf));
  SynNode *val = eval(OBJ_LEAF(OBJ_NEXT2(leaf)), envsp, rc);

  if (LP_SUCCESS(rc))
    {
      if (var->object.type != OBJTYPE_SYMBOL)
        {
          rc = Lisp::throwError(var->line, 0, "target variable has invalid type.");
          return 0;
        }
      rc = envstack().defineVariable(envsp, var, val);
    }
  return 0;
}

/**
 * Define a lambda.
 */
SynNode*
Lisp::symbolLambda(SynNode *leaf, EnvSP envsp, __OUT int &rc)
{
  SynNode *params = OBJ_LEAF(OBJ_NEXT(leaf));
  SynNode *body = OBJ_NEXT2(leaf);
  SynNode *n;
  rc = gc().createFunc(params, body, envsp, leaf->line, &n);
  if (LP_SUCCESS(rc))
    {
      return n;
    }
  return 0;
}

/**
 * If branch.
 */
SynNode*
Lisp::symbolIf(SynNode *leaf, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(leaf, 4, "if");
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  SynNode *prev = eval(OBJ_LEAF(OBJ_NEXT(leaf)), envsp, rc);
  if (LP_SUCCESS(rc))
    {
      if (OBJTYPE_BOOLEAN != prev->object.type)
        {
          rc = Lisp::throwError(leaf->line, 0, "'if' expected a boolean expression.");
          return 0;
        }
      if (OBJ_VALUE(OBJTYPE_BOOLEAN, prev))
        {
          return eval(OBJ_LEAF(OBJ_NEXT2(leaf)), envsp, rc); /* true */
        }
      else
        {
          return eval(OBJ_LEAF(OBJ_NEXT3(leaf)), envsp, rc); /* false */
        }
      //!todo collection
    }
  return 0;
}

/*
 * Begin.
 */
SynNode *
Lisp::symbolBegin(SynNode *leaf, EnvSP envsp, __OUT int &rc)
{
  return dispatchEvaling(OBJ_NEXT(leaf), envsp, rc);
  //!todo collection
}

SynNode *
Lisp::symbolCond(SynNode *leaf, EnvSP envsp, __OUT int &rc)
{
  SynNode *body = OBJ_NEXT(leaf);
  SynNode *cur;
  SynNode *res;

  while (body)
    {
      cur = OBJ_LEAF(body);
      SynNode *test = OBJ_LEAF(cur);
      if (OBJTYPE_SYMBOL == test->object.type)
        {
          /* expect else */
          int cmp = OBJ_VALUE(OBJTYPE_SYMBOL, test)->compare("else");
          if (cmp != 0)
            {
              rc = Lisp::throwError(cur->line, 0, "expected 'else'.");
              return 0;
            }
          else
            {
              res = dispatchEvaling(OBJ_NEXT(cur), envsp, rc);
              //!todo collection
              return res;
            }
        }
      else
        {
          SynNode *test_ret = eval(test, envsp, rc);
          if (LP_SUCCESS(rc))
            {
              if (OBJTYPE_BOOLEAN != test_ret->object.type)
                {
                  rc = Lisp::throwError(test_ret->line, 0, "expected a boolean expression.");
                  return 0;
                }
              else
                {
                  if (OBJ_VALUE(OBJTYPE_BOOLEAN, test_ret)) {
                    /* true */
                    res = dispatchEvaling(OBJ_NEXT(cur), envsp, rc);
                    //!todo collection.
                    return res;

                  } else {
                    /* false */
                    body = OBJ_NEXT(body);
                  }
                }
            }
        }
  }
  return 0;
}

/*
 * Add + operation.
 * (+ [operand1] [operand2] ... [operandN])
 */
SynNode *
Lisp::symbolAdd(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  file_off line = args->line;
  args = OBJ_NEXT(args);

  double sum = 0;
  while (args)
    {
      SynNode *each = OBJ_LEAF(args);
      /*
       * evaluate each operands
       */
      SynNode *tmp = eval(each, envsp, rc);
      if (OBJTYPE_NUMBER == tmp->object.type)
        {
          sum += OBJ_VALUE(OBJTYPE_NUMBER, tmp);
        }
      else
        {
          rc = Lisp::throwError(each->line, 0, "add - operand(s) type mismatched.");
          return 0;
        }
      args = OBJ_NEXT(args);
    }

  //!todo collection
  SynNode *res;
  createAtom(gc(), OBJTYPE_NUMBER, res, sum, line, rc);
  if (LP_SUCCESS(rc))
    {
      return res;
    }
  return 0;
}

/*
 * Sub - operation.
 * (- [operand1] [operand2])
 */
SynNode *
Lisp::symbolSub(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(args, 3, "-");
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  file_off line = args->line;

  /*
   * evaluating operands
   */
  SynNode *first = eval(OBJ_LEAF(OBJ_NEXT(args)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  SynNode *second = eval(OBJ_LEAF(OBJ_NEXT2(args)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  double sub = 0.0;
  if (OBJTYPE_NUMBER != first->object.type)
    {
      rc = Lisp::throwError(first->line, 0, "sub - operand(s) type mismatched.");
      return 0;
    }
  if (OBJTYPE_NUMBER != second->object.type)
    {
      rc = Lisp::throwError(second->line, 0, "sub - operand(s) type mismatched.");
      return 0;
    }
  else
    {
      sub = OBJ_VALUE(OBJTYPE_NUMBER, first) - OBJ_VALUE(OBJTYPE_NUMBER, second);
    }

  //!todo collection
  SynNode *res;
  createAtom(gc(), OBJTYPE_NUMBER, res, sub, line, rc);
  if (LP_SUCCESS(rc))
    {
      return res;
    }
  return 0;
}

/*
 * Mul * operation.
 * (* [operand1] [operand2] ... [operandN])
 */
SynNode *
Lisp::symbolMul(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  file_off line = args->line;
  args = OBJ_NEXT(args);

  double mul = 1.0;
  while (args)
    {
      SynNode *each = OBJ_LEAF(args);
      /*
       * evaluate each operands
       */
      SynNode *tmp = eval(each, envsp, rc);
      if (OBJTYPE_NUMBER == tmp->object.type)
        {
          mul *= OBJ_VALUE(OBJTYPE_NUMBER, tmp);
        }
      else
        {
          rc = Lisp::throwError(each->line, 0, "mul - operand(s) type mismatched.");
          return 0;
        }
      args = OBJ_NEXT(args);
    }

  //!todo collection
  SynNode *res;
  createAtom(gc(), OBJTYPE_NUMBER, res, mul, line, rc);
  if (LP_SUCCESS(rc))
    {
      return res;
    }
  return 0;
}

/*
 * Div / operation.
 * (/ [operand1] [operand2])
 */
SynNode *
Lisp::symbolDiv(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(args, 3, "/");
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  file_off line = args->line;

  /*
   * evaluating operands
   */
  SynNode *first = eval(OBJ_LEAF(OBJ_NEXT(args)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  SynNode *second = eval(OBJ_LEAF(OBJ_NEXT2(args)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  double divs = 0.0;
  if (OBJTYPE_NUMBER != first->object.type)
    {
      rc = Lisp::throwError(first->line, 0, "sub - operand(s) type mismatched.");
      return 0;
    }
  if (OBJTYPE_NUMBER != second->object.type)
    {
      rc = Lisp::throwError(second->line, 0, "sub - operand(s) type mismatched.");
      return 0;
    }
  else
    {
      divs = OBJ_VALUE(OBJTYPE_NUMBER, first) / OBJ_VALUE(OBJTYPE_NUMBER, second);
    }

  //!todo collection
  SynNode *res;
  createAtom(gc(), OBJTYPE_NUMBER, res, divs, line, rc);
  if (LP_SUCCESS(rc))
    {
      return res;
    }
  return 0;
}

/**
 * Cons
 * (cons [param1] [param2])
 */
SynNode*
Lisp::symbolCons(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(args, 3, "cons");
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  SynNode *first = eval(OBJ_LEAF(OBJ_NEXT(args)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  SynNode *second = eval(OBJ_LEAF(OBJ_NEXT2(args)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }

  SynNode *res;
  rc = gc().createPair(first, second, args->line, &res);
  //!todo collection
  return res;
}

/**
 * CAR
 * (car [list])
 */
SynNode*
Lisp::symbolCar(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(args, 2, "car");
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  SynNode *after = eval(OBJ_LEAF(OBJ_NEXT(args)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  if (OBJTYPE_PAIR != after->object.type)
    {
      rc = Lisp::throwError(args->line, 0, "car - the result is invalid.");
      return 0;
    }
  return OBJ_LEAF(after);
}

/**
 * CDR
 * (cdr [list])
 */
SynNode*
Lisp::symbolCdr(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(args, 2, "cdr");
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  SynNode *after = eval(OBJ_LEAF(OBJ_NEXT(args)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  if (OBJTYPE_PAIR != after->object.type)
    {
      rc = Lisp::throwError(args->line, 0, "cdr - the result is invalid.");
      return 0;
    }
  return OBJ_NEXT(after);
}

/**
 * Quote
 * (quote [list])
 */
SynNode*
Lisp::symbolQuote(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(args, 2, "quote");
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  UNUSED(envsp);
  rc = LINF_SUCCEEDED;
  return OBJ_LEAF(OBJ_NEXT(args));
}

/**
 * Inner, printing.
 */
SynNode*
Lisp::displayInner(SynNode *args, EnvSP envsp, bool ln, __OUT int &rc)
{
  rc = validateSyntax(args, 2, "display");
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  SynNode *node = eval(OBJ_LEAF(OBJ_NEXT(args)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  if (m_printAtom)
    {
      m_printAtom(node, ln);
    }
  SynNode *res;
  createAtom(gc(), OBJTYPE_BOOLEAN, res, true, args->line, rc);
  return res;
}

/**
 * Display
 * (display [list])
 */
SynNode*
Lisp::symbolDisplay(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  return displayInner(args, envsp, true, rc);
}

/**
 * Print without new-line.
 * (print [list])
 */
SynNode*
Lisp::symbolPrint(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  return displayInner(args, envsp, false, rc);
}

/**
 * Eval
 * (eval [list]
 */
SynNode*
Lisp::symbolEval(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(args, 2, "eval");
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  SynNode *res = eval(OBJ_LEAF(OBJ_NEXT(args)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  res = eval(res, envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  return res;
}

/**
 * Append
 * (append [target] [list])
 */
SynNode*
Lisp::symbolAppend(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  rc = validateSyntax(args, 3, "append");
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  SynNode *first = eval(OBJ_LEAF(OBJ_NEXT(args)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  SynNode *second = eval(OBJ_LEAF(OBJ_NEXT(args)), envsp, rc);
  if (LP_FAILURE(rc))
    {
      return 0;
    }
  SynNode *res = first;
  while ((OBJTYPE_PAIR == first->object.type) && OBJ_NEXT(first))
    {
      first = OBJ_NEXT(first);
    }
  if (OBJ_NEXT(first))
    {
      rc = Lisp::throwError(args->line, 0, "append - expected a list.");
      return 0;
    }
  OBJ_NEXT(first) = second;
  return res;
}

/**
 * Inner, point out whether a object is typed the type designated.
 */
SynNode*
Lisp::predTypeInner(SynNode *args, EnvSP envsp, objType type, __OUT int &rc)
{
  /* do the judgement */
  bool b = (type == OBJ_LEAF(OBJ_NEXT(args))->object.type);
  SynNode *res;
  createAtom(gc(), OBJTYPE_BOOLEAN, res, b, args->line, rc);
  return res;
}

/**
 * Resolve whether the target is a boolean
 */
SynNode*
Lisp::symbolBooleanP(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  return predTypeInner(args, envsp, OBJTYPE_BOOLEAN, rc);
}

/**
 * Resolve whether the target is a number
 */
SynNode*
Lisp::symbolNumberP(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  return predTypeInner(args, envsp, OBJTYPE_NUMBER, rc);
}

/**
 * Resolve whether the target is a character.
 */
SynNode*
Lisp::symbolCharP(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  return predTypeInner(args, envsp, OBJTYPE_CHARACTER, rc);
}

/**
 * Resolve whether the target is a string.
 */
SynNode*
Lisp::symbolStringP(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  return predTypeInner(args, envsp, OBJTYPE_STRING, rc);
}


enum CmpOpcode {
  CMPOP_EQUAL = 0,
  CMPOP_GREATER,
  CMPOP_LESS,
  CMPOP_EGREATER,
  CMPOP_ELESS
};

/**
 * Inner, comparing.
 */
SynNode*
Lisp::cmpInner(SynNode *args, EnvSP envsp, int op, __OUT int &rc)
{
  SynNode *first = eval(OBJ_LEAF(OBJ_NEXT(args)), envsp, rc);
  SynNode *second = eval(OBJ_LEAF(OBJ_NEXT2(args)), envsp, rc);

  double rt = 0.0;
  if (OBJTYPE_NUMBER != first->object.type)
    {
      rc = Lisp::throwError(first->line, 0, "equal - type mismatched.");
      return 0;
    }
  if (OBJTYPE_NUMBER != second->object.type)
    {
      rc = Lisp::throwError(second->line, 0, "equal - type mismatched.");
      return 0;
    }
  else
    {
      rt = OBJ_VALUE(OBJTYPE_NUMBER, first) - OBJ_VALUE(OBJTYPE_NUMBER, second);
    }

  bool cmp = false;
  switch (op)
  {
    case CMPOP_EQUAL:
      {
        cmp = (rt == 0);
      }
      break;

    case CMPOP_GREATER:
      {
        cmp = (rt > 0);
      }
      break;

    case CMPOP_LESS:
      {
        cmp = (rt < 0);
      }
      break;

    case CMPOP_EGREATER:
      {
        cmp = (rt >= 0);
      }
      break;

    case CMPOP_ELESS:
      {
        cmp = (rt <= 0);
      }
      break;

    default:
      LP_ASSERT(0);
  }

  SynNode *res;
  createAtom(gc(), OBJTYPE_BOOLEAN, res, cmp, args->line, rc);
  return res;
}

/**
 * Equal predicate.
 * ([operand1] = [operand2])
 */
SynNode*
Lisp::symbolEqual(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  return cmpInner(args, envsp, CMPOP_EQUAL, rc);
}

/**
 * Greater predicate.
 * ([operand1] > [operand2])
 */
SynNode*
Lisp::symbolGreater(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  return cmpInner(args, envsp, CMPOP_GREATER, rc);
}

/**
 * Greater predicate.
 * ([operand1] < [operand2])
 */
SynNode*
Lisp::symbolLess(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  return cmpInner(args, envsp, CMPOP_LESS, rc);
}

/**
 * Equal or Greater predicate.
 * ([operand1] >= [operand2])
 */
SynNode*
Lisp::symbolEGreater(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  return cmpInner(args, envsp, CMPOP_EGREATER, rc);
}

/**
 * Equal or Less predicate.
 * ([operand1] <= [operand2])
 */
SynNode*
Lisp::symbolELess(SynNode *args, EnvSP envsp, __OUT int &rc)
{
  return cmpInner(args, envsp, CMPOP_ELESS, rc);
}


} // namespace DSL
