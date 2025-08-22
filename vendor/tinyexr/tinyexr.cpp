//Please include your own zlib-compatible API header before
//including `tinyexr.h` when you disable `TINYEXR_USE_MINIZ`
#define TINYEXR_USE_MINIZ 0
//#include "zlib.h"
//Or, if your project uses `stb_image[_write].h`, use their
//zlib implementation:
#define TINYEXR_USE_STB_ZLIB 1
#define TINYEXR_IMPLEMENTATION
#include "tinyexr/tinyexr.h"

