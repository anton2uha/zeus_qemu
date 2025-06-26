zeus_ss = ss.source_set()
#zeus_ss.add(files(
#    'boot.c',
#))
#common_ss.add(when: 'CONFIG_ZEUS_VIRT', if_true: files('fw_cfg.c'))
zeus_ss.add(when: 'CONFIG_ZEUS_VIRT', if_true: files(
  'virt.c'
))

hw_arch += {'zeus': zeus_ss}
