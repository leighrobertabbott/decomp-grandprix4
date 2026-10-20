#include <gp4/gp4.h>
#include <gp4/flags.h>

// Camera-window setup (pushal/popal: every register is preserved). For each end of the
// segment window, [0x00866ed0] then [0x00866ed4], it copies the record's first word (to
// 0x00868538 / 0x0086853a), selects the byte 0x28 (0x29 when the byte [0x00868582] is negative)
// in [0x00934df8] and runs the record scaling 0x00550d52; the eax/ebx products are negated unless
// [0x00868582] is negative and give the points (eax + [0x00934df0], [0x00934df4] - ebx) at
// 0x00868528/0x0086852c and 0x00868530/0x00868534. Then the 12-byte entry of the per-circuit
// table 0x006f2e94 for the circuit [0x007ad894] is copied out: four words to 0x00875718..0x0087571e,
// the words +4 and +0xa times the 0x160 record size to 0x00875710 / 0x00875714.
// The flags are those of the last `mul edx` (pinned oracle model: CF = OF = high half != 0,
// which is zero here; ZF/SF/PF from the low half, AF = 0).
// FUNCTION: GP4 0x00550dbf
void __cdecl FUN_00550dbf(gp4::Regs* r) {
    gp4::Regs c = *r;

    uint32_t seg = GP4_GLOBAL(uint32_t, 0x00866ed0);
    uint16_t head = GP4_FIELD(uint16_t, seg, 0);
    c.edi = seg;
    c.eax = (c.eax & 0xffff0000u) | head;
    GP4_GLOBAL(uint16_t, 0x00868538) = head;
    GP4_GLOBAL(uint32_t, 0x00934df8) = 0x28u;
    if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) < 0) {
        GP4_GLOBAL(uint32_t, 0x00934df8) = 0x29u;
    }
    gp4::call_regs(0x00550d52, c);
    if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) >= 0) {
        c.eax = 0u - c.eax;
        c.ebx = 0u - c.ebx;
    }
    c.eax += GP4_GLOBAL(uint32_t, 0x00934df0);
    GP4_GLOBAL(uint32_t, 0x00868528) = c.eax;
    c.eax = GP4_GLOBAL(uint32_t, 0x00934df4) - c.ebx;
    GP4_GLOBAL(uint32_t, 0x0086852c) = c.eax;

    seg = GP4_GLOBAL(uint32_t, 0x00866ed4);
    head = GP4_FIELD(uint16_t, seg, 0);
    c.edi = seg;
    c.eax = (c.eax & 0xffff0000u) | head;
    GP4_GLOBAL(uint16_t, 0x0086853a) = head;
    GP4_GLOBAL(uint32_t, 0x00934df8) = 0x28u;
    if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) < 0) {
        GP4_GLOBAL(uint32_t, 0x00934df8) = 0x29u;
    }
    gp4::call_regs(0x00550d52, c);
    if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) >= 0) {
        c.eax = 0u - c.eax;
        c.ebx = 0u - c.ebx;
    }
    GP4_GLOBAL(uint32_t, 0x00868530) = c.eax + GP4_GLOBAL(uint32_t, 0x00934df0);
    GP4_GLOBAL(uint32_t, 0x00868534) = GP4_GLOBAL(uint32_t, 0x00934df4) - c.ebx;

    const uint32_t off = GP4_GLOBAL(uint32_t, 0x007ad894) * 0xcu;
    GP4_GLOBAL(uint16_t, 0x00875718) = GP4_FIELD(uint16_t, off, 0x006f2e94);
    GP4_GLOBAL(uint16_t, 0x0087571a) = GP4_FIELD(uint16_t, off, 0x006f2e96);
    GP4_GLOBAL(uint32_t, 0x00875710) = static_cast<uint32_t>(GP4_FIELD(uint16_t, off, 0x006f2e98)) * 0x160u;
    GP4_GLOBAL(uint16_t, 0x0087571c) = GP4_FIELD(uint16_t, off, 0x006f2e9a);
    GP4_GLOBAL(uint16_t, 0x0087571e) = GP4_FIELD(uint16_t, off, 0x006f2e9c);
    const uint32_t last = static_cast<uint32_t>(GP4_FIELD(uint16_t, off, 0x006f2e9e)) * 0x160u;
    GP4_GLOBAL(uint32_t, 0x00875714) = last;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(last));
}
GP4_IMPL(0x00550dbf, FUN_00550dbf, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=seg_00550d52+win_0042b10d+sign_00868582+trk_007ad894+head_00550dbf+circ_006f2e94")
