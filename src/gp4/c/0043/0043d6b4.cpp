#include <gp4/gp4.h>

// Flags of a 16-bit `sub left, right` / `cmp left, right`.
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// FUNCTION: GP4 0x0043d6b4
// Walks the ring of 0x160-byte records starting at [0x00891504], (word [0x00868558] + 0x100 + 1)
// records long (the ring wraps at [0x00866eb8] by subtracting [0x00866ebc]). For each record
// sets or clears bit 2 of byte +0x12 depending on whether (byte +0x10 & 0x7f) is below a
// threshold built from +0x10, +0x14 and +0xa6.
// Register results are those of the last record.
void __cdecl FUN_0043d6b4(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(GP4_GLOBAL(uint32_t, 0x00891504));
    uint16_t si = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00868558) + 0x100);

    uint8_t al = rec[0x10];
    uint8_t ah = 0;
    uint8_t bl = 0;
    uint8_t bh = 0;
    uint16_t cx = 0;
    uint16_t dx = 0;
    uint32_t flags = 0;
    bool first = true;

    for (;;) {
        if (!first) {
            al = rec[0x10];
        }
        if (first || ((al ^ bl) & 0x80u) != 0) {
            ah = static_cast<uint8_t>((al & 0x7fu) >> 4);
        }
        first = false;
        bl = al;

        int16_t level = static_cast<int16_t>(GP4_FIELD(uint16_t, rec, 0xa6) - 0x2000);
        if (level > 0x2000) {
            level = 0x2000;
        }
        if (level < 0) {
            level = 0;
        }
        cx = static_cast<uint16_t>(static_cast<uint16_t>(level) >> 10);

        dx = GP4_FIELD(uint16_t, rec, 0x14);
        bool useDefault = true;
        if (dx != 0) {
            const uint8_t dh = static_cast<uint8_t>((dx >> 8) ^ al);
            dx = static_cast<uint16_t>((static_cast<uint16_t>(dh) << 8) | (dx & 0xffu));
            if ((dh & 0x80u) != 0) {
                uint16_t magnitude = GP4_FIELD(uint16_t, rec, 0x14);
                if (static_cast<int16_t>(magnitude) < 0) {
                    magnitude = static_cast<uint16_t>(-magnitude);
                }
                if (static_cast<int16_t>(magnitude) > 0x800) {
                    magnitude = 0x800;
                }
                dx = static_cast<uint16_t>(magnitude >> 6);
                bh = static_cast<uint8_t>(0xe + (dx & 0xffu) + (cx & 0xffu));
                useDefault = false;
            }
        }
        if (useDefault) {
            bh = static_cast<uint8_t>(0xe + ah + (cx & 0xffu));
        }

        al &= 0x7f;
        if (static_cast<int8_t>(al) < static_cast<int8_t>(bh)) {
            rec[0x12] |= 4;
        } else {
            rec[0x12] &= 0xfb;
        }

        rec += 0x160;
        if (!(reinterpret_cast<uintptr_t>(rec) < GP4_GLOBAL(uint32_t, 0x00866eb8))) {
            rec -= GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        flags = subFlags16(si, 1);
        si = static_cast<uint16_t>(si - 1);
        if ((si & 0x8000u) != 0) {
            break;
        }
    }

    r->eax = (r->eax & 0xffff0000u) | (static_cast<uint32_t>(ah) << 8) | al;
    r->ebx = (r->ebx & 0xffff0000u) | (static_cast<uint32_t>(bh) << 8) | bl;
    r->ecx = (r->ecx & 0xffff0000u) | cx;
    r->edx = (r->edx & 0xffff0000u) | dx;
    r->esi = (r->esi & 0xffff0000u) | si;
    r->edi = reinterpret_cast<uintptr_t>(rec);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043d6b4, FUN_0043d6b4, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0")
