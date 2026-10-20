#include <gp4/gp4.h>

struct Unknown_0043259c_Car {
    uint8_t Unknown00[0x71];
    uint8_t Flags71;
    uint8_t Unknown72[0x8d - 0x72];
    uint8_t Flags8D;
    uint8_t Unknown8E[0xfe - 0x8e];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x1fc - 0xff];
    uint32_t Deadline1FC;
    uint8_t Unknown200[0x230 - 0x200];
    uint8_t Flags230;
};

struct Unknown_0043259c_Seg {
    uint8_t Unknown00[0x92];
    uint8_t Flags92;
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of "test"/"and"/"cmp x, 0" on a byte result (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0043259c
// Per-car timer/state check on the car in esi (edi = the car's track segment). A non-zero deadline
// at +0x1fc that the clock 0x0066d814 has reached is cleared. Unless byte 0x007c2e75 is set, a car
// with bit 7 of +0x71 set either runs the state machine 0x00432feb (bit 1 of +0x8d set) or, when
// bit 7 of +0x230 is set and the segment's flags (+0x92 & 3) are clear, drops bit 7 of +0x71 and
// bit 2 of +0xfe. The flags (and al on the last paths) are those of the last instruction executed.
void __cdecl FUN_0043259c(gp4::Regs* r) {
    Unknown_0043259c_Car* car = reinterpret_cast<Unknown_0043259c_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0043259c_Seg* seg = reinterpret_cast<const Unknown_0043259c_Seg*>(static_cast<uintptr_t>(r->edi));

    // The original tests the deadline twice in a row (the second test cannot differ).
    if (car->Deadline1FC != 0u) {
        const uint32_t sum = (0u - car->Deadline1FC) + GP4_GLOBAL(uint32_t, 0x0066d814);
        if ((sum & 0x80000000u) == 0u) {
            car->Deadline1FC = 0u;
        }
    }

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2e75);
    uint32_t flags = logicFlags8(mode);
    if (mode == 0u) {
        const uint32_t armed = car->Flags71 & 0x80u;
        flags = logicFlags8(armed);
        if (armed != 0u) {
            const uint32_t queued = car->Flags8D & 2u;
            flags = logicFlags8(queued);
            if (queued != 0u) {
                gp4::Regs call = *r;
                call.st_in = 0;
                call.st_out = 0;
                gp4::call_regs(0x00432feb, call);
                *r = call;
                return;
            }
            const uint32_t pending = car->Flags230 & 0x80u;
            flags = logicFlags8(pending);
            if (pending != 0u) {
                const uint32_t low = seg->Flags92 & 3u;
                r->eax = (r->eax & 0xffffff00u) | low;
                flags = logicFlags8(low);
                if (low == 0u) {
                    car->Flags71 = static_cast<uint8_t>(car->Flags71 & 0x7fu);
                    car->FlagsFE = static_cast<uint8_t>(car->FlagsFE & 0xfbu);
                    flags = logicFlags8(car->FlagsFE);
                }
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043259c, FUN_0043259c, "regs(eax:u32, edi:ptr[0x9a]:bytes, esi:ptr[0x420]:layout=rec_0043259c+rec_00410894+rec_004087aa) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rows_00434044+state_00432cc2+flag_00622454+state_00432f18+state_004327cd+state_0043260e+state_00432a8b+state_00410894+state_004087aa+state_0066d8c0+counter_0040f0dd+trk_007ad894+state_0040f112+div_0060138c+state_00411109")
