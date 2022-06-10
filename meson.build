project('chip8', 'c', default_options: ['c_std=c11'], version: '0.0.1')

cc = meson.get_compiler('c')

inc = include_directories(['.', 'include'], is_system: false)
sdl2_dep = dependency('sdl2')

conf = configuration_data({
  'LOG_LEVELS': '@0@'.format(get_option('log-level')),
  'ENABLE_DTRACE': get_option('enable-dtrace'),
})

if get_option('enable-dtrace')
  cc.check_header('sys/sdt.h', required: true)
  dtrace = find_program('dtrace', required: true)
endif

configure_file(output: 'config.h', configuration: conf)

subdir('src')
subdir('tests')
subdir('examples')
