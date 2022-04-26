#include "vulkan.h"
#include "xclient.h"

/*
 * "VK_LAYER_KHRONOS_validation"
 * All of the useful standard validation is
 * bundled into a layer included in the SDK
 */
const char *validation_layers[] = { };

const char *instance_extensions[] = {
  "VK_KHR_xcb_surface",
  "VK_KHR_surface"
};

struct appwhole {
  struct uvrvk app;
  struct uvrxcb xclient;
};

void appwhole_destory(struct appwhole *whole) {
  uvr_xcb_destory(&whole->xclient);
  uvr_vk_destory(&whole->app);
}

/*
 * Example code demonstrating how to connect Vulkan to X11
 */
int main(void) {
  struct appwhole whole;
  memset(&whole, 0, sizeof(struct appwhole));

  if (uvr_vk_create_instance(&whole.app, "Example App", "No Engine",
                             ARRAY_LEN(validation_layers), validation_layers,
                             ARRAY_LEN(instance_extensions), instance_extensions) == -1) {
    appwhole_destory(&whole);
    exit(1);
  }

  if (uvr_xcb_create_client(&whole.xclient, NULL, NULL, "Example App", true) == -1) {
    appwhole_destory(&whole);
    exit(1);
  }

  if (uvr_vk_create_surfaceKHR(&whole.app, XCB_CLIENT_SURFACE, "c:w", whole.xclient.conn, whole.xclient.window) == -1) {
    appwhole_destory(&whole);
    exit(1);
  }

  uvr_xcb_display_window(&whole.xclient);

  /* Wait for 5 seconds to display */
  sleep(5);

  appwhole_destory(&whole);

  return 0;
}
