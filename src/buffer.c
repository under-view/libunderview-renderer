#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "buffer.h"


struct uvr_buffer uvr_buffer_create(struct uvr_buffer_create_info *uvrbuff) {
  struct gbm_device *gbmdev = NULL;
  struct uvr_buffer_object *bois = NULL;

  if (uvrbuff->bType == GBM_BUFFER || uvrbuff->bType == GBM_BUFFER_WITH_MODIFIERS) {
    gbmdev = gbm_create_device(uvrbuff->kmsFd);
    if (!gbmdev) goto exit_uvr_buffer_null_struct;

    bois = calloc(uvrbuff->bufferCount, sizeof(struct uvr_buffer_object));
    if (!bois) {
      uvr_utils_log(UVR_DANGER, "[x] calloc: %s", strerror(errno));
      goto exit_uvr_buffer_gbmdev_destroy;
    }
  }

  if (uvrbuff->bType == GBM_BUFFER) {
    for (unsigned b = 0; b < uvrbuff->bufferCount; b++) {
      bois[b].kmsFd = uvrbuff->kmsFd;
      bois[b].bo = gbm_bo_create(gbmdev, uvrbuff->width, uvrbuff->height, uvrbuff->pixformat, uvrbuff->gbmBoFlags);
      if (!bois[b].bo) {
        uvr_utils_log(UVR_DANGER, "[x] gbm_bo_create: failed to create gbm_bo with res %u x %u", uvrbuff->width, uvrbuff->height);
        goto exit_uvr_buffer_gbm_bo_detroy;
      }
    }
  }

  if (uvrbuff->bType == GBM_BUFFER_WITH_MODIFIERS) {
    for (unsigned b = 0; b < uvrbuff->bufferCount; b++) {
      bois[b].kmsFd = uvrbuff->kmsFd;
      bois[b].bo = gbm_bo_create_with_modifiers2(gbmdev, uvrbuff->width, uvrbuff->height, uvrbuff->pixformat, uvrbuff->modifiers, uvrbuff->modifierCount, uvrbuff->gbmBoFlags);
      if (!bois[b].bo) {
        uvr_utils_log(UVR_DANGER, "[x] gbm_bo_create_with_modifiers: failed to create gbm_bo with res %u x %u", uvrbuff->width, uvrbuff->height);
        goto exit_uvr_buffer_gbm_bo_detroy;
      }
    }
  }

  for (unsigned b = 0; b < uvrbuff->bufferCount; b++) {
    bois[b].planeCount = gbm_bo_get_plane_count(bois[b].bo);
    bois[b].modifier = gbm_bo_get_modifier(bois[b].bo);
    bois[b].format = gbm_bo_get_format(bois[b].bo);

    for (unsigned p = 0; p < bois[b].planeCount; p++) {
      union gbm_bo_handle h;
      memset(&h,0,sizeof(h));

      h = gbm_bo_get_handle_for_plane(bois[b].bo, p);
      if (!h.u32 || h.s32 == -1) {
        uvr_utils_log(UVR_DANGER, "[x] failed to get BO plane %d gem handle (modifier 0x%" PRIx64 ")", p, bois[b].modifier);
        goto exit_uvr_buffer_gbm_bo_detroy;
      }

      bois[b].gem_handles[p] = h.u32;

      bois[b].pitches[p] = gbm_bo_get_stride_for_plane(bois[b].bo, p);
      if (!bois[b].pitches[p]) {
        uvr_utils_log(UVR_DANGER, "[x] failed to get stride/pitch for BO plane %d (modifier 0x%" PRIx64 ")", p, bois[b].modifier);
        goto exit_uvr_buffer_gbm_bo_detroy;
      }

      bois[b].offsets[p] = gbm_bo_get_offset(bois[b].bo, p);

      struct drm_prime_handle prime_request = {
        .handle = bois[b].gem_handles[p],
        .flags  = DRM_RDWR,
        .fd     = -1
      };

      /*
       * Retrieve a DMA-BUF fd (PRIME fd) for a given GEM buffer via the GEM handle.
       * This fd can be passed along to other processes
       */
      if (ioctl(bois[b].kmsFd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime_request) == -1)  {
        uvr_utils_log(UVR_DANGER, "[x] ioctl(DRM_IOCTL_PRIME_HANDLE_TO_FD): %s", strerror(errno));
        goto exit_uvr_buffer_gbm_bo_detroy;
      }

      bois[b].dma_buf_fds[p] = prime_request.fd;
    }

    /*
     * TAKEN from Daniel Stone kms-quads
     * Wrap our GEM buffer in a KMS framebuffer, so we can then attach it
     * to a plane.
     *
     * drmModeAddFB2(struct drm_mode_fb_cmd) accepts multiple image planes (not to be confused with
     * the KMS plane objects!), for images which have multiple buffers.
     * For example, YUV images may have the luma (Y) components in a
     * separate buffer to the chroma (UV) components.
     *
     * When using modifiers (which we do not for dumb buffers), we can also
     * have multiple planes even for RGB images, as image compression often
     * uses an auxiliary buffer to store compression metadata.
     *
     * Dumb buffers are always strictly single-planar, so we do not need
     * the extra planes nor the offset field.
     *
     * drmModeAddFB2WithModifiers(struct drm_mode_fb_cmd2) takes a list of modifiers per plane, however
     * the kernel enforces that they must be the same for each plane
     * which is there, and 0 for everything else.
     */
    if (uvrbuff->bType == GBM_BUFFER) {
      struct drm_mode_fb_cmd f;
      memset(&f,0,sizeof(struct drm_mode_fb_cmd));

      f.bpp    = uvrbuff->bpp;
      f.depth  = uvrbuff->bitdepth;
      f.width  = uvrbuff->width;
      f.height = uvrbuff->height;
      f.pitch  = bois[b].pitches[0];
      f.handle = bois[b].gem_handles[0];
      bois[b].fbid = 0;

      if (ioctl(bois[b].kmsFd, DRM_IOCTL_MODE_ADDFB, &f) == -1) {
        uvr_utils_log(UVR_DANGER, "[x] ioctl(DRM_IOCTL_MODE_ADDFB): %s", strerror(errno));
        goto exit_uvr_buffer_gbm_bo_detroy;
      }

      bois[b].fbid = f.fb_id;
    }

    if (uvrbuff->bType == GBM_BUFFER_WITH_MODIFIERS) {
      struct drm_mode_fb_cmd2 f;
      memset(&f,0,sizeof(struct drm_mode_fb_cmd2));

      f.width  = uvrbuff->width;
      f.height = uvrbuff->height;
      f.pixel_format = bois[b].format;
      f.flags = DRM_MODE_FB_MODIFIERS;

      memcpy(f.handles , bois[b].gem_handles, sizeof(f.handles));
      memcpy(f.pitches , bois[b].pitches    , sizeof(f.pitches));
      memcpy(f.offsets , bois[b].offsets    , sizeof(f.offsets));
      memcpy(f.modifier, uvrbuff->modifiers , sizeof(f.modifier));
      bois[b].fbid = 0;

      if (ioctl(bois[b].kmsFd, DRM_IOCTL_MODE_ADDFB2, &f) == -1) {
        uvr_utils_log(UVR_DANGER, "[x] ioctl(DRM_IOCTL_MODE_ADDFB2): %s", strerror(errno));
        goto exit_uvr_buffer_gbm_bo_detroy;
      }

      bois[b].fbid = f.fb_id;
    }
  }

