#include <gp4/gp4.h>
#include <gp4/flags.h>

// Starts the sound sample 0x15 (a looping effect) when the record ebx asks for it. Bit 8 of the dword +4 of the
// record selects it (clear: nothing happens, flags of that test). Otherwise edx becomes 0x14000 when bit 0 of the
// dword +0xa0 is set and 0 when not. The sound system must be enabled ([0x007ad868] nonzero) and unmuted
// (byte [0x00852d6a] zero); 0x0051f27f then tells with CF whether sample 0x15 is loaded (the first dword of its
// 0x1c-byte entry is nonzero). Loaded: the channel request cells are filled ([0x0088fbac] = 0, [0x0088f6e0] = 0xff,
// [0x0088f6d0] = [0x0088f6f0] * 2, [0x0088f6d4] = 0x100000, [0x0088f6d8] = edx, or 0xa000 when the byte [0x007ab185]
// is not 0xff) and 0x0051f263 starts the channel. eax is pushed and popped, so only edx and the flags change; the
// flags are those of the last flag writer on the path taken.
// FUNCTION: GP4 0x0051ace6
void __cdecl FUN_0051ace6(gp4::Regs* r) {
    const uint32_t record = r->ebx;

    const uint32_t request = GP4_FIELD(uint32_t, record, 4) & 0x100u;
    if (request == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(request));
        return;
    }

    uint32_t edx = 0u;
    if ((GP4_FIELD(uint32_t, record, 0xa0) & 1u) != 0u) {
        edx = 0x14000u;
    }
    r->edx = edx;

    const uint32_t enabled = GP4_GLOBAL(uint8_t, 0x007ad868);
    if (enabled == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(enabled, 0u));
        return;
    }

    const uint32_t muted = GP4_GLOBAL(uint8_t, 0x00852d6a);
    if (muted != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(muted));
        return;
    }

    GP4_GLOBAL(uint32_t, 0x0088f6cc) = 0x15u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = 0x15u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(0xffu, 0u));
    gp4::call_regs(0x0051f27f, c);
    if ((c.eflags & gp4::CF) == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, c.eflags);
        return;
    }

    GP4_GLOBAL(uint32_t, 0x0088fbac) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088f6e0) = 0xffu;
    GP4_GLOBAL(uint32_t, 0x0088f6d0) = GP4_GLOBAL(uint32_t, 0x0088f6f0) << 1;
    GP4_GLOBAL(uint32_t, 0x0088f6d4) = 0x100000u;

    const uint32_t selector = GP4_GLOBAL(uint8_t, 0x007ab185);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(selector, 0xffu));
    c.eax = (selector == 0xffu) ? edx : 0xa000u;
    GP4_GLOBAL(uint32_t, 0x0088f6d8) = c.eax;
    gp4::call_regs(0x0051f263, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051ace6, FUN_0051ace6, "regs(ebx:ptr[0xa8]:bytes) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=chan_005188f5+chan_00518654+gate_0051ae76+full_00518b09+snd_00518758+snd_0051ace6")
