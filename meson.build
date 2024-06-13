project('cache-latency', 'c',
  version : '0.1',
  default_options : [
    'warning_level=3',
    'buildtype=release',
    'c_std=c23',
  ])

exe = executable('cache-latency', 'src/cache_latency.c', c_args : ['-march=native'],)
