#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a 32-bit logic result (test / and / xor): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Hands the registers and flags a called routine leaves back to the caller's register file.
static inline void takeResult(gp4::Regs* r, const gp4::Regs& call) {
    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->esi = call.esi;
    r->edi = call.edi;
    r->ebp = call.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}

// FUNCTION: GP4 0x004379a0
// Reacts to a change of the car's input-state dword (+0x40 against its previous value +0x44):
// nothing when +0x40 is zero; otherwise it releases the car's slot [0x6098c0 + byte +0x25e] (when
// bit 2 of +0xcc is set and the byte is not 0xff), refreshes the status bits (0x437960) and, when the
// states differ in the low gear bits (0x3e) or in the high bits (0x1e0000), marks the car (+0x80
// 0x80/0x40, +0x81 0x20, +0x86 2/1, +0x203 0x10, +0x25f &= 0xfb) and runs 0x437a49 and 0x40fbf3.
// pushal/popal restore every register; the flags are those of the last flag writer on each path.
void __cdecl FUN_004379a0(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t state = GP4_FIELD(uint32_t, car, 0x40);
    if (state == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(state);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    if ((GP4_FIELD(uint8_t, car, 0xcc) & 4u) != 0u && GP4_FIELD(uint8_t, car, 0x25e) != 0xffu) {
        const uint32_t slot = GP4_FIELD(uint8_t, car, 0x25e);
        c.eax = slot;
        GP4_ARRAY(uint32_t, 0x006098c0)[slot] = 0u;
    }

    gp4::call_regs(0x00437960, c);

    const uint32_t changed = GP4_FIELD(uint32_t, car, 0x40) ^ GP4_FIELD(uint32_t, car, 0x44);
    c.ebp = changed;

    bool highChange = (changed & 0x001e0000u) != 0u;
    if ((changed & 0x3eu) != 0u) {
        GP4_FIELD(uint8_t, car, 0x80) |= 0x80;
        GP4_FIELD(uint8_t, car, 0x81) |= 0x20;
        GP4_FIELD(uint8_t, car, 0x86) |= 2;
        GP4_FIELD(uint8_t, car, 0x203) |= 0x10;
        GP4_FIELD(uint8_t, car, 0x25f) &= 0xfb;
    } else if (!highChange) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(changed & 0x001e0000u);
        return;
    }
    if (highChange) {
        GP4_FIELD(uint8_t, car, 0x80) |= 0x40;
        GP4_FIELD(uint8_t, car, 0x81) |= 0x20;
        GP4_FIELD(uint8_t, car, 0x86) |= 1;
        GP4_FIELD(uint8_t, car, 0x203) |= 0x10;
        GP4_FIELD(uint8_t, car, 0x25f) &= 0xfb;
    }

    gp4::call_regs(0x00437a49, c);
    gp4::call_regs(0x0040fbf3, c);
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x004379a0, FUN_004379a0, "regs(esi:ptr[0x420]:layout=car_004129fa+car_00411109) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=curves_0062ecb0+state_004087aa+div_0060138c+trk_007ad894+circ_007adb50+codes_0043ba12+state_0040fbf3+state_004059ef+state_0040e84e")
