import os
import os.path

top = '.'
out = 'build'

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    import os
    os.environ.setdefault('CC', 'clang')
    os.environ.setdefault('CXX', 'clang++')
    ctx.load('pebble_sdk')

def build(ctx):
    ctx.load('pebble_sdk')

    use_demo = os.environ.get('DEMO_DATA')
    demo_state = os.environ.get('DEMO_STATE', '2')

    build_worker = os.path.exists('worker_src')
    binaries = []
    cached_env = ctx.env
    for platform in ctx.env.TARGET_PLATFORMS:
        ctx.env = ctx.all_envs[platform]
        # DEMO_DATA=1 [DEMO_STATE=N] pebble build — injects fake data for QA
        # Demo sources live in src/c/demo/ and are excluded from release builds.
        if use_demo:
            ctx.env.append_value('CFLAGS',
                ['-DDEMO_DATA', '-DDEMO_STATE=' + demo_state])
            c_sources = ctx.path.ant_glob('src/c/**/*.c')
        else:
            c_sources = ctx.path.ant_glob('src/c/**/*.c',
                                          excl=['src/c/demo/**'])
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app_elf = '{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pbl_build(source=c_sources,
                      target=app_elf, bin_type='app')
        if build_worker:
            worker_elf = '{}/pebble-worker.elf'.format(ctx.env.BUILD_DIR)
            binaries.append({'platform': platform, 'app_elf': app_elf,
                             'worker_elf': worker_elf})
            ctx.pbl_build(source=ctx.path.ant_glob('worker_src/c/**/*.c'),
                          target=worker_elf, bin_type='worker')
        else:
            binaries.append({'platform': platform, 'app_elf': app_elf})
    ctx.env = cached_env
    ctx.set_group('bundle')
    ctx.pbl_bundle(binaries=binaries,
                   js=ctx.path.ant_glob(['src/pkjs/**/*.js',
                                         'src/pkjs/**/*.json']),
                   js_entry_file='src/pkjs/index.js')