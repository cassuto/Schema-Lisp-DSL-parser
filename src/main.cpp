/** @file
 * LispDSL - Main entry for console program.
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

////////////////////////////////////////////////////////////////////////////////

using namespace DSL;

/**
 * Inner, output the SynNode.
 * @param node Pointer to the target node.
 * @param ln Whether print a new line.
 * @return status code.
 */
static int
OutputSynNode(SynNode *node, bool ln)
{
  int rc;
  if (!node)
    {
      LOG(INFO) << "nil";
    }
  else
    {
  switch(node->object.type)
  {
    case OBJTYPE_PAIR:
      {
        LOG(INFO) << "( ";
        while (node)
          {
            if (node->object.type == OBJTYPE_PAIR)
              {
                rc = OutputSynNode(OBJ_LEAF(node), false);
                if (LP_FAILURE(rc))
                  {
                    return rc;
                  }
                LOG(INFO) << " ";
              }
            else
              {
                LOG(INFO) << ". ";
                rc = OutputSynNode(node, false);
                if (LP_FAILURE(rc))
                  {
                    return rc;
                  }
                LOG(INFO) << " ";
              }
            node = OBJ_NEXT(node);
          }
        LOG(INFO) << ") ";
      }
      break;
    case OBJTYPE_BOOLEAN:
      {
        if(OBJ_VALUE(OBJTYPE_BOOLEAN, node) == true)
          {
            LOG(INFO) << "#t";
          }
        else
          LOG(INFO) << "#f";
      }
      break;

    case OBJTYPE_NUMBER:
      {
        LOG(INFO) << OBJ_VALUE(OBJTYPE_NUMBER, node);
      }
      break;
    case OBJTYPE_STRING:
      {
        LOG(INFO) << "\"" << OBJ_VALUE(OBJTYPE_STRING, node)->buffer() << "\"";
      }
      break;
    case OBJTYPE_CHARACTER:
      {
        LOG(INFO) << "'" << OBJ_VALUE(OBJTYPE_CHARACTER, node) << "'";
      }
      break;
    case OBJTYPE_SYMBOL:
      {
        LOG(INFO) << "symbol = " << OBJ_VALUE(OBJTYPE_SYMBOL, node)->buffer();
      }
      break;
    case OBJTYPE_FUNC:
      {
        LOG(INFO) << "#func";
      }
      break;

    default:
      LOG(INFO) << "(unknown)";
  } // switch
  } // if
  if (ln)
    {
      LOG(INFO) << "\n"; /* println */
    }
  return LINF_SUCCEEDED;
}

int main(int argc, char *argv[]) {
  int rc = 0;
  IStream *stream = Stream::CreateStream();
  if (stream)
    {
      rc = stream->Open("test.scm", "r");

      if (LP_SUCCESS(rc))
        {
          Lisp *lisp = new Lisp();
          lisp->setPrintAtomCallback(&OutputSynNode);
          rc = lisp->parser(stream);

          if (LP_SUCCESS(rc))
            {
              LOG(INFO) << "launched\n";

              SynNode *res;
              rc = lisp->run(&res);
              if (LP_SUCCESS(rc))
                {
                  OutputSynNode(res, true);
                  return 0;
                }
              else
                LOG(ERROR) << "eval the code.\n";
            }
          else
            LOG(ERROR) << "parser the code.\n";
        }
      else
        LOG(ERROR) << "open the input file\n";
    }
  else
    LOG(ERROR) << "create the stream.\n";

  std::cout << "error" << rc << std::endl;
  LOG(ERROR) << "rc = (" << rc << ")\n";
  while(1);
  return 1;
}
