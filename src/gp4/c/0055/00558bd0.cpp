#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sibling of 0x558aca: reads a 16-bit count at esi (advancing esi past it) and,
// when the mode byte 0x86857c is 2, sets bit 1 of byte +0x70 in that many
// consecutive 0x160-byte segment records starting at ebx, wrapping by the ring
// wrap length [0x866ebc] once ebx reaches the ring end [0x866eb8]. Leaves
// ecx = 0; the flags are those of the last flag writer (add esi,2 / or ecx,ecx /
// the last cmp or sub).

// FUNCTION: GP4 0x00558bd0
void __cdecl FUN_00558bd0(gp4::Regs* r) {
    const uint16_t* cursor = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t rec = r->ebx;
    uint32_t flags;

    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2) {
        r->esi += 2;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(r->esi - 2, 2));
        return;
    }
    uint32_t count = *cursor;
    r->esi += 2;
    flags = gp4::flags::logic32(count);
    while (count != 0) {
        GP4_FIELD(uint8_t, rec, 0x70) |= 2;
        rec += 0x160;
        flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866eb8));
        if (rec >= GP4_GLOBAL(uint32_t, 0x00866eb8)) {
            flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866ebc));
            rec -= GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        --count;
    }
    r->ecx = 0;
    r->ebx = rec;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00558bd0, FUN_00558bd0, "regs(esi:ptr[8]:u16[0..40], ebx:rec[0x8943a0,0x160,0..29]) -> (esi, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0")
