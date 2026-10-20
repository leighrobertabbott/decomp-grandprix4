#include <gp4/gp4.h>

// bts dword ptr [base], bit: the bit offset may address dwords past `base`.
static inline void setBit(uint32_t* base, uint32_t bit) {
    base[bit >> 5] |= 1u << (bit & 31u);
}

// For each of the 22 car records at 0x0066fef0 (stride 0x41c) that is not flagged out (+0x7a bits
// 0x80 / 0x40), collects three car-number bit sets: DAT_0063b960 for a car in state +0xfe bit 4 that
// passes the +0x8d/+0x71/+0x26b/+0x230 flag test (it also runs 0x00432719 once and latches +0x25f
// bit 4; without +0xfe bit 4 the latch is cleared), DAT_0063b95c for a car on kind-1 (+0xe4)
// whose signed +0x90 (negated when DAT_00868582 < 0) reaches DAT_00603b1a, whose 0x00432c21 index is
// zero and whose word +0x9a is at least 0x3ab (bit = byte +0x73 - 1, clamped at 0), and
// DAT_0063b964 for a car with +0x273 bit 0. The car-number bit is (byte +0x7c - 1) & 0x3f.
// Leaves ecx = 0, esi = the end of the car array, the flags of "dec ecx" reaching zero (ZF, PF) and
// eax = the last value computed (low word only for the +0x90 compare).
// FUNCTION: GP4 0x00433d8d
void __cdecl FUN_00433d8d(gp4::Regs* r) {
    uint32_t eax = r->eax;
    GP4_GLOBAL(uint32_t, 0x0063b95c) = 0;
    GP4_GLOBAL(uint32_t, 0x0063b960) = 0;
    GP4_GLOBAL(uint32_t, 0x0063b964) = 0;

    uint8_t* car = GP4_ARRAY(uint8_t, 0x0066fef0);
    uint32_t count = 0x16;
    do {
        if ((GP4_FIELD(uint8_t, car, 0x7a) & 0x80u) == 0 && (GP4_FIELD(uint8_t, car, 0x7a) & 0x40u) == 0) {
            if ((GP4_FIELD(uint8_t, car, 0xfe) & 4u) == 0) {
                GP4_FIELD(uint8_t, car, 0x25f) = GP4_FIELD(uint8_t, car, 0x25f) & 0xfbu;
            } else {
                bool mark;
                if ((GP4_FIELD(uint8_t, car, 0x8d) & 2u) == 0) {
                    mark = (GP4_FIELD(uint8_t, car, 0x71) & 2u) == 0;
                } else if ((GP4_FIELD(uint8_t, car, 0x26b) & 0x80u) != 0) {
                    mark = true;
                } else {
                    mark = (GP4_FIELD(uint8_t, car, 0x230) & 0x80u) == 0;
                }
                if (mark) {
                    eax = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x7c) - 1) & 0x3fu;
                    setBit(GP4_ARRAY(uint32_t, 0x0063b960), eax);
                    if ((GP4_FIELD(uint8_t, car, 0x25f) & 4u) == 0) {
                        gp4::Regs callee = *r;
                        callee.st_in = 0;
                        callee.st_out = 0;
                        callee.eax = eax;
                        callee.ecx = count;
                        callee.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
                        gp4::call_regs(0x00432719, callee);
                        GP4_FIELD(uint8_t, car, 0x25f) = GP4_FIELD(uint8_t, car, 0x25f) | 4u;
                    }
                }
            }
            if ((GP4_FIELD(uint8_t, car, 0x8d) & 2u) != 0 && GP4_FIELD(uint8_t, car, 0xe4) == 1) {
                uint16_t ax = GP4_FIELD(uint16_t, car, 0x90);
                if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) < 0) {
                    ax = static_cast<uint16_t>(0u - ax);
                }
                eax = (eax & 0xffff0000u) | ax;
                if (static_cast<int16_t>(ax) >= GP4_GLOBAL(int16_t, 0x00603b1a)) {
                    gp4::Regs callee = *r;
                    callee.st_in = 0;
                    callee.st_out = 0;
                    callee.eax = eax;
                    callee.ecx = count;
                    callee.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
                    gp4::call_regs(0x00432c21, callee);
                    eax = callee.eax;
                    if (eax == 0 && GP4_FIELD(int16_t, car, 0x9a) >= 0x3ab) {
                        int32_t index = static_cast<int32_t>(GP4_FIELD(uint8_t, car, 0x73)) - 1;
                        if (index < 0) {
                            index = 0;
                        }
                        eax = static_cast<uint32_t>(index);
                        setBit(GP4_ARRAY(uint32_t, 0x0063b95c), eax);
                    }
                }
            }
            if ((GP4_FIELD(uint8_t, car, 0x273) & 1u) != 0) {
                eax = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x7c) - 1) & 0x3fu;
                setBit(GP4_ARRAY(uint32_t, 0x0063b964), eax);
            }
        }
        car += 0x41c;
        --count;
    } while (count != 0);

    r->eax = eax;
    r->ecx = 0;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
    // dec ecx reaching zero: ZF and PF set (CF keeps the carry of the last add esi, 0x41c = 0)
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00433d8d, FUN_00433d8d, "regs(eax:u32) -> (eax, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+state_00432330+state_0043c677+segw_0043c608+rows_00434044+state_0040e84e+state_0041104f+state_00432dcf+flags_00432dcf+carx_0042fe2f+state_00413f28+state_00433d8d+cars_00433d8d")
