/*
 * main.h.
 *
 * Generated by c2x version 0.9.2 adapt priority.
 *
 */

#include <stddef.h>

/* GTG ugly stuff : */
extern uint64_t epoc;
#include "MJPEG.h"

/**********************************************************************/

/* resize function prototype : */

void resize_body (void* taskarg);

/* resize argument structure : */

typedef struct resize_arg_t {
	 frame_chunk_t* chunk;
} resize_arg_t;


/**********************************************************************/

/* main function prototype : */

void main_body (void* taskarg);

/* main argument structure : */

typedef struct main_arg_t {
	 int argc;
	 char** argv;
} main_arg_t;


/**********************************************************************/

/* fetch function prototype : */

void fetch_body (void* taskarg);

/* fetch argument structure : */

typedef struct fetch_arg_t {
	 int fid;
} fetch_arg_t;


/**********************************************************************/

/* render function prototype : */

void render_body (void* taskarg);

/* render argument structure : */

typedef struct render_arg_t {
	 int width;
	 int height;
	 int framerate;
} render_arg_t;


/**********************************************************************/

/* decode function prototype : */

void decode_body (void* taskarg);

/* decode argument structure : */

typedef struct decode_arg_t {
	 frame_chunk_t* chunk;
} decode_arg_t;
