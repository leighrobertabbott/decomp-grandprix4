#include <gp4/gp4.h>

// Only the two bytes this routine marks.
struct Unknown_008943a0 {
    uint8_t Unknown00[0x48];
    uint8_t Flags48;
    uint8_t Unknown49[0x49];
    uint8_t Flags92;
    uint8_t Unknown93[0xcd];
};
static_assert(sizeof(Unknown_008943a0) == 0x160, "layout");

static inline Unknown_008943a0* recordAt(uint32_t address) {
    return reinterpret_cast<Unknown_008943a0*>(static_cast<uintptr_t>(address));
}

static inline void mark(uint32_t address, uint8_t bit) {
    Unknown_008943a0* record = recordAt(address);
    record->Flags48 |= bit;
    record->Flags92 |= bit;
}

// FUNCTION: GP4 0x0040adc2
// Marks the track-segment records (0x160 bytes at 0x008943a0) around the pit lane: bit 1 of bytes
// +0x48 and +0x92 from the pit-entry record [0x00866f00] forward up to four records past
// [0x00866ef0], and bit 2 from the pit-exit record [0x00866f04] backward down to four records
// before [0x00866ef4] (lap records wrap by the lap length [0x00866ec4]; pit records clamp at the
// pit end [0x00866ef8] / pit start [0x00891500]); then 0x0040aec5 marks the rest. Every register is
// restored by the popal; the flags are those 0x0040aec5 leaves.
void __cdecl FUN_0040adc2(gp4::Regs* r) {
    uint32_t stop = GP4_GLOBAL(uint32_t, 0x00866ef0) + 0x580u;
    if (GP4_GLOBAL(uint32_t, 0x00866ef0) < GP4_GLOBAL(uint32_t, 0x00891500)) {
        if (stop >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            stop -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    } else if (stop >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
        stop = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
    }

    uint32_t cursor = GP4_GLOBAL(uint32_t, 0x00866f00);
    do {
        mark(cursor, 0x01);
        cursor += 0x160u;
        if (cursor >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            if (cursor < GP4_GLOBAL(uint32_t, 0x00891500)) {
                cursor -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else if (cursor >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                cursor = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }
    } while (cursor != stop);

    stop = GP4_GLOBAL(uint32_t, 0x00866ef4) - 0x580u;
    if (GP4_GLOBAL(uint32_t, 0x00866ef4) < GP4_GLOBAL(uint32_t, 0x00891500)) {
        if (stop < GP4_GLOBAL(uint32_t, 0x00891504)) {
            stop += GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    } else if (stop < GP4_GLOBAL(uint32_t, 0x00891500)) {
        stop = GP4_GLOBAL(uint32_t, 0x00891500);
    }

    cursor = GP4_GLOBAL(uint32_t, 0x00866f04);
    do {
        if (cursor < GP4_GLOBAL(uint32_t, 0x00891500)) {
            cursor -= 0x160u;
            if (cursor < GP4_GLOBAL(uint32_t, 0x00891504)) {
                cursor += GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
        } else {
            cursor -= 0x160u;
            if (cursor < GP4_GLOBAL(uint32_t, 0x00891500)) {
                cursor = GP4_GLOBAL(uint32_t, 0x00891500);
            }
        }
        mark(cursor, 0x02);
    } while (cursor != stop);

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0040aec5, call);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0040adc2, FUN_0040adc2, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=pitwin_0040adc2")
