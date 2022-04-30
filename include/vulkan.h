#ifndef UVR_VULKAN_H
#define UVR_VULKAN_H

#include "common.h"
#include "utils.h"

#ifdef INCLUDE_WAYLAND
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif

#ifdef INCLUDE_XCB
#define VK_USE_PLATFORM_XCB_KHR
#endif

#include <vulkan/vulkan.h>


/*
 * Due to Vulkan not directly exposing functions for all platforms.
 * Dynamically (at runtime) retrieve or acquire the address of a VkInstance func.
 * Via token concatenation and String-izing Tokens
 */
#define UVR_VK_INSTANCE_PROC_ADDR(inst, var, func) \
  do { \
    var = (PFN_vk##func) vkGetInstanceProcAddr(inst, "vk" #func); \
  } while(0);


/*
 * Due to Vulkan not directly exposing functions for all platforms.
 * Dynamically (at runtime) retrieve or acquire the address of a VkDevice (logical device) func.
 * Via token concatenation and String-izing Tokens
 */
#define UVR_VK_DEVICE_PROC_ADDR(dev, var, func) \
  do { \
    var = (PFN_vk##func) vkGetDeviceProcAddr(dev, "vk" #func); \
  } while(0);


/*
 * struct uvrvk_instance (Underview Renderer Vulkan Instance)
 *
 * members:
 * @app_name                - A member of the VkApplicationInfo structure reserved for
 *                            the name of the application.
 * @engine_name             - A member of the VkApplicationInfo structure reserved for
 *                            the name of the engine used to create a given application.
 * @enabledLayerCount       - A member of the VkInstanceCreateInfo structure used to pass
 *                            the number of Vulkan Validation Layers one wants to enable.
 * @ppEnabledLayerNames     - A member of the VkInstanceCreateInfo structure used to pass
 *                            the actual Vulkan Validation Layers one wants to enable.
 * @enabledExtensionCount   - A member of the VkInstanceCreateInfo structure used to pass
 *                            the the number of vulkan instance extensions one wants to enable.
 * @ppEnabledExtensionNames - A member of the VkInstanceCreateInfo structure used to pass
 *                            the actual vulkan instance extensions one wants to enable.
 */
struct uvrvk_instance {
  const char *app_name;
  const char *engine_name;
  uint32_t enabledLayerCount;
  const char **ppEnabledLayerNames;
  uint32_t enabledExtensionCount;
  const char **ppEnabledExtensionNames;
};


/*
 * uvr_vk_create_instance: Creates a VkInstance object and establishes a connection to the Vulkan API.
 *                         It also acts as an easy wrapper that allows one to define instance extensions.
 *                         Instance extensions basically allow developers to define what an app is setup to do.
 *                         So, if I want the application to work with wayland or X etc…​ One should enable those
 *                         extensions inorder to gain access to those particular capabilities.
 *
 * args:
 * @uvrvk - pointer to a struct uvrvk_instance
 * return:
 *    VkInstance handle on success
 *    VK_NULL_HANDLE on failure
 */
VkInstance uvr_vk_instance_create(struct uvrvk_instance *uvrvk);


/*
 * enum uvrvk_surface_type (Underview Renderer Surface Type)
 *
 * Display server protocol options used by uvr_vk_surface_create
 * to create a VkSurfaceKHR object based upon platform specific information
 */
typedef enum uvrvk_surface_type {
  WAYLAND_CLIENT_SURFACE,
  XCB_CLIENT_SURFACE,
} uvrvk_surface_type;


/*
 * struct uvrvk_surface (Underview Renderer Vulkan Surface)
 *
 * members:
 * @vkinst   - Must pass a valid VkInstance handle to create/associate surfaces for an application
 * @sType    - Must pass a valid enum uvrvk_surface_type value. Used in determine what vkCreate*SurfaceKHR
 *             function and associated structs to use.
 * @surface  - Must pass a pointer to a wl_surface interface
 * @display  - Must pass either a pointer to wl_display interface or a pointer to an xcb_connection_t
 * @window   - Must pass an xcb_window_t window or an unsigned int representing XID
 */
struct uvrvk_surface {
  VkInstance vkinst;
  uvrvk_surface_type sType;
  void *surface;
  void *display;
  unsigned int window;
};


/*
 * uvr_vk_surface_create: Creates a VkSurfaceKHR object based upon platform specific
 *                        information about the given surface
 *
 * args:
 * @uvrvk - pointer to a struct uvrvk_surface
 * return:
 *    VkSurfaceKHR handle on success
 *    VK_NULL_HANDLE on failure
 */
VkSurfaceKHR uvr_vk_surface_create(struct uvrvk_surface *uvrvk);


/*
 * struct uvrvk_phdev (Underview Renderer Vulkan Physical Device)
 *
 * members:
 * @vkinst   - Must pass a valid VkInstance handle to create/associate surfaces for an application
 * @vkpdtype - Must pass a valid enum uvrvk_surface_type value.
 * @drmfd    - Must pass a valid kms file descriptor for which a VkPhysicalDevice will be created
 *             if corresponding properties match
 */
struct uvrvk_phdev {
  VkInstance vkinst;
  VkPhysicalDeviceType vkpdtype;
#ifdef INCLUDE_KMS
  int drmfd;
#endif
};


/*
 * uvr_vk_phdev_create: Creates a VkPhysicalDevice handle if certain characteristics of
 *                      a physical device are meet
 *
 * args:
 * @uvrvk - pointer to a struct uvrvk_phdev
 * return:
 *    VkPhysicalDevice handle on success
 *    VK_NULL_HANDLE on failure
 */
VkPhysicalDevice uvr_vk_phdev_create(struct uvrvk_phdev *uvrvk);


/*
 * struct uvrvk_destroy (Underview Renderer Vulkan Destroy)
 *
 * members:
 * @vkinst_cnt  - The amount of VkInstance handles allocated for a given application
 * @vkinsts     - Must pass an array of valid VkInstance handles
 * @vklgdev_cnt - The amount of VkDevice handles allocated for a given application
 * @vklgdevs    - Must pass an array of valid VkDevice handles
 * @vksurf_cnt  - The amount of VkSurfaceKHR handles allocated for a given application
 * @vksurfs     - struct that stores the associated VkInstance used to create a VkSurfaceKHR
 *              + @vkinst - Must pass valid VkInstance handle
 *              + @vksurf - Must pass valid VkSurfaceKHR handle
 */
struct uvrvk_destroy {
  int vkinst_cnt;
  VkInstance *vkinsts;

  int vklgdev_cnt;
  VkDevice *vklgdevs;

  int vksurf_cnt;
  struct vkinstsurfs {
    VkInstance vkinst;
    VkSurfaceKHR vksurf;
  } vksurfs[WINDOW_CNT];
};


/*
 * uvr_vk_destory: frees any allocated memory defined by user
 *
 * args:
 * @uvrvk - pointer to a struct uvrvk_destroy contains all objects created during
 *          application lifetime in need freeing
 */
void uvr_vk_destory(struct uvrvk_destroy *uvrvk);

#endif
