#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sibling of the 0x556e..0x558c.. segment-ring command handlers. When the mode byte 0x86857c is 2
// it reads a 16-bit word w at esi (esi advances by 2), forms the offset edx = (w * 0x160) & 0xffff
// (16-bit imul, zero-extended) and stores al (the low byte of w) into a segment record:
//   cl == 0x11: record = ebx + edx, wrapped down by the ring wrap length [0x866ebc] once it reaches
//               the ring end [0x866eb8]; the byte goes to record + 0x17;
//   otherwise : record = ebx - edx, wrapped up by [0x866ebc] when it is below the ring start
//               [0x866ec0]; the byte goes to record + 0x16.
// ebx keeps the wrapped record, ax holds w. The flags are those of the last cmp / sub / add
// (`add esi, 2` alone in any other mode).
// FUNCTION: GP4 0x00556e7c
void __cdecl FUN_00556e7c(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2) {
        const uint32_t before = r->esi;
        r->esi = before + 2u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 2u));
        return;
    }
    const uint16_t word = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t offset = static_cast<uint16_t>(word * 0x160u);
    uint32_t rec = r->ebx;
    uint32_t flags;
    if ((r->ecx & 0xffu) == 0x11u) {
        rec += offset;
        flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866eb8));
        if (rec >= GP4_GLOBAL(uint32_t, 0x00866eb8)) {
            flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866ebc));
            rec -= GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        GP4_FIELD(uint8_t, rec, 0x17) = static_cast<uint8_t>(word);
    } else {
        rec -= offset;
        flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866ec0));
        if (rec < GP4_GLOBAL(uint32_t, 0x00866ec0)) {
            flags = gp4::flags::add32(rec, GP4_GLOBAL(uint32_t, 0x00866ebc));
            rec += GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        GP4_FIELD(uint8_t, rec, 0x16) = static_cast<uint8_t>(word);
    }
    r->eax = (r->eax & 0xffff0000u) | word;
    r->edx = offset;
    r->esi += 2u;
    r->ebx = rec;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00556e7c, FUN_00556e7c, "regs(eax:u32, ecx:u32{0x11,0x10,0x12,0}, ebx:rec[0x8943a0,0x160,0..29], esi:ptr[8]:u16[0..31]) -> (eax, edx, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+ring_0043d30c+end_00556e7c")
