#ifndef DEFINE_COMMON_H
#define DEFINE_COMMON_H

#define SWITCH_ENDIAN_32(x)   \
	x = ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8)  |  \
((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24)

#define SWITCH_ENDIAN_16(x) \
	x = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8)

#define CPU_DATA_IS_BIGENDIAN(type,data) SWITCH_ENDIAN_##type(data)

#define FILE_ERROR(file, msg, expected, actual) \
        fprintf(stderr, "Error reading input file " msg " (%d elements read, expected %d)\n", \
                actual, expected); \
        if (feof(file)) fprintf(stderr, "(end of file)\n"); \
        /* exit(1) */

#define NEXT_TOKEN(res, movie) {					\
	if (fread ((void *) & res, 1, 1, movie) != 1) {         \
		FILE_ERROR(movie, "in NEXT_TOKEN", 1, 1);       \
	}                                                       \
}

#define COPY_SECTION(to, size, movie) {			    \
	int elem_read = fread ((void *)to, size, 1, movie); \
	if (elem_read != 1) {			\
		FILE_ERROR(movie, "in COPY_SECTION", elem_read, 1); \
                abort();\
	}					\
}

#define SKIP(n, movie) {							\
	uint8_t * waste=(uint8_t *)malloc(n);                           \
	if (n != 0) {                                                   \
                int elem_read = fread ((void *) waste, n, 1, movie);    \
		if (elem_read != 1) {                                   \
			FILE_ERROR(movie, "in SKIP", elem_read, n);     \
		}                                                       \
        }                                                               \
	free(waste);                                                    \
}


#define BLACK   30
#define RED     31
#define GREEN   32
#define YELLOW  33
#define BLUE    34
#define MAGENTA 35
#define CYAN    36
#define WHITE   37

#ifdef VERBOSE
#define VPRINTF(format, ...) printf ("[%s] " format, __FUNCTION__, ## __VA_ARGS__)
#else
#define VPRINTF(format, ...)
#endif

#ifdef INFO
#define IPRINTF(format, ...) printf ("[%s] " format, __FUNCTION__, ## __VA_ARGS__)
#else
#define IPRINTF(format, ...)
#endif

#ifdef INTER
#define APRINTF(format, ...) printf (format, ## __VA_ARGS__)
#else
#define APRINTF(format, ...)
#endif

#ifdef _RESIZE_DEBUG
#define PRESIZE(format, ...) printf ("%c[%d;%d;%dm[Resize::%s]%c[%d;%d;%dm " format, 0x1B, 0,GREEN,40,__FUNCTION__, 0x1B, 0, WHITE, 40, ## __VA_ARGS__)
#else
#define PRESIZE(format, ...)
#endif

#ifdef _FETCH_DEBUG
#define PFETCH(format, ...) printf ("%c[%d;%d;%dm[Fetch::%s]%c[%d;%d;%dm " format, 0x1B, 0,BLUE,40,__FUNCTION__, 0x1B,0, WHITE, 40, ## __VA_ARGS__)
#else
#define PFETCH(format, ...)
#endif

#ifdef _DECODE_DEBUG
#define PDECODE(format, ...) printf ("%c[%d;%d;%dm[Decode::%s]%c[%d;%d;%dm " format, 0x1B, 0,YELLOW,40,__FUNCTION__, 0x1B, 0, WHITE, 40, ## __VA_ARGS__)
#else
#define PDECODE(format, ...)
#endif

#ifdef _RENDER_DEBUG
#define PRENDER(format, ...) printf ("%c[%d;%d;%dm[Render::%s]%c[%d;%d;%dm " format, 0x1B, 0,RED,40,__FUNCTION__, 0x1B, 0, WHITE, 40, ## __VA_ARGS__)
#else
#define PRENDER(format, ...)
#endif

#endif //DEFINE_COMMON_H