/** @file
 * LispDSL - String pool management.
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
#include <string.h>
#include "lispdsl.h"

namespace DSL {

////////////////////////////////////////////////////////////////////////////////

StringPool::StringPool()
  : heap_buff(0),
    len(0),
    buffsize(0),
    inheap(false),
    curpos(0)
{
  memset(buff, 0, sizeof(buff));
}

StringPool::~StringPool() {
  if (heap_buff)
    delete [] heap_buff;
}

/**
 * Join a string to the buffer, this will append a '\0'
 * termination at the end of string as well.
 * @param src Pointer to the source string.
 * @param len Length of source string.
 * @return status code.
 */
int
StringPool::append(const char *src, size_t lensrc) {
  int rc;
  size_t pos = len;
  size_t lenwr = lensrc + 1; // padding '\0'
  size_t lenwrSum = len + lenwr;

  if (lenwrSum <= _MAX_BUFF_SIZE)
    {
      memcpy(&buff[pos], src, lensrc);
      buff[pos + lensrc] = 0;
      len += lensrc;
      rc = LINF_SUCCEEDED;
    }
  else
    { /* using the heap buffer */
      if (lenwrSum > buffsize)
        {
          rc = resizeBuffer(buffsize + lenwr);
          if (LP_SUCCESS(rc))
            {
              /* move the buff data into heap */
              if (!inheap)
                {
                  strcpy(heap_buff, buff);
                  inheap = true;
                }
            }
          else
            return rc;
        }

      /* append the source */
      memcpy(&heap_buff[pos], src, lensrc);
      heap_buff[pos + lensrc] = 0;
      len += lensrc;
      rc = LINF_SUCCEEDED;
    }
  return rc;
}

int
StringPool::append(const char *src)
{
  return append(src, strlen(src));
}

/**
 * Copy the string to the buffer (replace the original).
 * this will append a '\0' termination at the end of string as well.
 * @param src Pointer to the source string.
 * @param len Length of source string.
 * @return status code.
 */
int
StringPool::copy(const char *src, size_t lensrc)
{
  int rc;
  size_t lenwr = lensrc + 1; // padding '\0'

  if (lenwr <= _MAX_BUFF_SIZE)
    {
      memcpy(buff, src, lensrc);
      buff[lensrc] = 0;
      len = lensrc;
      rc = LINF_SUCCEEDED;
    }
  else
    { /* using the heap buffer */
      rc = reallocBuffer(lenwr);
      if (LP_SUCCESS(rc))
        {
          memcpy(heap_buff, src, lensrc);
          heap_buff[lensrc] = 0;
          len = lensrc;
        }
    }
  return rc;
}

int
StringPool::copy(const char *src)
{
  return copy(src, strlen(src));
}

int
StringPool::copy(const StringPool &src)
{
  return copy(src.buffer()); //!fixme
}

/**
 * Compare the string in buffer with the source specified.
 * @param src Pointer to the source string.
 * @return 0 if the two are exactly the same.
 */
int
StringPool::compare(const char *src)
{
  return strcmp(buffer(), src);
}

int
StringPool::compare(const StringPool &src)
{
  return strcmp(buffer(), src.buffer());
}

/**
 * Inner, Resize the heap buffer.
 * @param newsize The new size, NOT increment.
 * @return status code.
 */
int
StringPool::resizeBuffer(size_t newsize)
{
  char *oldbuff = heap_buff;

  /*
   * Calculate the new size of buffer.
   * The function image of the result may be near a
   * para-curve, when the newsize is a constant.
   */
  newsize |= _MAX_BUFF_SIZE - 1;
  if (newsize / 3 < buffsize / 2)
    newsize = buffsize + buffsize / 2; // = 3/2 * buffsize;

  heap_buff = new (std::nothrow) char[newsize];
  if (!heap_buff)
    {
      if (oldbuff)
        delete [] oldbuff;
      return LERR_ALLOC_MEMORY;
    }

  /* copy the original data */
  if (oldbuff)
    {
      strcpy(heap_buff, oldbuff);
      delete [] oldbuff;
    }
  buffsize = newsize;

  return LINF_SUCCEEDED;
}

/**
 * Re allocate the buffer.
 * @param size The size want of new buffer.
 * @return status code.
 */
int
StringPool::reallocBuffer(size_t size)
{
  if (heap_buff)
    delete [] heap_buff;
  heap_buff = new (std::nothrow) char [size];
  if (heap_buff)
    {
      buffsize = size;
      return LINF_SUCCEEDED;
    }
  return LERR_ALLOC_MEMORY;
}

/**
 * Get the current length of string, barring '\0' termination.
 * @retur the value in bytes.
 */
size_t
StringPool::length() {
  return len;
}

/**
 * Get the pointer of buffer.
 * Be careful that the pointer exposed will become
 * @return pointer to the buffer.
 */
char *
StringPool::buffer() const {
  return (len + 1 <= _MAX_BUFF_SIZE ? /*fixme???*/const_cast<char*>(buff) : heap_buff);
}


////////////////////////////////////////////////////////////////////////////////


/*
 * Convert a number-formated string to double.
 * @param src Pointer to the source buffer.
 * @param out Where to store the result.
 * @return status code.
 */
int
parserNumberStr(const char *src, __OUT double *out)
{
  double d = 0.0, power = 1.0;

  /* parse the sign */
  int sign = 1;
  for (; *src; src++)
    {
      if (*src == ' ') continue;
      if (*src == '-') { sign = -1; continue; }
      if (*src == '+') { sign = 1; continue; }
      break;
    }

  for (; *src; src++)
    {
      /* parse the integer part */
      if (IsDigit(*src))
        {
          d = d* 10.0 + (*src - '0');
        }
      /* parse the float part */
      else if (*src == '.')
        {
          for (src++; *src && IsDigit(*src); src++)
            {
              d = d* 10.0 + (*src - '0');
              power *= 10;
            }
          break;
        }
      else
        break;
    }

  if (*src != '\0')
    {
      return LERR_FAILED;
    }

  *out = sign * d/power;
  return LINF_SUCCEEDED;
}


} // namespace DSL
