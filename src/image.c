#include "bimage.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#ifndef BIMAGE_NO_PTHREAD
#include <pthread.h>
#endif

int BIMAGE_NUM_CPU = -1;

/* BIMAGE TYPE */

double bimageTypeMax(BIMAGE_TYPE t) {
  switch (bimageTypeDepth(t)) {
  case BIMAGE_U8:
    return (double)0xFF;
  case BIMAGE_U16:
    return (double)0xFFFF;
  case BIMAGE_U32:
    return (double)0xFFFFFFFF;
  case BIMAGE_F32:
    return 1.0;
  case BIMAGE_C32:
    return 1.0 + 0.0i;
  case BIMAGE_F64:
    return 1.0;
  default:
    return 1.0;
  }
}

/* BIMAGE */

bimage *bimageCreateWithData(uint32_t width, uint32_t height, BIMAGE_TYPE t,
                             void *data, bool owner, bool ondisk) {
  if (!data) {
    return NULL;
  }

  if (width == 0 || height == 0) {
    if (owner) {
      bFree(data);
    }
    return NULL;
  }

  bimage *im = bAlloc(sizeof(bimage));
  if (!im) {
    if (owner) {
      bFree(data);
    }
    return NULL;
  }

  im->width = width;
  im->height = height;
  im->type = t;
  im->data = data;
  im->owner = owner;
  im->ondisk = ondisk;

  return im;
}

size_t bimageDepthSize(BIMAGE_DEPTH d) {
  switch (d) {
  case BIMAGE_U8:
    return 8;
  case BIMAGE_U16:
    return 16;
  case BIMAGE_U32:
    return 32;
  case BIMAGE_F32:
    return 32;
  case BIMAGE_C32:
    return 64;
  case BIMAGE_F64:
    return 64;
  default:
    return 0;
  }
}

bimage *bimageCreate(uint32_t width, uint32_t height, BIMAGE_TYPE t) {
  if (width == 0 || height == 0) {
    return NULL;
  }

  void *data = bAlloc(bimageTotalSize(width, height, t));
  if (!data) {
    return NULL;
  }

  return bimageCreateWithData(width, height, t, data, true, false);
}

bimage *bimageClone(bimage *image) {
  bimage *tmp = bimageCreate(image->width, image->height, image->type);
  if (!tmp) {
    return NULL;
  }

  memcpy(tmp->data, image->data,
         bimageTotalSize(image->type, image->width, image->height));

  return tmp;
}

#define MMAP_HEADER_SIZE ((sizeof(uint32_t) * 3) + 4)

bimage *bimageCreateOnDiskFd(int fd, uint32_t width, uint32_t height,
                             BIMAGE_TYPE t) {
  bool loadFile =
      (width == 0 && height == 0) || bimageTypeDepth(t) == BIMAGE_UNKNOWN;
  if (loadFile) {
    char hdr[4];
    if (read(fd, &hdr, 4) != 4) {
      return NULL;
    }

    if (strncmp(hdr, "BIMG", 4) != 0) {
      return NULL;
    }

    uint32_t tmp;
    if (read(fd, &tmp, sizeof(tmp)) != sizeof(tmp)) {
      return NULL;
    }
    t = (BIMAGE_TYPE)ntohl(tmp);

    if (read(fd, &tmp, sizeof(tmp)) != sizeof(tmp)) {
      return NULL;
    }
    width = ntohl(tmp);

    if (read(fd, &height, sizeof(tmp)) != sizeof(tmp)) {
      return NULL;
    }
    height = ntohl(tmp);

    if (width == 0 || height == 0 || bimageTypeDepth(t) == BIMAGE_UNKNOWN ||
        bimageTypeChannels(t) == 0) {
      return NULL;
    }

    if (lseek(fd, 0, SEEK_END) < width * height *
                                     bimageDepthSize(bimageTypeDepth(t)) *
                                     bimageTypeChannels(t)) {
      return NULL;
    }

    lseek(fd, 0, SEEK_SET);
  }

  // Write header for new images
  if (!loadFile) {
    if (write(fd, "BIMG", 4) != 4) {
      return NULL;
    }

    uint32_t tmp = htonl((uint32_t)t);
    if (write(fd, &tmp, sizeof(tmp)) != sizeof(tmp)) {
      return NULL;
    }

    tmp = htonl(width);
    if (write(fd, &tmp, sizeof(tmp)) != sizeof(tmp)) {
      return NULL;
    }

    tmp = htonl(height);
    if (write(fd, &tmp, sizeof(tmp)) != sizeof(tmp)) {
      return NULL;
    }

    // Allocate enough memory on disk
    lseek(fd, bimageTotalSize(width, height, t), SEEK_SET);
    if (write(fd, "\0", 1) != 1) {
      return NULL;
    }
  }

  void *data = mmap(NULL, bimageTotalSize(width, height, t) + MMAP_HEADER_SIZE,
                    PROT_READ | PROT_WRITE, MAP_SHARED, fd, false);

  // Check data
  if (!data) {
    return NULL;
  }

  return bimageCreateWithData(width, height, t,
                              (uint8_t *)data + MMAP_HEADER_SIZE, true, true);
}

