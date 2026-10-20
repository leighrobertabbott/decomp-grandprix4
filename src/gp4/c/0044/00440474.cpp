#include <gp4/gp4.h>

// 0x160-byte track-segment record, only the words this routine touches.
struct SegmentRecord {
    uint8_t Unknown00[0x1a];
    uint16_t Flags1a;        // low 11 bits compared against the limit word 0x00868566
    uint8_t Unknown1c[0xa6 - 0x1c];
    int16_t WordA6;          // minimum-style value propagated back along the pit lane
    uint8_t UnknownA8[0x15c - 0xa8];
    uint16_t Word15c;        // copied together with WordA6
    uint8_t Unknown15e[2];
};
static_assert(sizeof(SegmentRecord) == 0x160, "record stride");

static inline uint32_t Flags16Sub(uint32_t a, uint32_t b) {
    // flags of 'cmp a16, b16' as CF PF AF ZF SF OF
    const uint32_t res = (a - b) & 0xffffu;
    uint32_t parity = res & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    uint32_t f = 0;
    if ((a & 0xffffu) < (b & 0xffffu)) f |= 0x001u;
    if (!(parity & 1u)) f |= 0x004u;
    if ((a ^ b ^ res) & 0x10u) f |= 0x010u;
    if (res == 0) f |= 0x040u;
    if (res & 0x8000u) f |= 0x080u;
    if (((a ^ b) & (a ^ res)) & 0x8000u) f |= 0x800u;
    return f;
}

// Walks the pit-lane records backwards from the pit end [0x866ef8], in step with a second cursor that
// follows the lap ring ([0x866f04], wrapped by [0x866ec4] below the lap start [0x891504], clamped to the pit
// start [0x891500]). While the record's masked word +0x1a is not below the limit word [0x868566], the
// destination takes the second cursor's word +0xa6 (and +0x15c) when that is smaller; the walk stops at the
// first record that is not updated, at the limit, or after 0x3d steps.
// FUNCTION: GP4 0x00440474
void __cdecl FUN_00440474(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t esi = r->esi;
    uint32_t ebp = r->ebp;
    const int16_t limit = GP4_GLOBAL(int16_t, 0x00868566);
    const uint32_t edx = (r->edx & 0xffff0000u) | (uint16_t)limit;
    uint32_t edi = GP4_GLOBAL(uint32_t, 0x00866f04);
    uint32_t ebx = GP4_GLOBAL(uint32_t, 0x00866ef8);
    uint32_t ecx = 0x3d;
    uint32_t flags;

    for (;;) {
        if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
            edi -= 0x160;
            if (edi < GP4_GLOBAL(uint32_t, 0x00891504)) edi += GP4_GLOBAL(uint32_t, 0x00866ec4);
        } else {
            edi -= 0x160;
            if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) edi = GP4_GLOBAL(uint32_t, 0x00891500);
        }
        ebx -= 0x160;
        SegmentRecord* dst = (SegmentRecord*)ebx;
        const SegmentRecord* src = (const SegmentRecord*)edi;

        const uint32_t masked = dst->Flags1a & 0x7ffu;
        eax = (eax & 0xffff0000u) | masked;
        if ((int16_t)masked < limit) {
            flags = Flags16Sub(masked, (uint16_t)limit);
            break;
        }

        ebp = 0;
        esi = (esi & 0xffff0000u) | (uint16_t)src->WordA6;
        eax = (eax & 0xffff0000u) | (uint16_t)dst->WordA6;
        if (src->WordA6 < dst->WordA6) {
            dst->WordA6 = src->WordA6;
            dst->Word15c = src->Word15c;
            ebp = 0xffffffffu;
        }
        if (ebp == 0) {
            flags = 0x044u;  // 'or ebp,ebp' with zero: ZF | PF
            break;
        }
        --ecx;
        if (ecx == 0) {
            flags = 0x044u;  // 'dec ecx' to zero (CF kept clear by the 'or'): ZF | PF
            break;
        }
    }

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->edx = edx;
    r->esi = esi;
    r->edi = edi;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00440474, FUN_00440474, "regs(eax:u32, edx:u32, esi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pitw_00409467+ring_008943a0+records_008943a0")
