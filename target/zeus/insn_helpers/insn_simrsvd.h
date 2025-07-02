/**@file
 * @brief Instruction SIMRSVD fields
 * @copyright 2024 Anton Lesik
 *
 *
 *
 */
#pragma once

#ifdef __cplusplus
#include <cstdint>
namespace isa {
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
enum class SimRsvdCmd : uint8_t {
    EXIT = 0,
    ASSERT,
    PRINT,
};
#else
enum SimRsvdCmd {
    SIMRSVD_CMD_EXIT = 0,
    SIMRSVD_CMD_ASSERT,
    SIMRSVD_CMD_PRINT,
};
#endif

#ifdef __cplusplus
enum class SimRsvdExitCmd : uint8_t {
    SIMULATION = 0,
};
#else
enum SimRsvdExitCmd {
    SIMRSVD_EXIT_SIMULATION = 0,
};
#endif

#ifdef __cplusplus
enum class SimRsvdAssertCmd : uint8_t {
    REGS_EQ = 0,
};
#else
enum SimRsvdAssertCmd {
    SIMRSVD_ASSERT_REGS_EQ = 0,
};
#endif

#ifdef __cplusplus
enum class SimRsvdPrintCmd : uint8_t {
    REG_GPR = 0,
};
#else
enum SimRsvdPrintCmd {
    SIMRSVD_PRINT_REG_GPR = 0,
};
#endif

typedef struct __attribute__((packed)) InsnSimRsvd {

    uint8_t cmd : 4;
    uint8_t subcmd : 4;

    union {
        struct __attribute__((packed)) {
            uint8_t r1 : 8;
            uint8_t r2 : 8;
        } assert_regs_eq;
        struct __attribute__((packed)) {
            uint8_t rid : 8;
        } print_reg_gpr;

    };

} InsnSimRsvd;

#ifdef __cplusplus
} // namespace isa
#endif
