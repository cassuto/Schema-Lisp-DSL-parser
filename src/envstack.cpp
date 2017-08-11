/** @file
 * LispDSL - environment stack.
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

EnvStack::EnvStack(GC *gc)
  : m_gc(gc),
    m_vars(0),
    m_sp(0)
{
}

/**
 * New environment.
 * @return status code.
 */
int
EnvStack::newenv()
{
  int rc;
  SynNode *frame;

  //!todo delete the old.

  rc = gc().createSynNode(0, 0, &frame);
  if (LP_SUCCESS(rc))
    {
      rc = gc().createSynNode(frame, 0, &m_vars);
      if (LP_SUCCESS(rc))
        {
          /* reset the stack */
          m_sp = 0;
          m_stack[0] = m_vars;
          return rc;
        }
    }
  return rc;
}

/**
 * Push the current environment.
 * @param vars New variables to be joined.
 * @param vals The value of variables.
 * @param sp Indexed the original tree.
 * @param out Where to store the stack index.
 * @return status code.
 */
int
EnvStack::push(SynNode *vars, SynNode *vals, EnvSP sp, __OUT EnvSP *out)
{
  int rc;
  SynNode *new_frame, *new_env;

  if (m_sp + 1 < _MAX_STACK_DEEPTH)
    {
      rc = gc().createPair(vars, vals, 0, &new_frame);
      if (LP_SUCCESS(rc))
        {
          rc = gc().createPair(new_frame, node(sp), 0, &new_env);
          if (LP_SUCCESS(rc))
            {
              *out = m_sp;
              m_stack[m_sp++] = new_env;
              return LINF_SUCCEEDED;
            }
        }
      return rc;
    }
  else
    return LERR_STACK_OVERFLOWS;
}

/**
 * Pop the environment on the stack top.
 * @return status code.
 */
void
EnvStack::pop()
{
  LP_ASSERT(m_sp - 1 >= 0);
  SynNode *target = m_stack[--m_sp];
#if 0
  releaseVarNode(target); //!fixme collect here ?
#else
  UNUSED(target);
#endif
  m_sp--;
}

/**
 * Lookup the variable and out the list.
 * @param sp Stack index.
 * @param src Pointer to the target symbol node.
 * @param out Where to store the result.
 * @return status code.
 */
int
EnvStack::lookupVariableList(EnvSP sp, SynNode *var, __OUT SynNode **out)
{
  if (var->object.type != OBJTYPE_SYMBOL) {
    return Lisp::throwError(var->line, 0, "type mismatch");
  }
  SynNode *n = node(sp);
  SynNode *f;

  while (n)
    {
      /* search in each frame */
      f = OBJ_LEAF(n);
      SynNode *each_var = OBJ_LEAF(f);
      SynNode *each_value = OBJ_NEXT(f);
      while (each_var && each_value)
        {
        if (OBJ_LEAF(each_var) == 0)
        {
          each_var = OBJ_NEXT(each_var);
          each_value = OBJ_NEXT(each_value);
          continue;
        }
        int cmp = OBJ_VALUE(OBJTYPE_SYMBOL, OBJ_LEAF(each_var))->compare(*OBJ_VALUE(OBJTYPE_SYMBOL, var));
        if (cmp == 0)
          {
            *out = each_value;
            return LINF_SUCCEEDED;
          }
        else
          {
            each_var = OBJ_NEXT(each_var);
            each_value = OBJ_NEXT(each_value);
          }
        } // while
      n = OBJ_NEXT(n);
    } // while
  *out = 0;
  return LERR_SYMBOL_NOT_FOUND;
}

/**
 * Lookup the variable.
 * @param sp Stack index.
 * @param src Pointer to the target symbol node.
 * @param out Where to store the result.
 * @return status code.
 */
int
EnvStack::lookupVariable(EnvSP sp, SynNode *src, __OUT SynNode **out)
{
  int rc = lookupVariableList(sp, src, out);
  if (LP_SUCCESS(rc))
    {
      *out = OBJ_LEAF((*out));
    }
  return rc;
}

/**
 * Define a new variable.
 * @param sp Stack index.
 * @param src Pointer to the source vaiable.
 * @param val The value of source variable.
 * @return status code.
 */
int
EnvStack::defineVariable(EnvSP sp, SynNode *src, SynNode *val)
{
  int rc;
  SynNode *n = node(sp);
  SynNode *frame = OBJ_LEAF((n));
  SynNode *leaf;

  /* insert a new variable */
  rc = gc().createSynNode(src, OBJ_LEAF(frame), &leaf);
  if (LP_SUCCESS(rc))
    {
      OBJ_LEAF(frame) = leaf;
      SynNode *next;

      /* insert the value */
      rc = gc().createSynNode(val, OBJ_NEXT(frame), &next);
      if (LP_SUCCESS(rc))
        {
          OBJ_NEXT(frame) = next;
        }
    }
  return rc;
}

/**
 * Set the value of variable
 * @param sp Stack index.
 * @param node Pointer to the target node.
 * @param val Pointer to the source value.
 */
int
EnvStack::setVariable(EnvSP sp, SynNode *node, SynNode *val)
{
  int rc;
  SynNode *var;
  rc = lookupVariableList(sp, node, &var);
  if (LP_SUCCESS(rc))
    {
      releaseVarNode(OBJ_LEAF(var)); /* collection */
      OBJ_LEAF(var) = val;
    }
  return rc;
}

/**
 * Release a SynNode.
 * @param node Pointer to the target node.
 */
void
EnvStack::releaseVarNode(SynNode *node)
{
  LOG(VERBOSE) << "collected:" << node << "\n";
  delete node;
}

} // namespace DSL
