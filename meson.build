project('chip8', 'c', default_options: ['c_std=c11'], version: '0.0.1')

inc = include_directories(['.', 'include'], is_system: false)
sdl2_dep = dependency('sdl2')

conf = configuration_data({
  'LOG_LEVEL': '@0@'.format(get_option('log_level')),
})
configure_file(output: 'config.h', configuration: conf)

subdir('src')
subdir('tests')
subdir('examples')
