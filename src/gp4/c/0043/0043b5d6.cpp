#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit `test` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

static inline uint32_t signExtend16(uint32_t word) {
    return static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(static_cast<uint16_t>(word))));
}

// Keeps the compiler from folding the record-relative offsets into absolute globals.
static __declspec(noinline) uint8_t* CarRecords() { return GP4_ARRAY(uint8_t, 0x0066fef0); }

// Bubble sort of the car-order list 0x0066fe3c (words k*0x41c, ended by a negative word) by the signed words +0x148
// (descending) and, when equal, +0x14a. Every swap stores the new list byte offsets in the records' byte +0x7d and,
// when [0x007c2ea9] is negative, flags a change in 0x0060415e; the pass restarts while the swap count byte is nonzero,
// and a flagged change runs 0x0043b689 at the end.
// FUNCTION: GP4 0x0043b5d6
void __cdecl FUN_0043b5d6(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint8_t* const records = CarRecords();
    const uint32_t keyBase = 0x00670038u - 0x0066fef0u;

    GP4_GLOBAL(uint8_t, 0x0060415e) = 0;
    bool finished = false;
    while (!finished) {
        uint32_t swaps = 0;
        c.ecx &= 0xffffff00u;
        c.edi = 0x0066fe3cu;
        c.ebx = signExtend16(*reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(c.edi)));
        c.edi += 2u;
        if ((c.ebx & 0x8000u) != 0u) {
            break;
        }

        for (;;) {
            // L5f3: the key of the element just read is the running maximum
            int16_t key = *reinterpret_cast<const int16_t*>(records + keyBase + c.ebx);
            c.eax = (c.eax & 0xffff0000u) | static_cast<uint16_t>(key);
            for (;;) {
                c.ebx = signExtend16(*reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(c.edi)));
                c.edi += 2u;
                if ((c.ebx & 0x8000u) != 0u) {
                    if ((c.ecx & 0xffu) != 0u) {
                        goto restart;
                    }
                    finished = true;
                    goto leave;
                }
                const int16_t other = *reinterpret_cast<const int16_t*>(records + keyBase + c.ebx);
                if (key > other) {
                    break;
                }
                if (key == other) {
                    c.esi = signExtend16(*reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(c.edi - 4u)));
                    const int16_t second = *reinterpret_cast<const int16_t*>(records + keyBase + 2u + c.esi);
                    const int16_t otherSecond = *reinterpret_cast<const int16_t*>(records + keyBase + 2u + c.ebx);
                    if (second >= otherSecond) {
                        break;
                    }
                }

                // swap the two list words and record the new positions
                c.esi = signExtend16(*reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(c.edi - 2u)));
                const uint32_t previous = signExtend16(*reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(c.edi - 4u)));
                *reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(c.edi - 2u)) = static_cast<uint16_t>(previous);
                *reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(c.edi - 4u)) = static_cast<uint16_t>(c.esi);
                c.esi += 0x0066fef0u;
                const uint32_t previousRecord = previous + 0x0066fef0u;
                c.edx = c.edi - 0x0066fe40u;
                *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi + 0x7du)) = static_cast<uint8_t>(c.edx);
                *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(previousRecord + 0x7du)) = static_cast<uint8_t>(c.edx + 2u);
                c.edx = (c.edx & 0xffffff00u) | ((c.edx + 2u) & 0xffu);
                if ((GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x80u) != 0u) {
                    GP4_GLOBAL(uint8_t, 0x0060415e) = 0x80u;
                }
                ++swaps;
                c.ecx = (c.ecx & 0xffffff00u) | (swaps & 0xffu);
            }
        }
    restart:;
    }

leave:
    {
        const uint32_t changed = GP4_GLOBAL(uint8_t, 0x0060415e);
        c.eflags = (c.eflags & ~0x8d5u) | testFlags8(changed);
        if (changed != 0u) {
            gp4::call_regs(0x0043b689, c);
        }
    }
    *r = c;
}
GP4_IMPL(0x0043b5d6, FUN_0043b5d6, "regs(eax:u32, ecx:u32, edx:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+list_0066fe3c+state_004059ef+carx_0042fe2f+lap_0043b482+state_0044334d")
