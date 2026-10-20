#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Writes a run of header bytes into consecutive 0x160-byte ring records. When the mode byte
// [0x86857c] is not 2 it only advances esi by 2 (the flags are those of that add). Otherwise it
// bumps the slot counter [0x874d2c] (index + 1 must be below 0x10, else int3), stores the low byte
// of [0x874d28] at 0x874d17[old counter], reads the word count at [esi] (esi += 2) and writes the
// byte (new counter | 0x80) at +0x6a of that many records, starting at ebx and stepping +0x160 with
// the wrap `if (ebx >= [0x866eb8]) ebx -= [0x866ebc]`. edx = new counter | 0x80, ax ends at 0
// (a zero count loops 65536 times), the upper half of eax stays that of [0x874d28]; the flags are
// those of the last `dec ax` (CF kept from the last compare/subtract of the wrap).
// FUNCTION: GP4 0x005582a7
void __cdecl FUN_005582a7(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(r->esi, 2u));
        r->esi += 2u;
        return;
    }
    uint32_t slot = GP4_GLOBAL(uint32_t, 0x00874d2c);
    slot += 1u;
    if (slot >= 0x10u) {
        __debugbreak();
    }
    GP4_GLOBAL(uint32_t, 0x00874d2c) = slot;
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00874d28);
    GP4_ARRAY(uint8_t, 0x00874d16)[slot] = static_cast<uint8_t>(value);
    const uint8_t mark = static_cast<uint8_t>(slot) | 0x80u;

    uint32_t esi = r->esi;
    uint16_t count = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi));
    esi += 2u;
    uint32_t ebx = r->ebx;
    uint32_t arith;
    do {
        GP4_FIELD(uint8_t, ebx, 0x6a) = mark;
        ebx += 0x160u;
        arith = gp4::flags::sub32(ebx, GP4_GLOBAL(uint32_t, 0x00866eb8));
        if ((arith & gp4::flags::CF) == 0u) {
            const uint32_t wrap = GP4_GLOBAL(uint32_t, 0x00866ebc);
            arith = gp4::flags::sub32(ebx, wrap);
            ebx -= wrap;
        }
        arith = gp4::flags::dec16(count, arith);
        count = static_cast<uint16_t>(count - 1u);
    } while (count != 0u);

    r->eax = (value & 0xffff0000u) | count;
    r->ebx = ebx;
    r->edx = (slot & 0xffffff00u) | mark;
    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005582a7, FUN_005582a7, "regs(esi:ptr[8]:i16[1..24], ebx:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+slot_005582a7")
