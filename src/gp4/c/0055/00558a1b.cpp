#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sibling of 0x558aca/0x558b10/0x558bd0/0x558c02. When the mode byte 0x86857c is 2, reads three
// words at esi (esi advances past all of them): a selector (0x8000 -> record offset 0xb4,
// anything else -> 0xb5), a count and a value. The value (zero-extended to eax) is stored as a
// byte at record + offset in `count` consecutive 0x160-byte segment records starting at ebx,
// wrapping by the ring wrap length [0x866ebc] once ebx reaches the ring end [0x866eb8]. ebx is
// saved and restored; ecx ends at 0 and edx holds the offset. The flags are those of the last
// `dec ecx` (CF kept from the last cmp / sub), or of `or ecx, ecx` for an empty walk. In any other
// mode only esi += 6 happens (flags of that add).
// FUNCTION: GP4 0x00558a1b
void __cdecl FUN_00558a1b(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2) {
        const uint32_t before = r->esi;
        r->esi = before + 6u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 6u));
        return;
    }
    const uint16_t* cursor = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t offset = (cursor[0] == 0x8000u) ? 0xb4u : 0xb5u;
    uint32_t count = cursor[1];
    const uint32_t value = cursor[2];
    uint32_t rec = r->ebx;
    uint32_t flags = gp4::flags::logic32(count);
    while (count != 0) {
        GP4_FIELD(uint8_t, rec, offset) = static_cast<uint8_t>(value);
        rec += 0x160u;
        flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866eb8));
        if (rec >= GP4_GLOBAL(uint32_t, 0x00866eb8)) {
            flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866ebc));
            rec -= GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        flags = gp4::flags::dec32(count, flags);
        --count;
    }
    r->eax = value;
    r->ecx = 0;
    r->edx = offset;
    r->esi += 6u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00558a1b, FUN_00558a1b, "regs(esi:ptr[8]:u16{0x8000,0,1,2,3,7,12,25,40}, ebx:rec[0x8943a0,0x160,0..29]) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0")
