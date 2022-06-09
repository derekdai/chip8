src = ['chip8.c',
       'ui.c',
       'sdlui.c',
       'termui.c']

libchip8 = library('chip8',
                   src,
                   dependencies: sdl2_dep,
                   include_directories: inc)

executable('chip8',
           'main.c',
           link_with: libchip8,
           include_directories: inc)
