dir = include_directories('.')

code_deps = [
    dependency('threads'),
    meson.get_compiler('c').find_library('m', required : false)
]

fossil_x_lib = library('fossil-x',
    files('src.c'),
    dependencies : [code_deps],
    install: true,
    include_directories: dir)

fossil_x_dep = declare_dependency(
    link_with: [fossil_x_lib],
    dependencies : [code_deps],
    include_directories: dir)
