
if libxcb.found() and libxcbewmh.found()
  executable('underview-renderer-xcb-client-triangle',
             'triangle.c',
             link_with: lib_underview_renderer,
             dependencies: lib_uvr_deps,
             include_directories: [inc],
             c_args: pargs,
             install: false)
endif
