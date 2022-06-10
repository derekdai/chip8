src = ['chip8.c',
       'ui.c',
       'sdlui.c',
       'termui.c']

if get_option('enable-dtrace')
  gen_sdt_header = generator(
    dtrace,
    output: '@BASENAME@.h',
    arguments: ['-s', '@INPUT@', '-h', '-o', '@OUTPUT@']
  )

  gen_sdt_object = generator(
    dtrace,
    output: '@BASENAME@.o',
    arguments: ['-s', '@INPUT@', '-G', '-o', '@OUTPUT@']
  )

  chip8_sdt_h = gen_sdt_header.process('chip8-sdt.d')
  chip8_sdt_o = gen_sdt_object.process('chip8-sdt.d')
  src += chip8_sdt_h
  src += chip8_sdt_o
endif

libchip8 = library('chip8',
                   src,
                   dependencies: sdl2_dep,
                   include_directories: inc)

executable('chip8',
           'main.c',
           link_with: libchip8,
           include_directories: inc)
