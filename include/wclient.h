
#ifndef UVR_WAYLAND_CLIENT_H
#define UVR_WAYLAND_CLIENT_H

#include "common.h"
#include <wayland-client.h>

/*
 * typedef struct _uvrwc uvrwc (Underview Renderer Wayland Client)
 *
 * @wl_display_fd - file descriptor associated with a given display
 * @display       - a global singleton representing the whole connection
 *                  to wayland server.
 * @registry      - a global singleton used to query information about server
 *                  state/properties and allocate other objects/interfaces
 * @compositor    - a global singleton representing the compositor itself
 *                  largely utilized to allocate surfaces & regions to put
 *                  pixels into
 * @shell -
 * @xdg_toplevel  -
 * @shm_fd        - A file descriptor to an open shm object
 * @shm           - Shared memory, used to transfer a file descriptor for the
 *                  compositor to mmap.
 * @shm_pool      - object used to encapsulate a piece of memory shared between the
 *                  compositor and client. @buffers (wl_buffer) can be created from
 *                  pool.
 * @shm_pool_size - Size of the amount of bytes to that contains pixels
 *                  width * height * bytes_per_pixel * @buffer_count
 * @shm_pool_data - Actual pixel data to be displayed
 * @buffer_count  - The amount of buffers contained in a given memory pool
 * @buffer        -
 * @surface       -
 * @xdg_surface   -
 */
typedef struct _uvrwc {
  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *compositor;

  struct xdg_wm_base *shell;
  struct xdg_toplevel *xdg_toplevel;

  int shm_fd;
  struct wl_shm *shm;
  struct wl_shm_pool *shm_pool;
  size_t shm_pool_size;
  uint8_t *shm_pool_data;

  int buffer_count;
  struct wl_buffer **buffers;
  struct wl_surface *surface;
  struct xdg_surface *xdg_surface;
} uvrwc;


/*
 * uvr_wclient_display_connect: Establish connection to Wayland display server
 * args:
 * @wl_display_name - Specify the Wayland display to run a client on. This will set
 *                    the $WAYLAND_DISPLAY variable to the desired display.
 * return:
 *    A reference to a wl_display on success
 *    NULL on failure
 */
struct wl_display *uvr_wclient_display_connect(const char *wl_display_name);


/*
 * uvr_wclient_get_interfaces: Sets up global objects/interfaces.
 * args:
 * @client - pointer to a struct _uvrwc contains all objects/interfaces
 *           necessary for a client to run.
 * return:
 *    0 on success
 *   -1 on failure
 */
int uvr_wclient_alloc_interfaces(uvrwc *client);


/*
 * uvr_wclient_alloc_shm_buffers: Adds way to get pixels from client to compositor
 *                                by creating writable shared buffers.
 * args:
 * @client - pointer to a struct _uvrwc contains all objects/interfaces
 *           necessary for a client to run.
 * @buffer_count - the amount of buffers to allocate when storing pixel data
 *                 2 is generally a good option as it allows for double buffering
 * @width - pixel width (i.e 3840, 1920, ...)
 * @height - pixel height (i.e 2160, 1080, ...)
 * @bytes_per_pixel - The amount of bytes per pixel generally going to 4 bytes (32 bits)
 * @format - memory layout of an individual pixel
 * return:
 *    0 on success
 *   -1 on failure
 */
int uvr_wclient_alloc_shm_buffers(uvrwc *client,
                                  const int buffer_count,
                                  const int width,
                                  const int height,
                                  const int bytes_per_pixel,
                                  uint64_t format);


/*
 * uvr_wclient_process_events: Processes incoming Wayland server events.
 *                             Best utilized with epoll(POLLIN)
 * args:
 * @client - pointer to a struct _uvrwc contains all objects/interfaces
 *           necessary for a client to run.
 * return:
 *    -1 on failure
 */
int uvr_wclient_process_events(uvrwc *client);


/*
 * uvr_wclient_flush_request: Flush all outgoing request to a Wayland server.
 * args:
 * @client - pointer to a struct _uvrwc contains all objects/interfaces
 *           necessary for a client to run.
 * return:
 *    -1 on failure
 */
int uvr_wclient_flush_request(uvrwc *client);


void uvr_wclient_destory(uvrwc *client);

#endif
