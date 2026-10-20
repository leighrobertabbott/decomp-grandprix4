#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (or byte, imm): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// FUNCTION: GP4 0x00428efb
// esi = car record, edi = track segment record (flag byte +0x51). Chooses between two ways of
// filling the car's +0x1a4/+0x272/+0xfe/+0x9d group; the flags are those of the final `or [esi+0x9d]`.
void __cdecl FUN_00428efb(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebx = r->ebx;
    uint32_t ecx = r->ecx;
    uint32_t ebp = r->ebp;

    bool useCurrent = false;     // 0x00428f5f: copy the blend fraction and mode
    if ((GP4_FIELD(uint8_t, car, 0x8d) & 2u) == 0u) {
        bool blocked = false;
        if (GP4_FIELD(int32_t, car, 0x1a0) < 0) {
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            gp4::call_regs(0x00428ec6, c);
            eax = c.eax;
            edx = c.edx;
            ebx = c.ebx;
            ecx = c.ecx;
            ebp = c.ebp;
            blocked = (c.eflags & 1u) != 0u;
        }
        const uint8_t segFlags = GP4_FIELD(uint8_t, segment, 0x51);
        if (!blocked && (segFlags & 0x20u) != 0u && GP4_FIELD(uint8_t, car, 0x294) != 7) {
            const bool carBit = (GP4_FIELD(uint8_t, car, 0x71) & 8u) != 0u;
            if ((segFlags & 0x80u) != 0u) {
                gp4::Regs c = *r;
                c.st_in = 0;
                c.st_out = 0;
                c.eax = eax;
                c.edx = edx;
                c.ebx = ebx;
                c.ecx = ecx;
                c.ebp = ebp;
                gp4::call_regs(0x00428dc1, c);
                eax = c.eax;
                edx = c.edx;
                ebx = c.ebx;
                ecx = c.ecx;
                ebp = c.ebp;
                useCurrent = (c.eflags & 1u) == 0u;
            } else if ((segFlags & 8u) != 0u) {
                useCurrent = !carBit;
            } else {
                useCurrent = carBit;
            }
        }
    }

    if (useCurrent) {
        GP4_FIELD(uint8_t, car, 0x9d) |= 0x40;
        const uint32_t fraction = GP4_FIELD(uint32_t, car, 0x1a0);
        GP4_FIELD(uint32_t, car, 0x1a4) = fraction;
        const uint8_t mode = GP4_FIELD(uint8_t, car, 0x294);
        GP4_FIELD(uint8_t, car, 0x272) = mode;
        eax = (fraction & 0xffffff00u) | mode;
        GP4_FIELD(uint8_t, car, 0xfe) &= 0x7f;
        if ((GP4_FIELD(uint8_t, car, 0x71) & 8u) != 0u) {
            GP4_FIELD(uint8_t, car, 0xfe) |= 0x80;
        }
        GP4_FIELD(uint8_t, car, 0x9d) |= 0x40;
    } else {
        const uint32_t word = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x10a)));
        GP4_FIELD(uint32_t, car, 0x1a4) = word;
        const uint8_t mode = GP4_FIELD(uint8_t, car, 0x295);
        GP4_FIELD(uint8_t, car, 0x272) = mode;
        eax = (word & 0xffffff00u) | mode;
        GP4_FIELD(uint8_t, car, 0xfe) &= 0x7f;
        if ((GP4_FIELD(uint8_t, car, 0xff) & 2u) != 0u) {
            GP4_FIELD(uint8_t, car, 0xfe) |= 0x80;
        }
        GP4_FIELD(uint8_t, car, 0x9d) |= 0x10;
    }

    r->eax = eax;
    r->edx = edx;
    r->ebx = ebx;
    r->ecx = ecx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(GP4_FIELD(uint8_t, car, 0x9d));
}
GP4_IMPL(0x00428efb, FUN_00428efb, "regs(edi:ptr[0x13d]:bytes,esi:ptr[0x29d]:layout=car_00431ffb+car_004035c3+car_0042b17e) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+words_008943a0+tune_00428dc1")
