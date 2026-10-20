#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00558c9a
// Sibling of 0x558c02. When the mode byte 0x86857c is 2, reads three words at esi (esi advances past
// all three): the first record index, the last one and a value kept for the callee. Starting at the
// 0x160-byte segment record [0x866ec0] + first * 0x160 it calls 0x40ad12 (last - first) times, one
// record each (record +0x51 |= 2, record +0x69 = the value), moving on by 0x160 and wrapping by
// [0x866ebc] once the pointer reaches the ring end [0x866eb8] (do-while: a count of 0 runs 2^32 times).
// eax = the first record's address, edx = the third word; ebx and ecx are saved and restored. The flags
// are those of the last cmp or sub. In any other mode only esi += 6 happens.
void __cdecl FUN_00558c9a(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2) {
        const uint32_t before = r->esi;
        r->esi = before + 6u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 6u));
        return;
    }
    const uint16_t* cursor = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t first = cursor[0];
    const uint32_t last = cursor[1];
    const uint32_t word = cursor[2];
    uint32_t count = last - first;
    const uint32_t start = first * 0x160u + GP4_GLOBAL(uint32_t, 0x00866ec0);

    uint32_t rec = start;
    uint32_t flags;
    do {
        gp4::Regs c = {};
        c.ebx = rec;
        c.edx = word;
        gp4::call_regs(0x0040ad12, c);

        rec += 0x160u;
        flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866eb8));
        if (rec >= GP4_GLOBAL(uint32_t, 0x00866eb8)) {
            flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866ebc));
            rec -= GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        --count;
    } while (count != 0u);

    r->eax = start;
    r->edx = word;
    r->esi += 6u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00558c9a, FUN_00558c9a, "regs(esi:ptr[8]:layout=words_00558c3e) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+ring_0043d30c+mode_0086857c")
