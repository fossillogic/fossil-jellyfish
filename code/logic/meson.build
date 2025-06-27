dir = include_directories('.')

fossil_fish_lib = static_library('fossil-jellyfish',
    files('jellyfish.c'),
    install: true,
    include_directories: dir)

fossil_fish_dep = declare_dependency(
    link_with: [fossil_fish_lib],
    include_directories: dir)
