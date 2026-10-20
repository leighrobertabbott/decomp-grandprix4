#include <gp4/gp4.h>

static inline uint8_t* bytesAt(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// Flags of an 8-bit logic result (test byte, imm): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    uint32_t p = result & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return ((p & 1u) == 0u ? 0x04u : 0u) | ((result & 0xffu) == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// FUNCTION: GP4 0x0041ed6b
// ebx = a track segment record. Looks up the surface class of its position (words +4 / +8 >> 3 through
// 0x0040c43b); when the class has no entry in the table 0x006225c0 it advances the segment's 10-byte entry at
// 0x00675a82 + 10 * word(+0x7a): the counter byte +5 gains a table step (0x006227fc[b1 >> 4]) and, on a carry,
// the bytes +3 / +4 are moved by the 0x0062280d table (+3 down to 0, +4 capped at b1 + 0x20 and 0x70). Any
// change sets 0x0062ac98 = 0xff, which then runs 0x0051f2e9 on the segment (edi, restored afterwards).
void __cdecl FUN_0041ed6b(gp4::Regs* r) {
    const uint32_t segmentAddress = r->ebx;
    uint8_t* const segment = bytesAt(segmentAddress);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint8_t, 0x0062ac98) = 0;

    c.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, segment, 4)) >> 3);
    c.edx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, segment, 8)) >> 3);
    gp4::call_regs(0x0040c43b, c);

    if (GP4_ARRAY(uint32_t, 0x006225c0)[c.eax] == 0u) {
        uint8_t* const entry = bytesAt(GP4_FIELD(uint16_t, segment, 0x7a) * 10u + 0x00675a82u);

        c.edx = static_cast<uint32_t>(GP4_FIELD(uint8_t, entry, 1)) >> 4;
        const uint8_t step = GP4_ARRAY(uint8_t, 0x006227fc)[c.edx];
        c.eax = (c.eax & 0xffffff00u) | step;
        const uint8_t counter = GP4_FIELD(uint8_t, entry, 5);
        const uint8_t sum = static_cast<uint8_t>(counter + step);
        GP4_FIELD(uint8_t, entry, 5) = sum;
        if (sum < counter) {
            uint32_t value = GP4_FIELD(uint8_t, entry, 4);
            uint32_t drop = GP4_ARRAY(uint8_t, 0x0062280d)[value >> 4];
            const uint32_t raised = value + drop;

            int32_t lowered = static_cast<int32_t>(GP4_FIELD(uint8_t, entry, 3)) - static_cast<int32_t>(drop);
            if (lowered < 0) {
                lowered = 0;
            }
            if (GP4_FIELD(uint8_t, entry, 3) != static_cast<uint8_t>(lowered)) {
                GP4_FIELD(uint8_t, entry, 3) = static_cast<uint8_t>(lowered);
                GP4_GLOBAL(uint8_t, 0x0062ac98) = 0xff;
            }

            int32_t capped = static_cast<int32_t>(raised);
            c.edx = static_cast<uint32_t>(GP4_FIELD(uint8_t, entry, 1)) + 0x20u;
            if (capped > static_cast<int32_t>(c.edx)) {
                capped = static_cast<int32_t>(c.edx);
            }
            if (capped > 0x70) {
                capped = 0x70;
            }
            c.eax = static_cast<uint32_t>(capped);
            if (GP4_FIELD(uint8_t, entry, 4) != static_cast<uint8_t>(capped)) {
                GP4_FIELD(uint8_t, entry, 4) = static_cast<uint8_t>(capped);
                GP4_GLOBAL(uint8_t, 0x0062ac98) = 0xff;
            }
        }
    }

    c.ebx = segmentAddress;
    uint32_t flags = logicFlags8(GP4_GLOBAL(uint8_t, 0x0062ac98));
    if (GP4_GLOBAL(uint8_t, 0x0062ac98) != 0u) {
        c.eflags = (c.eflags & ~0x8d5u) | flags;
        c.edi = segmentAddress;
        gp4::call_regs(0x0051f2e9, c);
        c.edi = r->edi;
        flags = c.eflags & 0x8d5u;
    }

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebx = segmentAddress;
    r->edi = c.edi;
    r->esi = c.esi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0041ed6b, FUN_0041ed6b, "regs(ebx:rec[0x8943a0,0x160,0..25]) -> (eax,ecx,edx,cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=map_0040c43b+ring_008943a0+ring_0040c0ae+surf_0041f02b")
