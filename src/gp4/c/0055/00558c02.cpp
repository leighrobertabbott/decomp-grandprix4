#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sibling of 0x558bd0: when the mode byte 0x86857c is 2, reads a 16-bit count and a second word at
// esi (esi advances past both; the word goes zero-extended to edx and to [0x6020ac]), then sets
// bit 0x40 of byte +0x64 in `count` consecutive 0x160-byte segment records from ebx (do-while on
// ax, so a count of 0 runs 65536 times), wrapping by the ring wrap length [0x866ebc] once ebx
// reaches the ring end [0x866eb8]. Leaves ax = 0; the flags are those of the last `dec ax`
// (CF kept from the last cmp / sub). In any other mode only esi += 4 happens.
// FUNCTION: GP4 0x00558c02
void __cdecl FUN_00558c02(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2) {
        const uint32_t before = r->esi;
        r->esi = before + 4u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 4u));
        return;
    }
    const uint16_t* cursor = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    uint16_t count = cursor[0];
    const uint32_t word = cursor[1];
    GP4_GLOBAL(uint32_t, 0x006020ac) = word;
    uint32_t rec = r->ebx;
    uint32_t flags;
    do {
        GP4_FIELD(uint8_t, rec, 0x64) |= 0x40;
        flags = gp4::flags::add32(rec, 0x160u);
        rec += 0x160u;
        flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866eb8));
        if (rec >= GP4_GLOBAL(uint32_t, 0x00866eb8)) {
            flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866ebc));
            rec -= GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        flags = gp4::flags::dec16(count, flags);
        count = static_cast<uint16_t>(count - 1u);
    } while (count != 0);
    r->eax = r->eax & 0xffff0000u;
    r->edx = word;
    r->esi += 4u;
    r->ebx = rec;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00558c02, FUN_00558c02, "regs(eax:u32, edx:u32, esi:ptr[8]:u16[0..40], ebx:rec[0x8943a0,0x160,0..29]) -> (eax, edx, esi, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0")
