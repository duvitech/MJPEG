#include <stdio.h>
#include "MJPEG.h"

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

/* Global definition : */

// Global stream table :
stream_info_t* Streams;

// Global frame achievement table (nb_chunk already treated) :
uint32_t* Achievements[FRAME_LOOKAHEAD];

/* Intern declarations : */

int32_t* MCUs[FRAME_LOOKAHEAD][3][64];
int32_t* unZZ_MCUs[FRAME_LOOKAHEAD][3][64];

/* Intern functions : */

static void usage(char *progname) {
	printf("Usage : %s [options] <mjpeg_file>\n", progname);
	printf("Options list :\n");
	printf("\t-f <framerate> : set framerate for the movie\n");
	printf("\t-h : display this help and exit\n");
}

int METHOD(entry, main)(void *_this, int argc, char** argv)
{
  printf ("Main start\n");

  /* TODO : dynamic alloc :
   * Streams 
   * Achievements
   * MCUs
   * unZZ_MCUs
   */

  CALL (REQUIRED.decode, decode_init); //?
  CALL (REQUIRED.resize, resize_init);
  CALL (REQUIRED.render, render_init);
  CALLMINE (fetch, fetch);
  printf ("Main end\n");

}

void METHOD(fetch, fetch)(void *_this)
{

  printf ("Fetch start\n");
  CALL (REQUIRED.decode, decode);
  printf ("Fetch end\n");

}
