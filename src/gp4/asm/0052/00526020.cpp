#include <gp4/gp4.h>
#include <gp4/flags.h>

// Loads the four-dword viewport descriptor chosen by the mode [0x874f48] (pointer table of 8-byte entries at
// 0x874f9c) into 0x874f34..0x874f40, mirrors the two screen dwords 0x7a8ea2/0x7a8ea6 into 0x875188/0x87518c
// and scales each descriptor dword by (screen << 14) / 640 (x entries 0x874f34, 0x874f3c) or / 480 (y entries
// 0x874f38, 0x874f40) in 18.14 fixed point, rounded (+2) down to a multiple of 4:
// 0x875194, 0x87519c, 0x875198, 0x8751a0. pushal/popal restore every register; the flags are those of the last
// `and eax, 0xfffffffc` (the idiv leaves them untouched in the oracle).
// FUNCTION: GP4 0x00526020
void __cdecl FUN_00526020(gp4::Regs* r) {
    const uint32_t mode = GP4_GLOBAL(uint32_t, 0x00874f48);
    const uint32_t* source = reinterpret_cast<const uint32_t*>(GP4_GLOBAL(uint32_t, 0x00874f9cu + mode * 8u));
    uint32_t* block = GP4_ARRAY(uint32_t, 0x00874f34);
    for (uint32_t i = 0u; i < 4u; ++i) {
        block[i] = source[i];
    }

    GP4_GLOBAL(uint32_t, 0x00875188) = GP4_GLOBAL(uint32_t, 0x007a8ea2);
    GP4_GLOBAL(uint32_t, 0x0087518c) = GP4_GLOBAL(uint32_t, 0x007a8ea6);

    uint32_t remainder;
    const int32_t scale_x = gp4::idiv64(static_cast<int64_t>(GP4_GLOBAL(uint32_t, 0x007a8ea2) << 14), 0x280, reinterpret_cast<int32_t*>(&remainder));
    const int32_t scale_y = gp4::idiv64(static_cast<int64_t>(GP4_GLOBAL(uint32_t, 0x007a8ea6) << 14), 0x1e0, reinterpret_cast<int32_t*>(&remainder));

    uint32_t value;
    value = static_cast<uint32_t>((static_cast<int64_t>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00874f34))) * scale_x) >> 14);
    value = (value + 2u) & 0xfffffffcu;
    GP4_GLOBAL(uint32_t, 0x00875194) = value;

    value = static_cast<uint32_t>((static_cast<int64_t>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00874f38))) * scale_y) >> 14);
    value = (value + 2u) & 0xfffffffcu;
    GP4_GLOBAL(uint32_t, 0x0087519c) = value;

    value = static_cast<uint32_t>((static_cast<int64_t>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00874f3c))) * scale_x) >> 14);
    value = (value + 2u) & 0xfffffffcu;
    GP4_GLOBAL(uint32_t, 0x00875198) = value;

    value = static_cast<uint32_t>((static_cast<int64_t>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00874f40))) * scale_y) >> 14);
    value = (value + 2u) & 0xfffffffcu;
    GP4_GLOBAL(uint32_t, 0x008751a0) = value;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
}
GP4_IMPL(0x00526020, FUN_00526020, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_00526020")
