/** @file
 * LispDSL - Garbage collection.
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

GC::GC()
{
}

/**
 * Create a variable node.
 * @param Pointer to the first leaf node.
 * @param next Pointer to the next node.
 * @param out Where to store the new node.
 * @return status code.
 */
int
GC::createSynNode(SynNode *leaf, SynNode *next, __OUT SynNode **out)
{
  SynNode *n;
  *out = n = new (std::nothrow) SynNode;
  if (n)
    {
      n->object.type = OBJTYPE_PAIR;
      n->line = 0;
      OBJ_LEAF(n) = leaf;
      OBJ_NEXT(n) = next;
      return LINF_SUCCEEDED;
    }
  return LERR_ALLOC_MEMORY;
}

/**
 * Inner, new a list-type syntax node.
 * @param Pointer to the first leaf node.
 * @param next Pointer to the next node.
 * @param line The number of current line.
 * @param out Where to store the new node.
 * @return status code.
 */
int
GC::createPair(SynNode *leaf, SynNode *next, file_off line, __OUT SynNode **out)
{
  SynNode *n;
  *out = n = new (std::nothrow) SynNode;
  if (n)
    {
      n->object.type = OBJTYPE_PAIR;
      n->line = line;
      OBJ_LEAF(n) = leaf;
      OBJ_NEXT(n) = next;
      return LINF_SUCCEEDED;
    }
  return LERR_ALLOC_MEMORY;
}


/**
 * Create a function-type synnode.
 * @param params Pointer to the parameters node.
 * @param body Pointer to the body of function.
 * @param sp Stack index of environment.
 * @param line The number of source line.
 * @param out Where to store the result.
 */
int
GC::createFunc(SynNode *params, SynNode *body, EnvSP sp, file_off line, __OUT SynNode **out)
{
  SynNode *n;
  *out = n = new (std::nothrow) SynNode;
  if (n)
    {
      n->object.type = OBJTYPE_FUNC;
      n->line = line;
      n->object.u.OBJTYPE_FUNC.params = params;
      n->object.u.OBJTYPE_FUNC.body = body;
      n->object.u.OBJTYPE_FUNC.envsp = sp;
      return LINF_SUCCEEDED;
    }
  return LERR_ALLOC_MEMORY;
}

/**
 * Working for macro createAtom().
 * @param out Where to store the result.
 * @return status code.
 */
int
GC::_createAtom(__OUT SynNode **out)
{
  *out = new (std::nothrow) SynNode;
  return *out ? LINF_SUCCEEDED : LERR_ALLOC_MEMORY;
}


} // namespace DSL
