#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal ... popal wrapper (every GPR is restored; the flags are those of the final `shl ebx, 2`).
// Looks up the driver slot selected by the signed word [0x852d72]:
//   [0x852d7c] = (order list byte at 0x675958 + index) & 0x3f           (1-based car number, word)
//   [0xa6987c] = byte table at 0x7c2a5e, indexed by number - 1
//   [0xa69880] = name entry for the number (0x51f2af, the "michael schumacher" table)
//   [0xa69884] = dword key at 0x675970, indexed by number - 1
// FUNCTION: GP4 0x005168ce
void __cdecl FUN_005168ce(gp4::Regs* r) {
    const int32_t listIndex = static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00852d72));
    const uint32_t number = GP4_ARRAY(uint8_t, 0x00675958)[listIndex] & 0x3fu;
    GP4_GLOBAL(uint16_t, 0x00852d7c) = static_cast<uint16_t>(number);

    const int32_t slot = static_cast<int32_t>(number) - 1;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = GP4_ARRAY(uint8_t, 0x007c2a5e)[slot];

    gp4::Regs c = {};
    c.eax = number;
    gp4::call_regs(0x0051f2af, c);
    GP4_GLOBAL(uint32_t, 0x00a69880) = c.eax;

    GP4_GLOBAL(uint32_t, 0x00a69884) = GP4_ARRAY(uint32_t, 0x00675970)[slot];

    // Flags of `shl ebx, 2` with ebx = number - 1 (oracle model: AF = 0, OF = bit31(ebx<<1 ^ ebx<<2)).
    const uint32_t value = static_cast<uint32_t>(slot);
    const uint32_t result = value << 2;
    uint32_t arith = gp4::flags::szp(result, 32);
    if ((value >> 30) & 1u) arith |= gp4::flags::CF;
    if (((value << 1) ^ result) & 0x80000000u) arith |= gp4::flags::OF;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005168ce, FUN_005168ce, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958")
