#include <stdlib.h>
#include <stdio.h>

#include "MJPEG.h"

#include "conv.h"
#include "define_common.h"
#include "idct.h"
#include "iqzz.h"
#include "resize.h"
#include "screen.h"
#include "upsampler.h"

/* Internal functions : */

void screen2surface_cpyrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, void *ptr, SDL_Surface* screen);

void decode(frame_chunk_t* chunk)
{
  /* to get ((SOF_component[component_index].HV >> 4) & 0xf) now use Streams[video_id].HV */
  uint8_t index;
  int32_t *MCU;
  int32_t unZZ_MCU[64];

  int index_X = chunk->x;
  int index_Y = chunk->y;

  //printf ("Decoding stream_id : %d, frame_id : %d\n", chunk->stream_id, chunk->frame_id);

  int stream_id = chunk->stream_id;
  int frame_id = chunk->frame_id;
  
  //printf ("Decoding max_h : %d, max_v : %d\n", streams[stream_id].max_ss_h, streams[stream_id].max_ss_v);

  uint16_t max_ss_h = streams[stream_id].max_ss_h;
  uint16_t max_ss_v = streams[stream_id].max_ss_v;
  int nb_MCU = ((streams[stream_id].HV >> 4) & 0xf) * (streams[stream_id].HV & 0xf);

  /*  
  uint8_t YCbCr_MCU[3][MCU_sx * MCU_sy * max_ss_h * max_ss_v];
  uint8_t YCbCr_MCU_ds[3][MCU_sx * MCU_sy * max_ss_h * max_ss_v];
  uint32_t RGB_MCU[MCU_sx * MCU_sy * max_ss_h * max_ss_v];
  */

  static volatile int is_init = 0;

  static uint8_t *YCbCr_MCU[3] = { NULL, NULL, NULL};
  static uint8_t *YCbCr_MCU_ds[3] = { NULL, NULL, NULL};
  static uint32_t *RGB_MCU = NULL;

  if (is_init == 0){
    is_init = 1;
    //printf ("wanted size : %d", MCU_sx * MCU_sy * max_ss_h * max_ss_v);
    //printf ("MCU_sx = %d, MCU_sy = %d, max_ss_h = %d, max_ss_v = %d\n",  MCU_sx , MCU_sy , max_ss_h , max_ss_v);
    for (int i = 0; i < 3; i++)
    {
      YCbCr_MCU[i] = malloc(MCU_sx * MCU_sy * max_ss_h * max_ss_v);
      YCbCr_MCU_ds[i] = malloc(MCU_sx * MCU_sy * max_ss_h * max_ss_v);
      if ((YCbCr_MCU_ds[i] == NULL) || (YCbCr_MCU[i] == NULL))
        printf("\nmalloc error line %d\n", __LINE__);
    }

    RGB_MCU = malloc (MCU_sx * MCU_sy * max_ss_h * max_ss_v * sizeof(int32_t));
    if (RGB_MCU == NULL)
      printf("\nmalloc error line %d\n", __LINE__);
  }


  for (index = 0; index < chunk->index; index++)
  {
    uint32_t component_index = chunk->component_index[index];

    for (int chroma_ss = 0; chroma_ss < nb_MCU; chroma_ss++)
    {
      MCU = chunk->data + (index * sizeof(int32_t) * 64);

      int i = chunk->DQT_index[index][chroma_ss];
      iqzz_block(MCU, unZZ_MCU, (uint8_t*)(chunk->DQT_table + i*64));

      IDCT(unZZ_MCU, YCbCr_MCU_ds[component_index] + (64 * chroma_ss));
    }

    upsampler(YCbCr_MCU_ds[component_index], YCbCr_MCU[component_index],
        max_ss_h / ((streams[stream_id].HV >> 4) & 0xf),
        max_ss_v / ((streams[stream_id].HV) & 0xf),
        max_ss_h, max_ss_v);
  }

  // TODO : replace RGB_MCU by the right place for display
  if (color && (chunk->index > 1)) {
    YCbCr_to_ARGB (YCbCr_MCU, RGB_MCU, max_ss_h, max_ss_v);
  } else {
    to_NB(YCbCr_MCU, RGB_MCU, max_ss_h, max_ss_v);
  }

  /* TODO : adress access problem to position[stream_id] table (which
     will be modified by click function */
  screen2surface_cpyrect
    (index_Y * MCU_sy * max_ss_h,// + decalage[position[stream_id]].x,
     index_X * MCU_sx * max_ss_v,// + decalage[position[stream_id]].y,
     MCU_sy * max_ss_h,
     MCU_sx * max_ss_v,
     RGB_MCU, Surfaces_normal[stream_id][frame_id % FRAME_LOOKAHEAD]);
  
  //TODO : lock start
  //  printf ("\tIncrement Achievement for stream %d frame %d value %d\n", stream_id, frame_id, Achievements[stream_id][frame_id]);
  Achievements[stream_id][frame_id % FRAME_LOOKAHEAD]++;
  if (Achievements[stream_id][frame_id % FRAME_LOOKAHEAD] == streams[stream_id].nb_MCU)
    {
      Achievements[stream_id][frame_id % FRAME_LOOKAHEAD] = 0;
      printf("\nCall Resize Component for stream %d frame %d\n", stream_id, frame_id);
      printf("\nAchievements[%d][%d] value %d\n", stream_id, frame_id, Achievements[stream_id][frame_id]); 
      resize (chunk);
    }
  //TODO : lock end
}

void screen2surface_cpyrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, void *ptr, SDL_Surface* screen)
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
