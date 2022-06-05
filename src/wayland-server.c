#include "wserver.h"


struct uvr_ws_core uvr_ws_core_create(struct uvr_ws_core_create_info *uvrws) {
  if (uvrws->inc_wlr_debug_logs)
    wlr_log_init(WLR_DEBUG, NULL);

  struct uvr_ws_core core;
  memset(&core, 0, sizeof(core));

  core.display = wl_display_create();
  if (!core.display) {
    uvr_utils_log(UVR_DANGER, "[x] uvr_ws_core_create(wl_display_create): Failed to create unix socket.");
    goto exit_ws_core;
  }

  core.backend = wlr_backend_autocreate(core.display);
  if (!core.backend) {
    uvr_utils_log(UVR_DANGER, "[x] uvr_ws_core_create(wlr_backend_autocreate): Failed to create backend {DRM/KMS, Wayland, X, Headless}");
    goto exit_ws_core_wl_display_destory;
  }

  return core;

// Leave Unreachable for now
  if (core.backend)
    wlr_backend_destroy(core.backend);
exit_ws_core_wl_display_destory:
  if (core.display)
    wl_display_destroy(core.display);
exit_ws_core:
  return (struct uvr_ws_core) { .display = NULL, .backend = NULL };
}


void uvr_ws_destroy(struct uvr_ws_destroy *uvrws) {
  if (uvrws->wscore.backend)
    wlr_backend_destroy(uvrws->wscore.backend);
  if (uvrws->wscore.display)
    wl_display_destroy(uvrws->wscore.display);
}
