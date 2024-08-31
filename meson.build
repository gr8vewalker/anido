project(
    'anido',
    'c',
    version: run_command('version.sh', 'get-vcs').stdout().strip(),
)

meson.add_dist_script('version.sh', 'set-dist', meson.project_version())

add_project_arguments(
    '-DANIDO_VERSION="@0@"'.format(meson.project_version()),
    '-DLOG_USE_COLOR',
    language: 'c',
)

subdir('src')

libanim_proj = subproject('libanim')
libanim_dep = libanim_proj.get_variable('libanim_dep')

deps = [libanim_dep]

executable('anido', sources, dependencies: deps)
