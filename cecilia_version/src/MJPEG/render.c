#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <SDL/SDL.h>

#include "MJPEG.h"
#include "define_common.h"

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"


// Global timeouted frames tables :
int32_t Done[FRAME_LOOKAHEAD];
int32_t Free[FRAME_LOOKAHEAD];

// Internal function : 
static void render_init(int width, int height, int framerate);

// Internal declaration :
static int dropped = 0; // number of frame dropped.
static int printed = 0; // number of frame printed.
static uint8_t initialized = 0;
static uint32_t global_framerate;
static SDL_Surface *screen;
// FrameRate management, stores previous counter value

void METHOD(render, render)(void *_this, int width, int height, int framerate)
{
  int delay; // time between two frames.
  int wait; // time to wait before printing next frame.
  int frame_id; // frame id that should be printed now.
  int frame_fetch_id = FRAME_LOOKAHEAD; // next fetched frame id.
  struct timeval beg, end;
  SDL_Event event;
  SDL_Surface* src;

  sleep(1);

  /* screen init and stuff : */
  render_init (width, height, framerate);

  delay = 1000 / global_framerate;
  gettimeofday (&beg, NULL);

  /* Because now it's a thread. */
  while (1)
  {
    /* Regarding the time since SDL init, get the frame id to print : */
    frame_id = SDL_GetTicks() / delay;

    /* Update the last known needed frame id (printed or dropped) : */
    if (last_frame_id != frame_id)
    {
      /* Store time : */
      gettimeofday (&end, NULL);
      TRACE_FRAME (last_frame_id, end, end, "next");

      for (int i = 0; i < FRAME_LOOKAHEAD; i++)
      {
        if ((in_progress[i] < frame_id) && (Done[i] == nb_streams))
        {
          Free[i] = 1;
        }
      }
      last_frame_id = frame_id;
      gettimeofday (&beg, NULL);
      dropped = last_frame_id - printed;
    }

    /* If frame_id is ready : */
    if ((Done[frame_id % FRAME_LOOKAHEAD] == nb_streams) &&
        (in_progress[frame_id % FRAME_LOOKAHEAD] == frame_id))
    {
      /* Copy the buffer into the right SDL_Surface : */
      src = Surfaces_resized[frame_id % FRAME_LOOKAHEAD];
      cpyrect2dest(0,0,WINDOW_H, WINDOW_W, src->pixels, screen);

      /* Store time : */
      gettimeofday (&end, NULL);
      TRACE_FRAME (frame_id, beg, end, "print");

      /* Reset Done and Free struct : */
      Done[frame_id % FRAME_LOOKAHEAD] = 0;
      Free[frame_id % FRAME_LOOKAHEAD] = 1;

      /* Now, wait until we have to print the frame : */
      wait = (delay * frame_id) - SDL_GetTicks();
      if (wait > 0 ) {
        SDL_Delay(wait);
      }

      /* And finally print the frame : */
      PRENDER("Printing frame %d\n", frame_id);
      if (SDL_Flip(screen) == -1)
      {
        printf("Could not refresh screen: %s\n.", SDL_GetError() );
      }
      
      doEvent ("P", frame_id);

      printed++;
    } 
    // TODO : else nanosleep?


    /* If next fetched frame has to be skipped : */
      while (frame_fetch_id <= last_frame_id + 2) //corresponding test fetch.c:150
      {
        //PFRAME ("Call to fetch component to skip frame %d\n", 4, frame_fetch_id);
        gettimeofday (&beg, NULL);

        // TODO TRACE :
        // traceEventThread FETCH START (event so steps are start, exec and
        // stop)!
        linkStart ("R", frame_fetch_id);
        CALL (fetch, fetch, beg);
        frame_fetch_id++;
      }

    /* If we have free places : */
    while (Free[frame_fetch_id % FRAME_LOOKAHEAD])
    {
      //PFRAME ("Call to fetch component to decode frame %d\n", 4, frame_fetch_id);
      gettimeofday (&beg, NULL);

      // TODO TRACE :
      // traceEventThread FETCH START (event so steps are start, exec and
      // stop)!
      linkStart ("R", frame_fetch_id);
      CALL (fetch, fetch, beg);
      Free[frame_fetch_id % FRAME_LOOKAHEAD] = 0;
      frame_fetch_id++;
    }


    /*****************/
    /* Termination : */
    /*****************/

    // Check wether close event was detected, otherwise SDL freezes
    if(SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_q) {
          printf("bye bye\n");
          initialized = 0;
          SDL_Quit();
          end_of_file = 1;
          return;
          //exit(1);
        } 
      }   
      if (event.type == SDL_QUIT) {
        printf("\n");
        initialized = 0;
        SDL_Quit();
        end_of_file = 1;
        return;
        //exit(1);
      }
    }

    if (end_of_file == 1)
    {
      initialized = 0;
      printf ("#dropped frames : %d\n", dropped);
      SDL_Quit();
      return;
    }
  }
}

static void render_init(int width, int height, int framerate)
{
  /* Initialize the SDL library */
  if (SDL_Init(SDL_INIT_VIDEO) < 0)  {
    fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  /* Framerate handling */
  if (framerate <= 0) {
    global_framerate = 25; 
  } else {
    global_framerate = framerate;
  }

  PRENDER ("Starting component with framerate %d\n", global_framerate);

  /* Screen initialization */
  screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
  if (screen == NULL) {
    fprintf(stderr, "Couldn't set %dx%dx32 video mode for screen: %s\n",
        width, height, SDL_GetError());
    exit(1);
  }

  initialized = 1;
}
