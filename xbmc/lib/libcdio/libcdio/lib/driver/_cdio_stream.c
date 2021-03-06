/*
    $Id: _cdio_stream.c,v 1.8 2006/04/15 03:05:14 rocky Exp $

    Copyright (C) 2000, 2004, 2005 Herbert Valerio Riedel <hvr@gnu.org>
    Copyright (C) 2005, 2006 Rocky Bernstein <rocky@cpan.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "cdio_assert.h"

/* #define STREAM_DEBUG  */

#include <cdio/logging.h>
#include <cdio/util.h>
#include "_cdio_stream.h"

static const char _rcsid[] = "$Id: _cdio_stream.c,v 1.8 2006/04/15 03:05:14 rocky Exp $";

/* 
 * DataSource implementations
 */

struct _CdioDataSource {
  void* user_data;
  cdio_stream_io_functions op;
  int is_open;
  long position;
};

void
cdio_stream_close(CdioDataSource_t *p_obj)
{
  if (!p_obj) return;

  if (p_obj->is_open) {
    cdio_debug ("closed source...");
    p_obj->op.close(p_obj->user_data);
    p_obj->is_open  = 0;
    p_obj->position = 0;
  }
}

void
cdio_stream_destroy(CdioDataSource_t *p_obj)
{
  if (!p_obj) return;

  cdio_stream_close(p_obj);

  p_obj->op.free(p_obj->user_data);

  free(p_obj);
}

/**
  Like 3 fgetpos.
  
  This function gets the current file position indicator for the stream
  pointed to by stream.  
  
  @return unpon successful completion, return value is positive, else,
  the global variable errno is set to indicate the error.
*/
ssize_t
cdio_stream_getpos(CdioDataSource_t* p_obj, /*out*/ ssize_t *i_offset)
{
  if (!p_obj || !p_obj->is_open) return DRIVER_OP_UNINIT;
  return *i_offset = p_obj->position;
}

CdioDataSource_t *
cdio_stream_new(void *user_data, const cdio_stream_io_functions *funcs)
{
  CdioDataSource_t *new_obj;

  new_obj = calloc (1, sizeof (CdioDataSource_t));

  new_obj->user_data = user_data;
  memcpy(&(new_obj->op), funcs, sizeof(cdio_stream_io_functions));

  return new_obj;
}

/* 
   Open if not already open. 
   Return false if we hit an error. Errno should be set for that error.
*/
static bool
_cdio_stream_open_if_necessary(CdioDataSource_t *p_obj)
{
  if (!p_obj) return false;

  if (!p_obj->is_open) {
    if (p_obj->op.open(p_obj->user_data)) {
      cdio_warn ("could not open input stream...");
      return false;
    } else {
      cdio_debug ("opened source...");
      p_obj->is_open = 1;
      p_obj->position = 0;
    }
  }
  return true;
}

/**
  Like fread(3) and in fact may be the same.
  
  DESCRIPTION:
  The function fread reads nmemb elements of data, each size bytes long,
  from the stream pointed to by stream, storing them at the location
  given by ptr.
  
  RETURN VALUE:
  return the number of items successfully read or written (i.e.,
  not the number of characters).  If an error occurs, or the
  end-of-file is reached, the return value is a short item count
  (or zero).
  
  We do not distinguish between end-of-file and error, and callers
  must use feof(3) and ferror(3) to determine which occurred.
*/
ssize_t
cdio_stream_read(CdioDataSource_t* p_obj, void *ptr, long size, long nmemb)
{
  long read_bytes;

  if (!p_obj) return 0;
  if (!_cdio_stream_open_if_necessary(p_obj)) return 0;

  read_bytes = (p_obj->op.read)(p_obj->user_data, ptr, size*nmemb);
  p_obj->position += read_bytes;

  return read_bytes;
}

/**
  Like 3 fseek and in fact may be the same.
  
  This  function sets the file position indicator for the stream
  pointed to by stream.  The new position, measured in bytes, is obtained
  by  adding offset bytes to the position specified by whence.  If whence
  is set to SEEK_SET, SEEK_CUR, or SEEK_END, the offset  is  relative  to
  the  start of the file, the current position indicator, or end-of-file,
  respectively.  A successful call to the fseek function clears the end-
  of-file indicator for the stream and undoes any effects of the
  ungetc(3) function on the same stream.
  
  @return unpon successful completion, return value is positive, else,
  the global variable errno is set to indicate the error.
*/
ssize_t
cdio_stream_seek(CdioDataSource_t* p_obj, ssize_t offset, int whence)
{
  if (!p_obj) return DRIVER_OP_UNINIT;

  if (!_cdio_stream_open_if_necessary(p_obj)) 
    /* errno is set by _cdio_stream_open_if necessary. */
    return DRIVER_OP_ERROR;

  if (offset < 0) return DRIVER_OP_ERROR;

  if (p_obj->position != offset) {
#ifdef STREAM_DEBUG
    cdio_warn("had to reposition DataSource from %ld to %ld!", obj->position, offset);
#endif
    p_obj->position = offset;
    return p_obj->op.seek(p_obj->user_data, offset, whence);
  }

  return 0;
}

/**
  Return whatever size of stream reports, I guess unit size is bytes. 
  On error return -1;
 */
ssize_t
cdio_stream_stat(CdioDataSource_t *p_obj)
{
  if (!p_obj) return -1;
  if (!_cdio_stream_open_if_necessary(p_obj)) return -1;

  return p_obj->op.stat(p_obj->user_data);
}


/* 
 * Local variables:
 *  c-file-style: "gnu"
 *  tab-width: 8
 *  indent-tabs-mode: nil
 * End:
 */