bimage *bimageCreateOnDisk(const char *filename, uint32_t width,
                           uint32_t height, BIMAGE_TYPE t) {
  bool loadExisting = (width == 0 && height == 0) || t == BIMAGE_UNKNOWN;
  int fd = open(filename, loadExisting ? O_RDWR : O_RDWR | O_CREAT, 0655);
  if (fd < 0) {
    return NULL;
  }

  bimage *im = bimageCreateOnDiskFd(fd, width, height, t);

  close(fd);
  return im;
}

bool bimageIsValid(bimage *im) {
  return im && im->data && im->width > 0 && im->height > 0;
}

static void bimageReleaseData(bimage *im) {
  if (im->data) {
    if (im->ondisk) {
      munmap((uint8_t *)im->data - MMAP_HEADER_SIZE,
             bimageTotalSize(im->width, im->height, im->type));
    } else {
      bFree(im->data);
    }

    im->data = NULL;
  }
}

void bimageRelease(bimage *im) {
  if (im) {
    if (im->owner) {
      bimageReleaseData(im);
    }

    bFree(im);
  }
}

void bimageDestroy(bimage **im) {
  if (im && *im) {
    bimageRelease(*im);
    *im = NULL;
  }
}

BIMAGE_STATUS
bimageMapToDisk(const char *filename, bimage **im) {
  bimage *tmp =
      bimageCreateOnDisk(filename, (*im)->width, (*im)->height, (*im)->type);
  if (!tmp) {
    return BIMAGE_ERR;
  }

  // Copy pixel data to temporary image
  memcpy(tmp->data, (*im)->data,
         bimageTotalSize(tmp->width, tmp->height, tmp->type));

  // Copy data to destination image
  bimageReleaseData((*im));
  (*im)->owner = true;
  (*im)->ondisk = true;
  (*im)->data = tmp->data;

  // Free temporary image
  tmp->owner = false;
  bimageRelease(tmp);

  return BIMAGE_OK;
}

bimage *bimageConsume(bimage **dst, bimage *src) {
  bimageRelease(*dst);
  *dst = src;
  return *dst;
}

void *bimageDataOffs(bimage *im, uint32_t x, uint32_t y) {
  int64_t offs = bimageIndex(im, x, y);
  switch (bimageTypeDepth(im->type)) {
  case BIMAGE_U8:
    return ((uint8_t *)im->data) + offs;
  case BIMAGE_U16:
    return ((uint16_t *)im->data) + offs;
  case BIMAGE_U32:
    return ((uint32_t *)im->data) + offs;
  case BIMAGE_F32:
    return ((float *)im->data) + offs;
  case BIMAGE_C32:
    return ((float _Complex *)im->data) + offs;
  case BIMAGE_F64:
    return ((double *)im->data) + offs;
  default:
    return NULL;
  }
}

