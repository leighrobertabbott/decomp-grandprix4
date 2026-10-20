#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00558c3e
// Sibling of 0x00558c02/0x00558bd0: when the mode byte 0x0086857c is 2, reads three words at esi
// (esi += 6): the first record index a, an end index b and a value v. Starting at the segment
// record [0x00866ec0] + a * 0x160 it calls 0x0040ad0a (sets bit 4 of byte +0x51 and stores dl = v
// at +0x68) on b - a consecutive 0x160-byte records, wrapping by [0x00866ebc] once the walk
// reaches the ring end [0x00866eb8]. b - a is a `loop` count (zero would run 2^32 times). ebx and
// ecx are saved and restored; eax = a * 0x160 + [0x00866ec0] and edx = v are left behind and the
// flags are those of the last ring-end compare or wrap subtraction. In any other mode only
// esi += 6 happens.
void __cdecl FUN_00558c3e(gp4::Regs* r) {
    const uint32_t esi_in = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        r->esi = esi_in + 6u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi_in, 6u));
        return;
    }

    const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi_in));
    const uint32_t first = words[0];
    uint32_t count = static_cast<uint32_t>(words[1]) - first;
    const uint32_t value = words[2];
    const uint32_t start = first * 0x160u + GP4_GLOBAL(uint32_t, 0x00866ec0);

    uint32_t rec = start;
    uint32_t flags;
    do {
        gp4::Regs c = {};
        c.ebx = rec;
        c.edx = value;
        gp4::call_regs(0x0040ad0a, c);
        rec += 0x160u;
        flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866eb8));
        if (rec >= GP4_GLOBAL(uint32_t, 0x00866eb8)) {
            flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866ebc));
            rec -= GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        count -= 1u;
    } while (count != 0u);

    r->eax = start;
    r->edx = value;
    r->esi = esi_in + 6u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00558c3e, FUN_00558c3e, "regs(eax:u32, edx:u32, esi:ptr[8]:layout=words_00558c3e) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+ring_0043d30c+segs_00558d9b")
