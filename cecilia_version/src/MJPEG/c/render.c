#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <SDL/SDL.h>
//#include <SDL/sge.h>

#include "MJPEG.h"

// Define debug for component Render to print info when a block is copied out
// of bound.
#define _RENDER_DEBUG

#define test

static uint32_t global_framerate;
static SDL_Surface *screen;

// Frame rate management, stores previous counter value
static int old_time;

// Global timeouted frames table :
int32_t Done[FRAME_LOOKAHEAD];
int32_t Drop[FRAME_LOOKAHEAD];
uint8_t initialized;

void cpyrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, void *ptr)
{
  void *dest_ptr;
  void *src_ptr;
  uint32_t line;
  uint32_t w_internal = w, h_internal = h;
  int w_length;

  SDL_LockSurface(screen);
  w_length = screen->pitch / (screen->format->BitsPerPixel / 8 );

#ifdef DEBUG
  if ((y) > screen->h) {
    printf("[%s] : block can't be copied, "
        "not in the screen (too low)\n", __func__);
    exit(1);
  }
  if ((x) > screen->w) {
    printf("[%s] : block can't be copied, "
        "not in the screen (right border)\n", __func__);
    exit(1);
  }
#endif
  if ((x+w) > screen->w) {
    w_internal = screen->w -x; 
  }
  if ((y+h) > screen->h) {
    h_internal = screen->h -y; 
  }
  for(line = 0; line < h_internal ; line++)
  {   
    // Positionning src and dest pointers
    //  _ src : must be placed at the beginning of line "line"
    //  _ dest : must be placed at the beginning
    //          of the corresponding block :
    //(line offset + current line + position on the current line)
    // We assume that RGB is 4 bytes

    dest_ptr = (void*)((uint32_t *)(screen->pixels) +
        ((y+line)*w_length) + x); 
    src_ptr = (void*)((uint32_t *)ptr + ((line * w)));
    memcpy(dest_ptr,src_ptr,w_internal * sizeof(uint32_t));
  }   

  SDL_UnlockSurface(screen);
}


void render_init(int width, int height, int framerate)
{

  int width_int = width , height_int = height;

  /* Initialize the SDL library */
  if (SDL_Init(SDL_INIT_VIDEO) < 0)  {
    fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  /* Clean up on exit */
  atexit(SDL_Quit);

  /* Framerate handling */
  if (framerate == 0) {
    global_framerate = 25; 
  } else {
    global_framerate = framerate;
  }

  /* Screen initialization */
  screen = SDL_SetVideoMode(width_int, height_int, 32, SDL_SWSURFACE);
  if (screen == NULL) {
    fprintf(stderr, "Couldn't set %dx%dx32 video mode for screen: %s\n",
        width, height, SDL_GetError());
    exit(1);
  }

  for (int fid = 0; fid < FRAME_LOOKAHEAD; fid++)
    Drop[fid] = -1;

  old_time = SDL_GetTicks();
  initialized = 1;
}

int render_exit()
{
  /* Shutdown all subsystems */
  SDL_Event event;
  while(initialized){
    if (SDL_PollEvent(&event)) {
      if ((event.type == SDL_QUIT )) {
        printf("\n");
        SDL_Quit();
        return 1;
      }
    }
    /*  
     * Something cleaner than polling would be nice
     * but waiting for a better solution, don't
     * waste all CPU time.
     */
    sleep(0);
  }
  return 0;
}



void render(void* nothing)
{
  //CALL (REQUIRED.fetch, fetch);

  int delay, frame_id, dropped = 0;
  uint64_t finish_time;
  SDL_Event event;

  /* TODO auto init?
     if (last_frame_id == -1)
     render_init ();
     */

  sleep(1);

  /* Because now it's a thread. */
  while (1) {



compute:

    finish_time = SDL_GetTicks();

    // Check if we can actually copy frame with Done[frame_id] table
    // filed by resize component et RAZ by render component.
    frame_id = last_frame_id + 1;

    while (Done[frame_id % FRAME_LOOKAHEAD] != nb_streams) {
#ifdef _RENDER_DEBUG
      printf("\nRENDER_DEBUG :: frame %d not ready yet (Done = %d\n",
          frame_id, Done[frame_id % FRAME_LOOKAHEAD]);
#endif
      delay = (1000/global_framerate) + old_time - SDL_GetTicks();
      if (delay > 0) {
        // TODO : Find a better delay?
        SDL_Delay(10);
      } else {
        // TODO : drop frames
        printf ("should drop frame, already dropped %d\n", dropped);
        last_frame_id++;
        frame_id = last_frame_id + 1;

        if (dropped < FRAME_LOOKAHEAD)
        {
          dropped++;
          Drop[frame_id % FRAME_LOOKAHEAD] = 1;
          goto compute;
        } else {
          printf ("FRAME_LOOKAHEAD frame to be dropped, aborting");
          abort();
        }
      }
    }

    SDL_Surface* src = Surfaces_resized[frame_id % FRAME_LOOKAHEAD];

    cpyrect(0,0,WINDOW_H, WINDOW_W, src->pixels);

    Done[frame_id % FRAME_LOOKAHEAD] = 0;

    if (dropped == 0)
    {
      printf ("\n no frame droped \n");
      // fetch();
    } else {
      printf ("\n %d dropped frames\n", dropped);
      // TODO : clean system? what to launch ? then update dropped
      dropped = 0;
    }

    delay = (1000 / global_framerate) + old_time - SDL_GetTicks();
    if (delay > 0 ) {
      // Use this time to compute droped frame.
      SDL_Delay(delay);
    }

    if (SDL_Flip(screen) == -1) {
      printf("Could not refresh screen: %s\n.", SDL_GetError() );
    }

    // update next frame to print :
    last_frame_id++;

    //\r is required to avoid latency due to screening on terminal
    printf("\r[screen] : framerate is %0.2ffps, "
        "computed one image in %0.2fms",
        1000.00f / (SDL_GetTicks() - old_time),
        (finish_time - old_time) * 1.00f);

    old_time = SDL_GetTicks();
    // Check wether close event was detected, otherwise SDL freezes
    if(SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_q) {
          printf("bye bye\n");
          initialized = 0;
          SDL_Quit();
          exit(1);
        } 
      }   
      if (event.type == SDL_QUIT) {
        printf("\n");
        initialized = 0;
        SDL_Quit();
        exit(1);
      }
    }
  }
  exit(0);
}