BIMAGE_STATUS
bimageGetPixelUnsafe(bimage *im, uint32_t x, uint32_t y, bimagePixel *p) {
  if (bimageTypeChannels(im->type) > 4) {
    return BIMAGE_ERR;
  }

  int i;
  int64_t offs = bimageIndex(im, x, y);
  double max = bimageTypeMax(im->type);
  p->data.f[3] = 1.0;

  for (i = 0; i < bimageTypeChannels(im->type); i++) {
    switch (bimageTypeDepth(im->type)) {
    case BIMAGE_U8:
      p->data.f[i] = (float)((double)bimageAt(im, offs + i, uint8_t)) / max;
      break;
    case BIMAGE_U16:
      p->data.f[i] = (float)((double)bimageAt(im, offs + i, uint16_t)) / max;
      break;
    case BIMAGE_U32:
      p->data.f[i] = (float)((double)bimageAt(im, offs + i, uint32_t)) / max;
      break;
    case BIMAGE_F32:
      p->data.f[i] = (bimageAt(im, offs + i, float));
      break;
    case BIMAGE_C32:
      p->data.f[i] = (float)(bimageAt(im, offs + i, float _Complex));
      break;
    case BIMAGE_F64:
      p->data.f[i] = (float)(bimageAt(im, offs + i, double));
      break;
    default:
      return BIMAGE_ERR;
    }
  }

  // Grayscale bimagePixels should have the same value for RGB channels
  if (bimageTypeChannels(im->type) == 1) {
    p->data.f[1] = p->data.f[2] = p->data.f[0];
  }

  return BIMAGE_OK;
}

BIMAGE_STATUS
bimageGetPixel(bimage *im, uint32_t x, uint32_t y, bimagePixel *p) {
  if (!p) {
    return BIMAGE_ERR;
  }

  if (!bimageBoundsCheck(im, x, y)) {
    bimagePixelZero(p);
    return BIMAGE_ERR;
  }

  return bimageGetPixelUnsafe(im, x, y, p);
}

BIMAGE_STATUS
bimageSetPixelUnsafe(bimage *im, uint32_t x, uint32_t y, bimagePixel p) {
  int i;
  int64_t offs = bimageIndex(im, x, y);
  double max = bimageTypeMax(im->type);
  for (i = 0; i < bimageTypeChannels(im->type) && i < 4; i++) {
    switch (bimageTypeDepth(im->type)) {
    case BIMAGE_U8:
      bimageAt(im, offs + i, uint8_t) = (uint8_t)((double)p.data.f[i] * max);
      break;
    case BIMAGE_U16:
      bimageAt(im, offs + i, uint16_t) = (uint16_t)((double)p.data.f[i] * max);
      break;
    case BIMAGE_U32:
      bimageAt(im, offs + i, uint32_t) = (uint32_t)((double)p.data.f[i] * max);
      break;
    case BIMAGE_F32:
      bimageAt(im, offs + i, float) = p.data.f[i];
      break;
    case BIMAGE_C32:
      bimageAt(im, offs + i, float _Complex) = (float _Complex)p.data.f[i];
      break;
    case BIMAGE_F64:
      bimageAt(im, offs + i, double) = (double)p.data.f[i];
      break;
    default:
      return BIMAGE_ERR;
    }
  }

  return BIMAGE_OK;
}

BIMAGE_STATUS
bimageSetPixel(bimage *im, uint32_t x, uint32_t y, bimagePixel p) {
  if (!bimageBoundsCheck(im, x, y)) {
    return BIMAGE_ERR;
  }

  return bimageSetPixelUnsafe(im, x, y, p);
}

/* bimageConvertDepth converts between 8, 16 and 32 bit images */
bimage *bimageConvertDepth(bimage *dst, bimage *im, BIMAGE_DEPTH depth) {
  bimagePixel px = BIMAGE_PIXEL_INIT;
  bimage *im2 = BIMAGE_CREATE_DEST(dst, im->width, im->height,
                                   depth | bimageTypeChannels(im->type));
  if (!im2) {
    return NULL;
  }

  bimageIterAll(im, x, y) {
    if (bimageGetPixelUnsafe(im, x, y, &px) == BIMAGE_OK) {
      bimageSetPixelUnsafe(im2, x, y, px);
    } else {
      break;
    }
  }

  return im2;
}

/* bimageConvertChannels converts between 1, 3, and 4 channel images
 * */
