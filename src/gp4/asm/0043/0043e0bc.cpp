#include <gp4/gp4.h>

// esi: a car record (only the word read and written here is named).
struct Unknown_0043e0bc_Car {
    uint8_t Unknown00[0x9a];
    int16_t Unknown9A;
};

// edi: a 0x160-byte track-segment record (array at 0x8943a0).
struct Unknown_0043e0bc_Segment {
    int16_t Unknown00;
    uint8_t Unknown02[0x10];
    uint8_t Flags12;
    uint8_t Unknown13[5];
    int16_t Unknown18;
    uint8_t Unknown1A[0x71 - 0x1a];
    uint8_t Unknown71;
};

static inline Unknown_0043e0bc_Segment* segmentAt(uint32_t addr) {
    return reinterpret_cast<Unknown_0043e0bc_Segment*>(static_cast<uintptr_t>(addr));
}

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | ((((left ^ right) & (left ^ result)) >> 31) << 11);
}

// One step back along the segment ring: with the direction byte 0x86857a set it only steps while the
// result stays at or above the pit start [0x891500]; otherwise from the pit it clamps at the pit start
// and from the lap it wraps forward by [0x866ec4] under the lap start [0x891504].
static inline uint32_t stepBack(uint32_t seg) {
    if (GP4_GLOBAL(uint8_t, 0x0086857a) != 0u) {
        const uint32_t prev = seg - 0x160u;
        return prev >= GP4_GLOBAL(uint32_t, 0x00891500) ? prev : seg;
    }
    if (seg >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        seg -= 0x160u;
        if (seg < GP4_GLOBAL(uint32_t, 0x00891500)) {
            seg = GP4_GLOBAL(uint32_t, 0x00891500);
        }
    } else {
        seg -= 0x160u;
        if (seg < GP4_GLOBAL(uint32_t, 0x00891504)) {
            seg += GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    }
    return seg;
}

// FUNCTION: GP4 0x0043e0bc
// Walks back along the segment ring from edi while the car's word +0x9a (esi) stays below the target
// [0x62ac50]: each step either (word below target) rescales the word with 0x0043dbdc from the angle
// change between the two records and clamps it to the target, or (word at target) raises it by one per
// step above the target ([0x62ac4c] counts them). Every visited record gets its step number (capped at
// 0xff) in byte +0x71, then 0x0043e02d runs, bit 3 of byte +0x12 is set when the word is still below
// 0x4800 with [0x622426] == 0 and [0x626998] >= [0x62699c], and 0x0043d76b (CF = done) closes the
// step. eax, edx, edi and the flags are what the last calls leave.
void __cdecl FUN_0043e0bc(gp4::Regs* r) {
    Unknown_0043e0bc_Car* car = reinterpret_cast<Unknown_0043e0bc_Car*>(static_cast<uintptr_t>(r->esi));
    uint32_t edi = r->edi;

    GP4_GLOBAL(uint32_t, 0x0062ac44) = 0;
    GP4_GLOBAL(uint32_t, 0x0062ac4c) = 0;

    const int32_t first = car->Unknown9A;
    r->eax = static_cast<uint32_t>(first);
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags(static_cast<uint32_t>(first), GP4_GLOBAL(uint32_t, 0x0062ac50));
    if (first >= GP4_GLOBAL(int32_t, 0x0062ac50)) {
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    for (;;) {
        if (static_cast<int32_t>(car->Unknown9A) < GP4_GLOBAL(int32_t, 0x0062ac50)) {
            const Unknown_0043e0bc_Segment* from = segmentAt(edi);
            uint16_t angle = static_cast<uint16_t>(from->Unknown18 + from->Unknown00);
            edi = stepBack(edi);
            const Unknown_0043e0bc_Segment* to = segmentAt(edi);
            angle = static_cast<uint16_t>(angle - to->Unknown18);
            angle = static_cast<uint16_t>(angle - to->Unknown00);
            int32_t change = static_cast<int16_t>(angle);
            if (change < 0) {
                change = -change;
            }
            c.eax = static_cast<uint32_t>(change);
            c.edi = edi;
            gp4::call_regs(0x0043dbdc, c);
            int32_t word = car->Unknown9A;
            if (word > GP4_GLOBAL(int32_t, 0x0062ac50)) {
                word = GP4_GLOBAL(int32_t, 0x0062ac50);
            }
            car->Unknown9A = static_cast<int16_t>(word);
        } else {
            GP4_GLOBAL(int32_t, 0x0062ac4c) += 1;
            car->Unknown9A = static_cast<int16_t>(GP4_GLOBAL(int32_t, 0x0062ac50) + GP4_GLOBAL(int32_t, 0x0062ac4c));
            edi = stepBack(edi);
        }

        GP4_GLOBAL(int32_t, 0x0062ac44) += 1;
        int32_t steps = GP4_GLOBAL(int32_t, 0x0062ac44);
        if (steps > 0xff) {
            steps = 0xff;
        }
        segmentAt(edi)->Unknown71 = static_cast<uint8_t>(steps);
        c.eax = static_cast<uint32_t>(steps);
        c.edi = edi;
        gp4::call_regs(0x0043e02d, c);

        // 0x43e20f loads eax from [0x626998] whenever [0x622426] == 0 (the compares after it only decide
        // the flag store); otherwise eax still holds the step count.
        const bool loaded = GP4_GLOBAL(uint8_t, 0x00622426) == 0u;
        if (loaded) {
            c.eax = GP4_GLOBAL(uint32_t, 0x00626998);
            if (GP4_GLOBAL(int32_t, 0x00626998) >= GP4_GLOBAL(int32_t, 0x0062699c)
                && car->Unknown9A < 0x4800) {
                segmentAt(edi)->Flags12 |= 8;
            }
        }

        c.edi = edi;
        gp4::call_regs(0x0043d76b, c);
        if ((c.eflags & 1u) != 0u) {
            break;
        }
    }

    r->eax = c.eax;
    r->edx = c.edx;
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0043e0bc, FUN_0043e0bc, "regs(esi:rec[0x66fef0,0x41c,0..21], edi:rec[0x8943a0,0x160,0..25]) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+bank_004279ab+ring_008943a0+state_00405489+engine_00413606+drive_00405552+search_00405552+seg_00405552+tab_00425f41+state_0043e0bc+list_0066fe3c")