  uvr_utils_log(UVR_SUCCESS, "Successfully create GBM buffers");

  return (struct uvr_buffer) { .gbmdev = gbmdev, .bufferCount = uvrbuff->bufferCount, .buffers = bois };

exit_uvr_buffer_gbm_bo_detroy:
  for (unsigned b = 0; b < uvrbuff->bufferCount; b++) {
    if (bois[b].fbid)
      ioctl(bois[b].kmsFd, DRM_IOCTL_MODE_RMFB, &bois[b].fbid);
    if (bois[b].bo)
      gbm_bo_destroy(bois[b].bo);
    for (unsigned p = 0; p < bois[b].planeCount; p++) {
      if (bois[b].dma_buf_fds[p] != -1) {
        drmCloseBufferHandle(bois[b].kmsFd, bois[b].dma_buf_fds[p]);
        bois[b].dma_buf_fds[p] = -1;
      }
    }
  }
  free(bois);
exit_uvr_buffer_gbmdev_destroy:
  if (gbmdev)
    gbm_device_destroy(gbmdev);
exit_uvr_buffer_null_struct:
  return (struct uvr_buffer) { .gbmdev = NULL, .bufferCount = 0, .buffers = NULL };
}


void uvr_buffer_destory(struct uvr_buffer_destroy *uvrbuff) {
  for (unsigned uvrb = 0; uvrb < uvrbuff->uvr_buffer_cnt; uvrb++) {
    for (unsigned b = 0; b < uvrbuff->uvr_buffer[uvrb].bufferCount; b++) {
      if (uvrbuff->uvr_buffer[uvrb].buffers[b].fbid)
        ioctl(uvrbuff->uvr_buffer[uvrb].buffers[b].kmsFd, DRM_IOCTL_MODE_RMFB, &uvrbuff->uvr_buffer[uvrb].buffers[b].fbid);
      if (uvrbuff->uvr_buffer[uvrb].buffers[b].bo)
        gbm_bo_destroy(uvrbuff->uvr_buffer[uvrb].buffers[b].bo);
      for (unsigned p = 0; p < uvrbuff->uvr_buffer[uvrb].buffers[b].planeCount; p++)  {
        if (uvrbuff->uvr_buffer[uvrb].buffers[b].dma_buf_fds[p] != -1)
          drmCloseBufferHandle(uvrbuff->uvr_buffer[uvrb].buffers[b].kmsFd, uvrbuff->uvr_buffer[uvrb].buffers[b].dma_buf_fds[p]);
      }
    }
    free(uvrbuff->uvr_buffer[uvrb].buffers);
    if (uvrbuff->uvr_buffer[uvrb].gbmdev)
      gbm_device_destroy(uvrbuff->uvr_buffer[uvrb].gbmdev);
  }
}
