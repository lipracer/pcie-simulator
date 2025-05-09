sgpu_ss = ss.source_set()
sgpu_ss.add(files(
    'sgpu_mmio.c',
    'sgpu_msix.c',
    'sgpu_dma.c',
    'sgpu.c',
))

system_ss.add_all(when: 'CONFIG_SGPU', if_true: sgpu_ss)
