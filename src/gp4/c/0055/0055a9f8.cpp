#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0055a9f8
// Track-position ratios from the pit and lap record pointers. [0x86856a] receives the low 11 bits of the word +0x1a
// of the pit-entry record [0x866f00]. Then three 16-bit divisions (div cx, each skipped, with CF set by stc, when
// the high word of its dividend is not below the divisor):
//   1. q1 = ((lap end [0x866ecc] - entry [0x866f00] + exit [0x866f04]) - lap start [0x891504]) / 0x160 (records
//      between the pit entry and the lap start);
//   2. q2 = (pit end [0x866ef8] - pit start [0x891500]) / 0x160, kept in cx;
//   3. [0x868568] = (int16(q1) << 14) / q2 (a Q14 ratio).
// eax, ecx (low word), edx and edi are left as the last instructions produced them; the flags are those of the last
// `cmp dx, cx` with CF forced by clc (divided) / stc (skipped).
void __cdecl FUN_0055a9f8(gp4::Regs* r) {
    const uint32_t entry = GP4_GLOBAL(uint32_t, 0x00866f00);
    const uint16_t entryWord = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(entry) + 0x1au);
    GP4_GLOBAL(uint16_t, 0x0086856a) = static_cast<uint16_t>(entryWord & 0x7ffu);

    uint32_t eax = GP4_GLOBAL(uint32_t, 0x00866ecc);
    eax -= GP4_GLOBAL(uint32_t, 0x00866f00);
    eax += GP4_GLOBAL(uint32_t, 0x00866f04);
    eax -= GP4_GLOBAL(uint32_t, 0x00891504);
    uint32_t edx = eax >> 16;
    uint32_t cx = 0x160u;
    uint32_t flags = gp4::flags::sub16(edx, cx);
    if (static_cast<uint16_t>(edx) < static_cast<uint16_t>(cx)) {
        const uint32_t dividend = (edx << 16) | (eax & 0xffffu);
        eax = (eax & 0xffff0000u) | (dividend / cx);
        edx = dividend % cx;
        flags &= ~gp4::flags::CF;
    } else {
        flags |= gp4::flags::CF;
    }
    const uint32_t first = eax;

    eax = GP4_GLOBAL(uint32_t, 0x00866ef8);
    eax -= GP4_GLOBAL(uint32_t, 0x00891500);
    edx = eax >> 16;
    flags = gp4::flags::sub16(edx, cx);
    if (static_cast<uint16_t>(edx) < static_cast<uint16_t>(cx)) {
        const uint32_t dividend = (edx << 16) | (eax & 0xffffu);
        eax = (eax & 0xffff0000u) | (dividend / cx);
        edx = dividend % cx;
        flags &= ~gp4::flags::CF;
    } else {
        flags |= gp4::flags::CF;
    }
    cx = eax & 0xffffu;

    eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(first))) << 14;
    edx = eax >> 16;
    flags = gp4::flags::sub16(edx, cx);
    if (static_cast<uint16_t>(edx) < static_cast<uint16_t>(cx)) {
        const uint32_t dividend = (edx << 16) | (eax & 0xffffu);
        eax = (eax & 0xffff0000u) | (dividend / cx);
        edx = dividend % cx;
        flags &= ~gp4::flags::CF;
    } else {
        flags |= gp4::flags::CF;
    }
    GP4_GLOBAL(uint16_t, 0x00868568) = static_cast<uint16_t>(eax);

    r->eax = eax;
    r->ecx = (r->ecx & 0xffff0000u) | cx;
    r->edx = edx;
    r->edi = entry;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0055a9f8, FUN_0055a9f8, "regs(ecx:u32) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pit_0055a9f8")
