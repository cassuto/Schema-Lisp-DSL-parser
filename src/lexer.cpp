/** @file
 * LispDSL - Lexical parser.
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
#include <new>
#include "lispdsl.h"

namespace DSL
{

////////////////////////////////////////////////////////////////////////////////

Lexer::Lexer()
    : stream(0),
      lexlist(0),
      lexlistTail(0),
      currentLine(1)
{
}

/**
 * Inner, Skip the node string.
 */
inline void
Lexer::skipComment(char *c)
{
  while (((*c = stream->Getchar()) != STREAM_EOF) && (*c != '\n'));
}

/**
 * Inner, new a LexNode object.
 * @param type Type of the lexicon.
 * @param word Optional, Pointer to the word string.
 * @param line The line number of code.
 * @param out Where to store the result.
 * @return status code
 */
/* static */
int
Lexer::createLexNode(LexType type, const char *word, file_off line, __OUT LexNode **out)
{
  int rc;
  LexNode *n = new (std::nothrow) LexNode();
  if (!n)
    return LERR_ALLOC_MEMORY;

  rc = word ? n->m_word.copy(word) : 1;
  if (LP_SUCCESS(rc))
    {
      n->m_type = type;
      n->m_line = line;
      n->m_prev =
      n->m_next = 0;
      *out = n;
    }
  return rc;
}

/**
 * Inner, insert a lexicon node(at font).
 * @param node Pointer to the target node.
 */
void
Lexer::insertLexNode(LexNode *node)
{
  node->m_next = 0;
  node->m_prev = lexlistTail;

  if (lexlistTail)
    {
      lexlistTail->m_next = node;
    }
  else
    {
      lexlist = node;
    }

  /* link it */
  lexlistTail = node;
}

/**
 * Inner, parser the misc tokens.
 * @param lex Lexical node.
 * @param c Current char.
 */
int
Lexer::lexMisc(LexNode *lex, char c)
{
  int rc = LINF_SUCCEEDED;
  int i = 0;
  bool pair = false;

  char buff[2];
  buff[0] = c;
  buff[1] = 0;
  lex->m_word.append(buff);

  if (c == '"')
      pair = true;
  while ((c = stream->Getchar()) != STREAM_EOF)
    {
      if ( !pair && ( c == ')'
                   || c == '('
                   || c == ';'
                   || IsSpace(c) ) )
        {
          break;
        }
      buff[i++] = c;

      /* handle the overflow */
      if (i == sizeof(buff))
        {
          rc = lex->m_word.append(buff, sizeof(buff));
          if (LP_FAILURE(rc))
            break;
          i = 0;
        }
      /* handle the pair */
      if (c == '"')
          pair = !pair;
    }

  /*
   * Flush the remainder if needed
   */
  if (LP_SUCCESS(rc))
    {
      if (i)
        rc = lex->m_word.append(buff, i/*len*/);
    }

  if (pair)
    {
      rc = Lisp::throwError(lex->m_line, 0, "String '\"' unpaired!\n");
    }

  stream->UnGetchar(c);

  return rc;
}


/**
 * Dump the lexical list.
 */
void
Lexer::dumplist()
{
  for (LexNode *node = lexlist; node; node = node->m_next)
    {
      LOG(VERBOSE)
          << "Lexical NODE:\n"
          << "m_type = (" << node->m_type << ")\n"
          << "m_word = '" << node->m_word.buffer() << "'\n"
          << "m_line = (" << node->m_line << ")\n";
    }
}

/**
 * Parser the lexicon
 * @param stream Pointer to the IStream interface.
 * @return status code.
 */
int
Lexer::lex(IStream *st)
{
  int rc = LINF_SUCCEEDED;
  stream = st;
  currentLine = 1;

  //!todo: delete the original list if needed

  LexNode *lex;

  char c;
  while ((c = stream->Getchar()) != STREAM_EOF)
    {
    /*
     * Skip the spaces and unused blocks
     */
    if (IsSpace(c) && c!= '\n')
      continue;
    if (c == ';')
      skipComment(&c);

    switch (c)
    {
	case '\n':
      currentLine++;
      break;

	case '(':
	  {
	    rc = createLexNode(LEX_OPEN_PAREN, 0, currentLine, &lex);
	    if (LP_SUCCESS(rc))
	      insertLexNode(lex);
	    break;
	  }

	case ')':
	  {
	    rc = createLexNode(LEX_CLOSE_PAREN, 0, currentLine, &lex);
	    if (LP_SUCCESS(rc))
	      insertLexNode(lex);
	    break;
	  }

    default:
      {
        if (c == '"')
          rc = createLexNode(LEX_STRING, 0, currentLine, &lex);
        else
          rc = createLexNode(LEX_MISC, 0, currentLine, &lex);
        if (LP_SUCCESS(rc))
          {
            rc = lexMisc(lex, c);

            if (LP_SUCCESS(rc))
              insertLexNode(lex);
            else
              delete lex;
          }
      }
    } // switch

    if (LP_FAILURE(rc))
      {
        return rc;
      }
  } // while

  return LINF_SUCCEEDED;
}

} // namespace DSL
