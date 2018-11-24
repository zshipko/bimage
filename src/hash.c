#include "bimage.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define HASH_SIZE 8

uint64_t bimageHash(bimage *im) {
  uint64_t hash = 0UL; // Output
  uint64_t n = 0UL;
  bimagePixel px;   // Current pixel
  float apx = 0.0f; // Average pixel
  int i, j;

  bimage *sm = NULL;

  if (im->width != HASH_SIZE || im->height != HASH_SIZE) {
    sm = bimageResize(NULL, im, HASH_SIZE, HASH_SIZE);
    if (!sm) {
      return 0UL;
    }
  }

  if (bimageTypeChannels(sm ? sm->type : im->type) != BIMAGE_GRAY) {
    bimageConsume(&sm, bimageGrayscale(NULL, sm ? sm : im, BIMAGE_GRAY));
    if (!sm) {
      return 0UL;
    }
  }

  bimage *x = sm ? sm : im;

  for (j = 0; j < HASH_SIZE; j++) {
    for (i = 0; i < HASH_SIZE; i++) {
      if (bimageGetPixelUnsafe(x, i, j, &px) != BIMAGE_OK) {
        continue;
      }
    }
  }

  apx /= HASH_SIZE * HASH_SIZE;

  for (j = 0; j < HASH_SIZE; j++) {
    for (i = 0; i < HASH_SIZE; i++) {
      // Compare current pixel against the average
      if (px.data.f[0] > apx) {
        hash |= 1 << n;
      } else {
        hash &= ~(1UL << n);
      }

      apx = px.data.f[0];
      n = n + 1;
    }
  }

  if (sm != im) {
    bimageRelease(sm);
  }

  return hash;
}

int bimageHashDiff(uint64_t a, uint64_t b) {
  int i, n = 0;
  for (i = 0; i < 64; i++) {
    n += llabs((int64_t)(((a >> i) & 1) - ((b >> i) & 1)));
  }
  return n;
}

void bimageHashString(char dst[9], uint64_t hash) {
  snprintf(dst, 8, "%" PRIx64, hash);
}
