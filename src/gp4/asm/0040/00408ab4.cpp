#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp value, 0`: CF = OF = AF = 0.
static inline uint32_t cmpZeroFlags32(uint32_t value) {
    return parityFlag8(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

static inline uint8_t* bytesAt(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x00408ab4
// Level-of-detail style re-selection for the record at esi: [0x601fb4] = byte +0x70 - 1 (>= 0), [0x601fb0]
// = min(byte +0x198, 3) rows, [0x601fb8] = the start row byte +0x136, [0x601fbc] = rows - start. While there
// are rows left it looks the start row up with 0x408936 (stored at 0x601fc4), compares it with the level, and
// for a gap above 3 either blends with 0x408998 (byte +0xe5 bit 2, or the 0x43bb15 code from +0x7c says so) or
// takes the next row; otherwise the fallback is the word 0x7c2e96. The chosen value minus the level, clamped
// at 0, plus 3, times 64 becomes the word +0x60; 0x408783 refreshes the derived words and 0x602b50 receives
// the clamped growth of that word.
void __cdecl FUN_00408ab4(gp4::Regs* r) {
    uint8_t* const esi = bytesAt(r->esi);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(int32_t, 0x00602b50) = 0;

    int32_t level = static_cast<int32_t>(GP4_FIELD(uint8_t, esi, 0x70)) - 1;
    if (level < 0) {
        level = 0;
    }
    GP4_GLOBAL(int32_t, 0x00601fb4) = level;

    int32_t rows = static_cast<int32_t>(GP4_FIELD(uint8_t, esi, 0x198));
    if (rows > 3) {
        rows = 3;
    }
    GP4_GLOBAL(int32_t, 0x00601fb0) = rows;
    c.ecx = static_cast<uint32_t>(rows);

    bool fallback = true;       // 0x00408bc3
    if (rows != 0) {
        const int32_t start = static_cast<int32_t>(GP4_FIELD(uint8_t, esi, 0x136));
        GP4_GLOBAL(int32_t, 0x00601fb8) = start;
        const int32_t left = rows - start;
        c.ecx = static_cast<uint32_t>(left);
        if (left > 0) {
            fallback = false;
            GP4_GLOBAL(int32_t, 0x00601fbc) = left;

            c.eax = GP4_GLOBAL(uint32_t, 0x00601fb8);
            gp4::call_regs(0x00408936, c);
            GP4_GLOBAL(uint32_t, 0x00601fc4) = c.eax;
            c.eax = GP4_GLOBAL(uint32_t, 0x00601fb8);
            gp4::call_regs(0x00408936, c);

            bool step = true;       // 0x00408b94: look up the next row and advance the start byte
            bool far = false;       // 0x00408bb7: look up the start row again
            if (static_cast<int32_t>(c.eax) != GP4_GLOBAL(int32_t, 0x00601fb4)) {
                int32_t gap = static_cast<int32_t>(c.eax - static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00601fb4)));
                if (gap < 0) {
                    gap = static_cast<int32_t>(0u - static_cast<uint32_t>(gap));
                }
                c.eax = static_cast<uint32_t>(gap);
                if (gap > 3) {
                    c.ebp = static_cast<uint32_t>(gap);
                    bool blend = (GP4_FIELD(uint8_t, esi, 0xe5) & 4u) != 0u;
                    if (!blend) {
                        int32_t code = static_cast<int32_t>(GP4_FIELD(uint8_t, esi, 0x7c) & 0x3fu) - 1;
                        if (code > 0x15) {
                            code = 0x15;
                        }
                        c.eax = static_cast<uint32_t>(code);
                        gp4::call_regs(0x0043bb15, c);
                        blend = GP4_GLOBAL(uint8_t, 0x00601faa) != 0;
                        if (!blend && gap >= 8) {
                            far = true;
                            step = false;
                        }
                    }
                    if (blend) {
                        gp4::call_regs(0x00408998, c);
                        if ((c.eflags & 1u) == 0u) {
                            step = false;
                        }
                    }
                }
            }
            if (far) {
                c.eax = GP4_GLOBAL(uint32_t, 0x00601fb8);
                gp4::call_regs(0x00408936, c);
            } else if (step) {
                c.eax = GP4_GLOBAL(uint32_t, 0x00601fb8) + 1u;
                gp4::call_regs(0x00408936, c);
                uint8_t advanced = static_cast<uint8_t>(GP4_FIELD(uint8_t, esi, 0x136) + 1u);
                GP4_FIELD(uint8_t, esi, 0x136) = advanced;
                if (static_cast<int8_t>(advanced) > 3) {
                    GP4_FIELD(uint8_t, esi, 0x136) = 3;
                }
            }
        }
    }
    if (fallback) {
        c.eax = GP4_GLOBAL(uint16_t, 0x007c2e96);
    }

    int32_t value = static_cast<int32_t>(c.eax - static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00601fb4)));
    if (value < 0) {
        value = 0;
    }
    value = static_cast<int32_t>((static_cast<uint32_t>(value) + 3u) << 6);
    c.eax = static_cast<uint32_t>(value);
    c.edx = (c.edx & 0xffff0000u) | GP4_FIELD(uint16_t, esi, 0x60);
    GP4_FIELD(uint16_t, esi, 0x60) = static_cast<uint16_t>(value);
    gp4::call_regs(0x00408783, c);

    c.edx = 0u - (c.edx & 0xffffu);
    c.eax = c.eax + c.edx;
    const uint32_t flags = cmpZeroFlags32(c.eax);
    if (static_cast<int32_t>(c.eax) < 0) {
        c.eax = 0;
    }
    GP4_GLOBAL(uint32_t, 0x00602b50) = c.eax;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408ab4, FUN_00408ab4, "regs(esi:ptr[0x1d0]:u8[0..40]) -> (eax,ecx,edx,ebp,cf,pf,af,zf,sf,of); cases=2000; globals=fuzz:i32+layout=state_004088c2+state_0066d8c0+state_00408998+trk_007ad894+circ_007adb50+codes_0043ba12+state_004087aa+gain_00408790")
