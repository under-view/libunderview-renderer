
#ifndef UVR_VULKAN_H
#define UVR_VULKAN_H

#include "common.h"

#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>

typedef struct _uvrvk {
  VkInstance instance;
} uvrvk;

VkInstance uvr_vk_create_instance(const char *app_name,
                                  const char *engine_name,
                                  uint32_t enabledLayerCount,
                                  const char *ppEnabledLayerNames[],
                                  uint32_t enabledExtensionCount,
                                  const char *ppEnabledExtensionNames[]);

void uvr_vk_destory(uvrvk *app);

#endif
