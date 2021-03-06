#ifndef UVR_SHADER_H
#define UVR_SHADER_H

#include "utils.h"

/*
 * struct uvr_shader_file (Underview Renderer Shader File)
 *
 * members:
 * @bytes    - Buffer that stores a given file's content
 * @byteSize - Size of buffer storing a given file's content
 */
struct uvr_shader_file {
  char *bytes;
  long byteSize;
};


/*
 * uvr_shader_file_load: Takes in a file and loads contents in to a buffer. This function is largely
 *                       only used to load shader files. struct uvr_shader_file member bytes can be
 *                       free'd with a call to uvr_shader_destroy(3).
 *
 * args:
 * @filename - Must pass path to file to load
 * return:
 *    on success struct uvr_shader_file
 *    on failure struct uvr_shader_file { with member nulled }
 */
struct uvr_shader_file uvr_shader_file_load(const char *filename);


#ifdef INCLUDE_SHADERC


#include <shaderc/shaderc.h>


/*
 * struct uvr_shader_spirv_create_info (Underview Renderer Shader SPIRV Create Information)
 *
 * members:
 * See: https://github.com/google/shaderc/blob/main/libshaderc/include/shaderc/shaderc.h#L478 for more information
 * @kind        - Used to specify what type of shader to create SPIR-V bytes from
 *                key: VkShaderStageFlagBits, value: shaderc_shader_kind
 * @source      - Pointer to a buffer containing actual shader code
 * @filename    - Used as a tag to identify the source string
 * @entryPoint  - Used to define the function name in the GLSL source that acts as an entry point for the shader
 */
struct uvr_shader_spirv_create_info {
  unsigned int kind;
  const char   *source;
  const char   *filename;
  const char   *entryPoint;
};


/*
 * struct uvr_shader_spirv (Underview Renderer Shader SPIRV)
 *
 * members:
 * @result   - An opaque handle to the results of a call to any shaderc_compile_into_*()
 *             Unfortunately we can't release until after the shader module is created.
 * @bytes    - Buffer that stores a given file's content
 * @byteSize - Size of buffer storing a given file's content
 */
struct uvr_shader_spirv {
  shaderc_compilation_result_t result;
  char                         *bytes;
  long                         byteSize;
};


/*
 * uvr_shader_compile_buffer_to_spirv: Takes in a character buffer containing shader code, it then compiles
 *                                     char buff into SPIRV-bytes at runtime. These SPIRV-bytes can later be
 *                                     passed to vulkan. struct uvr_shader_spirv member result can be free'd
 *                                     with a call to uvr_shader_destroy(3).
 *
 * args:
 * @uvrshader - Pointer to a struct uvr_shader_spirv_create_info containing infromation about what ops to do
 * return:
 *    on success struct uvr_shader_spirv
 *    on failure struct uvr_shader_spirv { with member nulled }
 */
struct uvr_shader_spirv uvr_shader_compile_buffer_to_spirv(struct uvr_shader_spirv_create_info *uvrshader);


#endif


/*
 * struct uvr_shader_destroy (Underview Renderer Shader Destroy)
 *
 * members:
 * @uvr_shader_spirv - Must pass a valid struct uvr_shader_spirv { free'd  members: shaderc_compilation_result_t handle }
 * @uvr_shader_file  - Must pass a valid struct uvr_shader_file  { free'd  members: char *bytes }
 */
struct uvr_shader_destroy {
#ifdef INCLUDE_SHADERC
  struct uvr_shader_spirv uvr_shader_spirv;
#endif
  struct uvr_shader_file  uvr_shader_file;
};


/*
 * uvr_shader_destroy: frees any allocated memory defined by customer
 *
 * args:
 * @uvrshader - pointer to a struct uvr_shader_destroy contains all objects created during
 *              application lifetime in need freeing
 */
void uvr_shader_destroy(struct uvr_shader_destroy *uvrshader);


#endif