bimage *bimageConvertChannels(bimage *dst, bimage *im,
                              BIMAGE_CHANNEL nchannels) {
  bimage *im2 = BIMAGE_CREATE_DEST(dst, im->width, im->height,
                                   bimageTypeDepth(im->type) | nchannels);
  if (!im2) {
    return NULL;
  }

  bimagePixel px = BIMAGE_PIXEL_INIT;
  bimageIterAll(im, x, y) {
    if (bimageGetPixelUnsafe(im, x, y, &px) == BIMAGE_OK) {
      bimageSetPixel(im2, x, y, px);
    }
  }

  return im2;
}

bimage *bimageCrop(bimage *dst, bimage *im, uint32_t x, uint32_t y, uint32_t w,
                   uint32_t h) {
  bimage *im2 = BIMAGE_CREATE_DEST(dst, w, h, im->type);
  if (!im2) {
    return NULL;
  }

  bimagePixel px = BIMAGE_PIXEL_INIT;
  bimageIter(im, i, j, x, y, w, h, 1, 1) {
    if (bimageGetPixel(im, i, j, &px) == BIMAGE_OK) {
      bimageSetPixel(im2, i - x, j - y, px);
    } else {
      break;
    }
  }

  return im2;
}

void bimageCopyTo(bimage *dst, bimage *src, uint32_t offs_x, uint32_t offs_y) {
  if (offs_x >= dst->width || offs_y >= dst->height) {
    return;
  }

  bimagePixel px = BIMAGE_PIXEL_INIT;
  bimageIterAll(src, x, y) {
    bimageGetPixelUnsafe(src, x, y, &px);
    bimageSetPixel(dst, x + offs_x, y + offs_y, px);
  }
}

void bimageAdjustGamma(bimage *im, float g) {
  int i, c = bimageTypeChannels(im->type);
  c = c > 3 ? 3 : c; // Ignore alpha channel
  bimagePixel px = BIMAGE_PIXEL_INIT;
  bimageIterAll(im, x, y) {
    bimageGetPixelUnsafe(im, x, y, &px);
    for (i = 0; i < c; i++) {
      px.data.f[i] = pow((px.data.f[i]), (1.0 / g));
    }
    bimageSetPixel(im, x, y, px);
  }
}

bimagePixel bimageAverageInRect(bimage *im, uint32_t x, uint32_t y, uint32_t w,
                                uint32_t h) {
  int64_t n = -1;
  bimagePixel px = BIMAGE_PIXEL_INIT, dst = bimagePixelCreate(0, 0, 0, 0);
  bimageIter(im, i, j, x, y, w, h, 1, 1) {
    if (bimageGetPixel(im, i, j, &px) == BIMAGE_OK) {
      bimagePixelAdd(&dst, px);
      n += 1;
    }
  }

  bimagePixelDiv(&dst, bimagePixelCreate(n, n, n, n));
  return dst;
}

bimage *bimageRandom(bimage *dst, uint32_t w, uint32_t h, BIMAGE_TYPE t) {
  bimage *im = BIMAGE_CREATE_DEST(dst, w, h, t);
  if (!im) {
    return NULL;
  }

  bimageIterAll(im, x, y) { bimageSetPixel(im, x, y, bimagePixelRandom()); }

  return im;
}

#ifndef BIMAGE_NO_PTHREAD

struct bimageParallelIterator {
  uint32_t x0, y0, x1, y1;
  bimage *image0, *image1;
  bimageParallelFn f;
  void *userdata;
};

void *bimageParallelWrapper(void *_iter) {
  struct bimageParallelIterator *iter = (struct bimageParallelIterator *)_iter;
  bimagePixel px;
  uint32_t i, j;
  for (j = iter->y0; j < iter->y1; j++) {
    for (i = iter->x0; i < iter->x1; i++) {
      if (bimageGetPixelUnsafe(iter->image1, i, j, &px) == BIMAGE_OK) {
        if (iter->f(i, j, &px, iter->userdata)) {
          bimageSetPixel(iter->image0, i, j, px);
        }
      }
    }
  }

  return NULL;
}

