#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

#define BUFFER_SIZE 4096

typedef struct {
	struct jpeg_source_mgr pub; /* public fields */
	int srcSize;
	boolean start_of_file;
	JOCTET * srcBuffer;
	JOCTET buffer[BUFFER_SIZE];
} my_source_mgr;

//typedef struct {
 // struct jpeg_source_mgr pub;	/* public fields */

  //FILE * infile;		/* source stream */
  //JOCTET * buffer;		/* start of buffer */
 // boolean start_of_file;	/* have we gotten any data yet? */
//} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
/*SL: How to throw an error from there?*/
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  src->start_of_file = TRUE;
}

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;
	size_t nbytes = 0;	
	/* Create a fake EOI marker */
	if(src->srcSize > BUFFER_SIZE) { 
		nbytes = BUFFER_SIZE; 
	}
	else
	{ 
		nbytes = src->srcSize; 
	}
	if(nbytes <= 0)
	{ 
		if(src->start_of_file) { 
			exit(1); // treat empty input file as fatal error 
		}
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}
	else
	{
		memcpy(src->buffer, src->srcBuffer,nbytes);
		src->srcBuffer += nbytes;
		src->srcSize -= nbytes;
	}

	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_file = FALSE;

	return TRUE;
}


METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;
	if (num_bytes <= src->pub.bytes_in_buffer ) 
		{
			src->pub.bytes_in_buffer -= num_bytes;
			src->pub.next_input_byte += num_bytes;
		}
	else
	{
		num_bytes -= src->pub.bytes_in_buffer;
		src->pub.bytes_in_buffer = 0;
		src->srcBuffer += num_bytes;
		src->srcSize -= num_bytes;
	}
}

METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
	(void)cinfo;
}

GLOBAL(void)
jpeg_buffer_src (j_decompress_ptr cinfo, JOCTET* inbuffer, size_t bytes_in_buffer)
{
  //struct jpeg_source_mgr* src;
	my_src_ptr src;

  if (cinfo->src == NULL) {   /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
              SIZEOF(my_source_mgr));
  }

  src = (my_src_ptr)cinfo->src;
  src->srcBuffer = inbuffer;
  src->srcSize = (int)bytes_in_buffer;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->pub.next_input_byte = NULL;//inbuffer;
  src->pub.bytes_in_buffer = 0;//NULL;//bytes_in_buffer;
}