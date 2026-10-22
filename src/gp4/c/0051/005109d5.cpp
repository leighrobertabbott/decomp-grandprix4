#include <gp4/gp4.h>
#include <gp4/flags.h>

// Snapshots the active entries of the 0x140 x 0xe0-byte part array at 0x00854414 into the 0x20-byte-stride
// table at [0x00a69878], inside a pushal/popal pair. For every record with bit 0 of dword +0x4c set it
// writes {byte +0x202 of the record at +0x74 (0x00408351), dword +0x48, x, y, z, word +0xc6, word +0xc2,
// word +0xca}, where (x, y, z) = (+0x18, +0x1c, +0x20) reduced by the record's rotated vector (0x00552c61
// works on [0x00934df0], ecx and [0x00934df4]). The table ends with two zero dwords (the record at the
// cursor is also cleared first). The loop's last flag writer is the closing `xor eax, eax`.
// FUNCTION: GP4 0x005109d5
void __cdecl FUN_005109d5(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = GP4_GLOBAL(uint32_t, 0x00a69878);
    c.ebx = 0x00854414u;
    c.ecx = 0u;
    GP4_GLOBAL(uint32_t, c.edi) = c.ecx;
    GP4_GLOBAL(uint32_t, c.edi + 4u) = c.ecx;

    do {
        const uint32_t active = GP4_FIELD(uint32_t, c.ebx, 0x4c) & 1u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(active));
        if (active != 0u) {
            const uint32_t counter = c.ecx;
            c.esi = GP4_FIELD(uint32_t, c.ebx, 0x74);
            gp4::call_regs(0x00408351, c);
            GP4_GLOBAL(uint32_t, c.edi) = c.eax;
            c.eax = GP4_FIELD(uint32_t, c.ebx, 0x48);
            GP4_GLOBAL(uint32_t, c.edi + 4u) = c.eax;

            const gp4::Regs saved = c;
            c.eax = GP4_FIELD(uint32_t, c.ebx, 0x18);
            GP4_GLOBAL(uint32_t, 0x00934df0) = c.eax;
            c.eax = GP4_FIELD(uint32_t, c.ebx, 0x1c);
            c.ecx = c.eax;
            c.eax = GP4_FIELD(uint32_t, c.ebx, 0x20);
            GP4_GLOBAL(uint32_t, 0x00934df4) = c.eax;
            gp4::call_regs(0x00552c61, c);
            GP4_GLOBAL(uint32_t, c.edi + 8u) = GP4_GLOBAL(uint32_t, 0x00934df0);
            GP4_GLOBAL(uint32_t, c.edi + 0x0cu) = c.ecx;
            GP4_GLOBAL(uint32_t, c.edi + 0x10u) = GP4_GLOBAL(uint32_t, 0x00934df4);
            GP4_GLOBAL(uint32_t, c.edi + 0x14u) = GP4_FIELD(uint16_t, c.ebx, 0xc6);
            GP4_GLOBAL(uint32_t, c.edi + 0x18u) = GP4_FIELD(uint16_t, c.ebx, 0xc2);
            GP4_GLOBAL(uint32_t, c.edi + 0x1cu) = GP4_FIELD(uint16_t, c.ebx, 0xca);

            const uint32_t flags = c.eflags;
            c = saved;
            c.eflags = flags;
            c.edi += 0x20u;
            c.ecx = counter;
        }
        c.ebx += 0xe0u;
        c.ecx += 1u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.ecx, 0x140u));
    } while (c.ecx < 0x140u);

    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(0u));
    GP4_GLOBAL(uint32_t, c.edi) = 0u;
    GP4_GLOBAL(uint32_t, c.edi + 4u) = 0u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005109d5, FUN_005109d5, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rec_005109d5")
