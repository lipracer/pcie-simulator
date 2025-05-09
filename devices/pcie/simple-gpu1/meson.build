sgpu_ss = ss.source_set()
sgpu_ss.add(files(
    'sgpu.c',
    'mmio.c',
))

system_ss.add_all(when: 'CONFIG_SGPU', if_true: sgpu_ss)
