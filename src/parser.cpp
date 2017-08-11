/** @file
 * LispDSL - Syntax parser & AST management.
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

#define LEX_NEXT(n) n->m_next
#define LEX_PREV(n) n->m_prev
#define LEX_PREV2(n) LEX_PREV(LEX_PREV(n))
#define LEX_PREV3(n) LEX_PREV(LEX_PREV2(n))

#define LEX_SET_NEXT(n) n = LEX_NEXT(n)
#define LEX_SET_PREV(n) n = LEX_PREV(n)

namespace DSL {

////////////////////////////////////////////////////////////////////////////////

Parser::Parser(GC *gc)
  : m_gc(gc),
    m_lexlist(0),
    m_lexlistTail(0),
    m_ast(0)
{
}

/**
 * Inner, process the list.
 * @param lexnode Reference to the pointer to the lexical node.
 * @param rc Where to store the status code.
 * @return 0 if failed.
 * @return pointer to the new syntax node.
 */
SynNode *
Parser::generateList(LexNode *& lexnode, __OUT int &rc)
{
  if (!(lexnode))
    {
      rc = Lisp::throwError(LEX_PREV(m_lexlistTail)->m_line, 0, "Parentheses do not match.");
      return 0;
    }
  if (lexnode->m_type == LEX_CLOSE_PAREN)
    {
      /* nil list */
      rc = LINF_SUCCEEDED;
      return 0;
    }
  else
    {
      SynNode *n = 0;
      SynNode *a = generate(lexnode, rc);
      if (LP_SUCCESS(rc))
        {
          SynNode *b = generateList(lexnode, rc);
          if (LP_SUCCESS(rc))
            {
              rc = gc().createPair(a, b, LEX_PREV(lexnode)->m_line, &n);
            }
        }
      return n;
    }
  return 0;
}

/**
 * Inner, process the number.
 * @param lexnode Reference to the pointer to the lexical node.
 * @param rc Where to store the status code.
 * @return 0 if failed.
 * @return pointer to the new syntax node.
 */
SynNode *
Parser::generateNumber(LexNode *& lexnode, __OUT int &rc)
{
  SynNode *n;
  double val;
  const char *src = lexnode->m_word.buffer();

  rc = parserNumberStr(src, &val);
  if (LP_SUCCESS(rc))
    {
      createAtom(gc(), OBJTYPE_NUMBER, n, val, lexnode->m_line, rc);
      if (LP_SUCCESS(rc))
        {
          return n;
        }
    }
  return 0;
}

/**
 * Inner, process the string.
 * @param lexnode Reference to the pointer to the lexical node.
 * @param rc Where to store the status code.
 * @return 0 if failed.
 * @return pointer to the new syntax node.
 */
SynNode *
Parser::generateString(LexNode *& lexnode, __OUT int &rc)
{
  size_t length = lexnode->m_word.length();
  const char *word = lexnode->m_word.buffer();

  /* check the lexicon */
  if (word[0] != '"' || word[length-1] != '"')
    {
      rc = Lisp::throwError(lexnode->m_line, 0, "String format mismatch.");
      return 0;
    }

  SynNode *n;
  StringPool *pool = new (std::nothrow) StringPool;
  if (pool)
    {
      rc = pool->copy(word + 1, length -2/*remove '\"' char */);
      if (LP_SUCCESS(rc))
        {
          createAtom(gc(), OBJTYPE_STRING, n, pool, lexnode->m_line, rc);
          if (LP_SUCCESS(rc))
            {
              return n;
            }
        }
    }
  else
    rc = LERR_ALLOC_MEMORY;
  return 0;
}

/**
 * Inner, process the symbol.
 * @param lexnode Reference to the pointer to the lexical node.
 * @param rc Where to store the status code.
 * @return 0 if failed.
 * @return pointer to the new syntax node.
 */
SynNode *
Parser::generateSymbol(LexNode *& lexnode, __OUT int &rc)
{
  SynNode *n;
  StringPool *pool = new (std::nothrow) StringPool;
  if (pool)
    {
      rc = pool->copy(lexnode->m_word);
      if (LP_SUCCESS(rc))
        {
          createAtom(gc(), OBJTYPE_SYMBOL, n, pool, lexnode->m_line, rc);
          if (LP_SUCCESS(rc))
            {
              return n;
            }
        }
    }
  else
    rc = LERR_ALLOC_MEMORY;
  return 0;
}

/**
 * Inner, process the boolean.
 * @param lexnode Reference to the pointer to the lexical node.
 * @param rc Where to store the status code.
 * @return 0 if failed.
 * @return pointer to the new syntax node.
 */
SynNode *
Parser::generateBoolean(LexNode *& lexnode, __OUT int &rc)
{
  SynNode *n = 0;
  const char *word = lexnode->m_word.buffer();
  if (word[0] != '#') {
    rc = Lisp::throwError(lexnode->m_line, 0, "Not a boolean value.");
    return 0;
  }
  if (word[1] == 't' || word[1] == 'T')
    {
      createAtom(gc(), OBJTYPE_BOOLEAN, n, true, lexnode->m_line, rc);
    }
  else if (word[1] == 'f' || word[1] == 'F')
    {
      createAtom(gc(), OBJTYPE_BOOLEAN, n, false, lexnode->m_line, rc);
    }
  else
    {
      rc = Lisp::throwError(lexnode->m_line, 0, "Not a boolean value.");
    }
  return LP_SUCCESS(rc) ? n : 0;
}

