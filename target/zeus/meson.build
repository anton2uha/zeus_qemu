zeus_ss = ss.source_set()
zeus_ss.add(files(
  'cpu.c',
))

gen = decodetree.process('insns.decode', extra_args: ['--insnwidth=64'])

zeus_tcg_ss = ss.source_set()
zeus_tcg_ss.add(gen)
zeus_tcg_ss.add(files(
  'cpu_tcg_ops.c',
  'insn_helpers/arithm_insn_helpers.c',
  'insn_helpers/compare_insn_helpers.c',
  'insn_helpers/branch_insn_helpers.c',
  'insn_helpers/special_insn_helpers.c',
  'translate.c',
  #'gdbstub.c',
))

zeus_softmmu_ss = ss.source_set()
zeus_softmmu_ss.add(files(
  'tlb_helper.c',
))

zeus_ss.add_all(when: 'CONFIG_TCG', if_true: [zeus_tcg_ss])

target_arch += {'metazeus': zeus_ss}
target_softmmu_arch += {'metazeus': zeus_softmmu_ss}
