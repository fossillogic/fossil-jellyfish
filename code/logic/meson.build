dir = include_directories('.')

code_deps = [
    meson.get_compiler('c').find_library('m', required : false)
]

fossil_jellyfish_lib = library('fossil-jellyfish',
    files('jellyfish.c'),
    dependencies : [code_deps],
    install: true,
    include_directories: dir)

fossil_jellyfish_dep = declare_dependency(
    link_with: [fossil_jellyfish_lib],
    dependencies : [code_deps],
    include_directories: dir)