/**
 * Inner, process the character.
 * @param lexnode Reference to the pointer to the lexical node.
 * @param rc Where to store the status code.
 * @return 0 if failed.
 * @return pointer to the new syntax node.
 */
SynNode *
Parser::generateCharacter(LexNode *& lexnode, __OUT int &rc)
{
  const char *word = lexnode->m_word.buffer();
  if (word[0] != '\'' || word[2] != '\'' || lexnode->m_word.length() != 3)
    {
      rc = Lisp::throwError(lexnode->m_line, 0, "Invalid syntax of character.");
      return 0;
    }
  SynNode *n;
  createAtom(gc(), OBJTYPE_CHARACTER, n, word[1], lexnode->m_line, rc);
  if (LP_SUCCESS(rc))
    {
      return n;
    }
  return 0;
}

/**
 * Inner, generate the ast.
 * @param lexnode Pointer to the current lexical node.
 * @param nextnode Where to store the next node.
 * @return status code.
 */
SynNode *
Parser::generate(LexNode *& lexnode, __OUT int &rc)
{
  SynNode *node = 0;
  switch (lexnode->m_type)
  {
    case LEX_OPEN_PAREN:
      {
        LEX_SET_NEXT(lexnode);
        node = generateList(lexnode, rc);
        break;
      }
    case LEX_STRING:
      {
        node = generateString(lexnode, rc);
        break;
      }
    case LEX_MISC:
      {
        switch (lexnode->m_word.buffer()[0])
        {
          case '#':
            {
              node = generateBoolean(lexnode, rc);
              break;
            }
          case '\'':
            {
              node = generateCharacter(lexnode, rc);
              break;
            }
          default:
            {
              const char *word = lexnode->m_word.buffer();

              if (((word[0] == '.') || word[0] == '+' || word[0] == '-')
                  && (IsDigit(word[1]) || word[1] == '.') )
                {
                  /* signed number */
                  node = generateNumber(lexnode, rc);
                  break;
                }
              else if (IsDigit(word[0]))
                {
                  /* number */
                  node = generateNumber(lexnode, rc);
                  break;
                }
              else
                {
                  /* symbol */
                  node = generateSymbol(lexnode, rc);
                  break;
                }
            }
        }
        break;
      }

    default:
      LOG(ERROR) << "invalid lexicon: type = (" << lexnode->m_type << ")\n";
      rc = LERR_INVALID_LEX;
      return 0;
    }

  if (LP_SUCCESS(rc))
    {
      LEX_SET_NEXT(lexnode);
    }

  return node;
}


static void
dumpNode(SynNode *node, int nest)
{
  objData *dat = &node->object;

  /* trunk leading */
  for (int i = 0; i < nest; i++)
    LOG(VERBOSE) << " ";
  LOG(VERBOSE) << "|-";

  LOG(VERBOSE) << "(" << nest << ")NODE: type = " << dat->type << "\n";

  while(node)
    {
      /* leaf leading */
      for (int i = 0; i < nest; i++)
          LOG(VERBOSE) << " ";
        LOG(VERBOSE) << "|l";

      dat = &node->object;

      switch(node->object.type)
        {
          case OBJTYPE_PAIR:
            {
              LOG(VERBOSE) << "\n";
              if (OBJ_LEAF(node))
                {
                  dumpNode(OBJ_LEAF(node), nest + 1);
                }
              if (OBJ_NEXT(node))
                {
                  node = OBJ_NEXT(node);
                  continue;
                }
            }
            break;

          case OBJTYPE_NUMBER:
            {
              LOG(VERBOSE) << "number = " << dat->u.OBJTYPE_NUMBER.v;
            }
            break;
          case OBJTYPE_STRING:
            {
              LOG(VERBOSE) << "string = \"" << dat->u.OBJTYPE_STRING.v->buffer() << "\"";
            }
            break;
          case OBJTYPE_BOOLEAN:
            {
              LOG(VERBOSE) << "boolean = " << (dat->u.OBJTYPE_BOOLEAN.v ? "true" : "false");
            }
            break;
          case OBJTYPE_CHARACTER:
            {
              LOG(VERBOSE) << "character = " << dat->u.OBJTYPE_CHARACTER.v;
            }
            break;
          case OBJTYPE_SYMBOL:
            {
              LOG(VERBOSE) << "symbol = " << dat->u.OBJTYPE_SYMBOL.v->buffer();
            }
            break;

          default:
            LOG(VERBOSE) << "(unknown)";
        }

      LOG(VERBOSE) << "\n";
      break;
    }
}

/**
 * Dump the AST.
 */
void
Parser::dumpast()
{
  dumpNode(m_ast, 0);
}

/**
 * Parser the lexical list and generate the AST.
 * @param root Pointer the root of lexical list.
 * @param tail Pointer to the tail of lexical list.
 * @return status code.
 */
int
Parser::parse(LexNode *root, LexNode *tail)
{
  int rc;
  SynNode *node;

  m_lexlist = root;
  m_lexlistTail = tail;
  //!todo: delete the original list if needed

  node = generate(root, rc);
  if (LP_SUCCESS(rc))
    {
      m_ast = node;
    }
  return rc;
}

} // namespace DSL
