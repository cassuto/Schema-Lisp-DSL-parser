/** @file
 * LispDSL - File stream.
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
#include <stdio.h>
#include "lispdsl.h"

namespace DSL
{

////////////////////////////////////////////////////////////////////////////////

class Filestream : public IStream
{
public:
  Filestream() :
      fd (0),
      lasterr (STREAM_ERR_INVALID)
  {
  }

  /**
   * Open a file stream.
   *
   * @returns status code.
   * @param filename  Path name of file to open.
   * @param mode      Open mode. followed the fopen() standard.
   *                  Syntax: <a|r|w>[+][b|t].
   */
  virtual int
  Open(const char *filename, const char *mode)
  {
    if (fd)
    return LERR_STREAM_HAS_BEEN_OPENED;

    fd = fopen(filename, mode);

    if (!fd || ferror(fd))
      lasterr = STREAM_ERR_OPEN;
    if (fd)
      return LINF_SUCCEEDED; else
    return LERR_FAILED; //@todo map the errcode to rc.
  }

  /**
   * Close the opened file.
   * @returns status code
   */

  virtual int
  Close()
  {
    int nerr = 1;
    if (fd)
    nerr = fclose(fd);
    if (nerr)
      {
        fd = NULL;
        lasterr = STREAM_ERR_CLOSE;
        return LERR_FAILED;
      }
    else
      return LINF_SUCCEEDED; //@todo map the errcode to rc.
  }

  /**
   * Read a count of bytes to the buffer specified.
   *
   * @returns The number of bytes read, if success.
   * @returns 0 if failed.
   * @param buffer            Pointer to the target buffer.
   * @param size              The size of elements.
   * @param count             The count of elements.So the valid size
   *                          should be size * count in bytes.
   */
  virtual file_off
  Read(void *buffer, file_off size, file_off count)
  {
    file_off len = 0;
    if (fd)
      {
        //@todo fixme later.
        len = static_cast<file_off>(
            fread(buffer,
                static_cast<size_t>(size),
                static_cast<size_t>(count), fd) );

        if (!len)
          lasterr = STREAM_ERR_READ;
      }
    return len;
  }

  /**
   * Write a count of bytes that is from the buffer specified.
   *
   * @returns The number of bytes written, if success.
   * @returns 0 if failed.
   * @param buffer            Pointer to the source buffer.
   * @param size              The size of elements.
   * @param count             The count of elements.So the valid size
   *                          should be size * count in bytes.
   */
  virtual file_off
  Write(const void *buffer, file_off size, file_off count) {
      file_off len = 0;
      if (fd)
        {
          //@todo fixme later.
          len = static_cast<file_off>(
              fwrite(buffer,
                  static_cast<size_t>(size),
                  static_cast<size_t>(count), fd) );
          if (!len)
            lasterr = STREAM_ERR_WRITE;
      }
      return len;
  }

  /**
   * Get a character from the stream.
   * @return the target char.
   */
  virtual char
  Getchar() {
    int c = getc(fd);
    return static_cast<char>(c);
  }

  /**
   * Fallback the stream by a char.
   * @return status code
   */
  virtual int
  UnGetchar(char c) {
    return (ungetc(static_cast<int>(c), fd) == static_cast<int>(c))
        ? LINF_SUCCEEDED
            : LERR_FAILED;
  }

  /**
   * Get a character from the stream, without changing of
   * current position.
   * @return status code.
   */
  virtual char
  Peek() {
    int c = getc(fd);
    ungetc(c, fd);
    return static_cast<char>(c);
  }

  /**
   * Set reading/writing offset position.
   *
   * @return status code.
   * @param pos    The new position.
   * @param mode   Mode of seeking.
   */
  virtual int
  Seek(file_off pos, StreamSeekMode mode) {
    int rc = LERR_FAILED;
    static const int mapMode[] = {
      SEEK_SET, // STREAM_SEEK_SET
      SEEK_CUR, // STREAM_SEEK_CUR
      SEEK_END  // STREAM_SEEK_END
    };

    if (fd)
      {
        rc = fseek(fd, static_cast<long>(pos), mapMode[mode])<0 ? LERR_FAILED:LINF_SUCCEEDED;
        if (ferror(fd))
          lasterr = STREAM_ERR_SEEK;
      }
    return rc;
  }

  /**
   * Tell the current position.
   * @return the result.
   */
  virtual file_off
  Tell()
  {
    file_off offset = 0;
    if (fd)
      {
        offset = static_cast<file_off>(ftell(fd));
        if (ferror(fd))
          lasterr = STREAM_ERR_TELL;
      }
    return offset;
  }

  /**
   * Get the size of file.
   * @return the result.
   */
  virtual file_off
  GetSize() {
    file_off len = 0;

    if (fd)
      {
        //@todo fixme later.
        size_t lastPos = (size_t)ftell(fd);
        fseek(fd, 0, SEEK_END);
        len = (file_off)ftell(fd);
        fseek(fd, lastPos, SEEK_SET);

        if (ferror(fd))
          lasterr = STREAM_ERR_GETSIZE;
      }
    return len;
  }

  /**
   * Flush the stream.
   * @return status code.
   */
  virtual int
  Flush()
  {
      int rc = LERR_FAILED;
      if (fd)
        {
          rc = fflush(fd)<0 ? LERR_FAILED:LINF_SUCCEEDED;
          if (ferror(fd))
            lasterr = STREAM_ERR_FLUSH;
        }
      return rc;
  }

  /**
   * Get the last error code.
   * @return the result
   */
  virtual StreamError
  GetError() const
  {
    return lasterr;
  }

private:
  FILE *fd;
  StreamError lasterr;
};


////////////////////////////////////////////////////////////////////////////////


/**
 * Get a valid stream interface.
 * @return 0 if failed.
 * @return pointer to the IStream.
 */
IStream *
Stream::CreateStream()
{
  return static_cast<IStream*>(new (std::nothrow) Filestream());
}

} // namespace DSL