BIMAGE_STATUS
bimageEachPixel2(bimage *dst, bimage *im, bimageParallelFn fn, int nthreads,
                 void *userdata) {
  if (im == NULL) {
    return BIMAGE_ERR;
  }

  if (dst == NULL) {
    dst = im;
  }

  if (nthreads <= 0) {
    nthreads = sysconf(_SC_NPROCESSORS_ONLN);
  } else if (nthreads == 1) {
    int i, j;
    bimagePixel px;
    for (j = 0; j < im->height; j++) {
      for (i = 0; i < im->width; i++) {
        if (bimageGetPixelUnsafe(im, i, j, &px) == BIMAGE_OK) {
          if (fn(i, j, &px, userdata)) {
            bimageSetPixel(dst, i, j, px);
          }
        }
      }
    }
    return BIMAGE_OK;
  }

  pthread_t threads[nthreads];
  int tries = 1, n;
  uint32_t height, x;

  height = im->height / nthreads;

  for (x = 0; x < nthreads; x++) {
    struct bimageParallelIterator iter;
    iter.x1 = im->width;
    iter.x0 = 0;
    iter.y1 = height * x;
    iter.y0 = height;
    iter.userdata = userdata;
    iter.image0 = dst;
    iter.image1 = im;
    iter.f = fn;
    if (pthread_create(&threads[x], NULL, bimageParallelWrapper, &iter) != 0) {
      if (tries <= 5) {
        x -= 1;
        tries += 1;
      } else {
        return BIMAGE_ERR;
      }
    } else {
      tries = 1;
    }
  }

  for (n = 0; n < nthreads; n++) {
    // Maybe do something if this fails?
    pthread_join(threads[n], NULL);
  }

  return BIMAGE_OK;
}

BIMAGE_STATUS
bimageEachPixel(bimage *im, bimageParallelFn fn, int nthreads, void *userdata) {
  return bimageEachPixel2(NULL, im, fn, nthreads, userdata);
}

bimage *bimageGetChannel(bimage *dest, bimage *im, int c) {
  if (c >= bimageTypeChannels(im->type)) {
    return NULL;
  }

  bimage *tmp = BIMAGE_CREATE_DEST(dest, im->width, im->height,
                                   bimageTypeDepth(im->type) | 1);
  if (!tmp) {
    return NULL;
  }

  bimagePixel px;
  bimageIterAll(im, x, y) {
    bimageGetPixelUnsafe(im, x, y, &px);
    px.data.f[0] = px.data.f[c];
    bimageSetPixelUnsafe(tmp, x, y, px);
  }

  return tmp;
}

BIMAGE_STATUS
bimageSetChannel(bimage *dest, bimage *im, int c) {
  if (c >= bimageTypeChannels(im->type) || im->width != dest->width ||
      im->height != dest->height) {
    return BIMAGE_ERR;
  } else if (!dest) {
    return BIMAGE_ERR;
  }

  bimagePixel px = BIMAGE_PIXEL_INIT;
  bimageIterAll(im, x, y) {
    bimageGetPixelUnsafe(im, x, y, &px);
    px.data.f[c] = px.data.f[0];
    bimageSetPixel(dest, x, y, px);
  }

  return BIMAGE_OK;
}

bimage **bimageSplitChannels(bimage *im, int *num) {
  bimage **dest = malloc(sizeof(bimage *) * bimageTypeChannels(im->type));
  if (!dest) {
    return NULL;
  }

  int i;
  for (i = 0; i < bimageTypeChannels(im->type); i++) {
    dest[i] = bimageGetChannel(NULL, im, i);
  }

  if (num) {
    (*num) = i;
  }

  return dest;
}

bimage *bimageJoinChannels(bimage *dest, bimage **channels, int num) {
  if (num <= 0) {
    return NULL;
  }

  int i;
  bimage *tmp =
      BIMAGE_CREATE_DEST(dest, channels[0]->width, channels[0]->height,
                         bimageTypeDepth(channels[0]->type) | num);
  if (!tmp) {
    return NULL;
  }

  bimagePixel px = BIMAGE_PIXEL_INIT, py = BIMAGE_PIXEL_INIT;
  bimageIterAll(dest, x, y) {

    for (i = 0; i < num; i++) {
      bimageGetPixel(channels[i], x, y, &px);
      py.data.f[i] = px.data.f[0];
    }

    bimageSetPixel(dest, x, y, py);
  }

  return tmp;
}

#endif // BIMAGE_NO_PTHREAD
