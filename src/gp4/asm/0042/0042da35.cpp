#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (test / or / xor byte): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit `cmp left, right` (also `sub left, right`).
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

static inline uint8_t* bytesAt(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// Loads the arithmetic flags the next callee starts from.
static inline void loadFlags(gp4::Regs& c, uint32_t flags) {
    c.eflags = (c.eflags & ~0x8d5u) | (flags & 0x8d5u);
}

// FUNCTION: GP4 0x0042da35
// Decides whether car esi may follow/overtake car ebx given the input block edi: clears the three
// steering-state bytes 0x00604150..52, stores the car gap word (+0xe2 difference, or 0x0040dc8a when either
// car has bit 1 of +0x9c) at 0x00603e70, runs 0x0042b17e with eax = 0x160, and then either takes the quick
// cases (0x0042da03 steering byte, or the +0x94 - +0x90 byte) or tests the four side-probe callees
// (0x0042d4e6 with ecx 0x10 / 0x50, 0x0042d782, 0x0042d7de, 0x0042eee3, 0x0042d883), recording which ones
// passed in the bits 0x80 / 0x40 of 0x00604150..52. 0x00604156 = 0xff on success, 0 on failure.
void __cdecl FUN_0042da35(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00604151) = 0;
    GP4_GLOBAL(uint8_t, 0x00604152) = 0;
    GP4_GLOBAL(uint8_t, 0x00604150) = 0;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint8_t* esi = bytesAt(c.esi);
    uint8_t* ebx = bytesAt(c.ebx);
    uint32_t fl;

    // gap word
    const uint32_t selfBit = GP4_FIELD(uint8_t, esi, 0x9c) & 2u;
    const uint32_t otherBit = (selfBit != 0u) ? 0u : (GP4_FIELD(uint8_t, ebx, 0x9c) & 2u);
    fl = logicFlags8(selfBit != 0u ? selfBit : otherBit);
    if (selfBit == 0u && otherBit == 0u) {
        const uint32_t otherKey = GP4_FIELD(uint16_t, ebx, 0xe2);
        const uint32_t selfKey = GP4_FIELD(uint16_t, esi, 0xe2);
        c.eax = (c.eax & 0xffff0000u) | ((otherKey - selfKey) & 0xffffu);
        fl = cmpFlags16(otherKey, selfKey);
    } else {
        loadFlags(c, fl);
        gp4::call_regs(0x0040dc8a, c);
        fl = c.eflags;
    }
    GP4_GLOBAL(uint16_t, 0x00603e70) = static_cast<uint16_t>(c.eax);

    c.eax = 0x160;
    loadFlags(c, fl);
    gp4::call_regs(0x0042b17e, c);
    fl = c.eflags;

    esi = bytesAt(c.esi);
    ebx = bytesAt(c.ebx);

    bool success = true;        // reaches 0x0042db66
    bool probe = false;         // 0x0042dace: the side-probe sequence
    const uint32_t reverse = GP4_FIELD(uint8_t, esi, 0x179) & 0x80u;
    fl = logicFlags8(reverse);
    if (reverse == 0u) {
        const uint32_t lateral = GP4_FIELD(uint8_t, ebx, 0x122) & 8u;
        fl = logicFlags8(lateral);
        uint32_t passing = 0;
        if (lateral != 0u) {
            passing = GP4_FIELD(uint8_t, ebx, 0xe5) & 4u;
            fl = logicFlags8(passing);
        }
        if (passing != 0u) {
            loadFlags(c, fl);
            gp4::call_regs(0x0042da03, c);
            fl = c.eflags;
        } else {
            const uint32_t quick = GP4_FIELD(uint8_t, ebx, 0x203) & 1u;
            fl = logicFlags8(quick);
            if (quick != 0u) {
                const uint32_t high = GP4_FIELD(uint16_t, ebx, 0x94);
                const uint32_t low = GP4_FIELD(uint16_t, ebx, 0x90);
                const uint32_t dx = (high - low) & 0xffffu;
                fl = cmpFlags16(high, low);
                c.edx = (c.edx & 0xffff0000u) | dx;
                GP4_GLOBAL(uint8_t, 0x00604153) = static_cast<uint8_t>(dx >> 8);
                GP4_GLOBAL(uint8_t, 0x00604154) = static_cast<uint8_t>(dx >> 8);
            } else {
                probe = true;
            }
        }
    } else {
        probe = true;
    }

    if (probe) {
        c.ecx = 0x10;
        loadFlags(c, fl);
        gp4::call_regs(0x0042d4e6, c);
        fl = c.eflags;
        if ((c.eflags & 1u) != 0u) {
            success = false;
        } else {
            GP4_GLOBAL(uint8_t, 0x00604150) = 0x80;
            uint8_t* input = bytesAt(c.edi);
            uint32_t dl = GP4_FIELD(uint8_t, input, 0x10);
            c.edx = (c.edx & 0xffffff00u) | dl;
            GP4_GLOBAL(uint8_t, 0x00604153) = static_cast<uint8_t>(dl);
            gp4::call_regs(0x0042d782, c);
            fl = c.eflags;
            if ((c.eflags & 1u) == 0u) {
                GP4_GLOBAL(uint8_t, 0x00604152) = 0x80;
            }
            gp4::call_regs(0x0042d7de, c);
            fl = c.eflags;
            if ((c.eflags & 1u) == 0u) {
                GP4_GLOBAL(uint8_t, 0x00604151) = 0x80;
            }

            input = bytesAt(c.edi);
            dl = GP4_FIELD(uint8_t, input, 0x50);
            c.edx = (c.edx & 0xffffff00u) | dl;
            fl = cmpFlags8(dl, 4u);
            bool both = false;      // 0x0042db37: the mirrored probe
            bool fallThrough = (dl < 4u);   // 0x0042db27
            if (!fallThrough) {
                c.ecx = 0x50;
                loadFlags(c, fl);
                gp4::call_regs(0x0042d4e6, c);
                fl = c.eflags;
                if ((c.eflags & 1u) != 0u) {
                    fallThrough = true;
                } else {
                    GP4_GLOBAL(uint8_t, 0x00604150) |= 0x40;
                    fl = logicFlags8(GP4_GLOBAL(uint8_t, 0x00604150));
                    both = true;
                }
            }
            if (fallThrough) {
                const uint32_t armed = GP4_GLOBAL(uint8_t, 0x00604150) & 0x80u;
                fl = logicFlags8(armed);
                if (armed != 0u) {
                    loadFlags(c, fl);
                    gp4::call_regs(0x0042eee3, c);
                    fl = c.eflags;
                    both = (c.eflags & 1u) != 0u;
                }
            }
            if (both) {
                input = bytesAt(c.edi);
                dl = GP4_FIELD(uint8_t, input, 0x10) ^ 0x80u;
                c.edx = (c.edx & 0xffffff00u) | dl;
                fl = logicFlags8(dl);
                GP4_GLOBAL(uint8_t, 0x00604153) = static_cast<uint8_t>(dl);
                loadFlags(c, fl);
                gp4::call_regs(0x0042d782, c);
                fl = c.eflags;
                if ((c.eflags & 1u) == 0u) {
                    GP4_GLOBAL(uint8_t, 0x00604152) |= 0x40;
                    fl = logicFlags8(GP4_GLOBAL(uint8_t, 0x00604152));
                }
                loadFlags(c, fl);
                gp4::call_regs(0x0042d7de, c);
                fl = c.eflags;
                if ((c.eflags & 1u) == 0u) {
                    GP4_GLOBAL(uint8_t, 0x00604151) |= 0x40;
                    fl = logicFlags8(GP4_GLOBAL(uint8_t, 0x00604151));
                }
            }
            loadFlags(c, fl);
            gp4::call_regs(0x0042d883, c);
            fl = c.eflags;
            success = (c.eflags & 1u) == 0u;
        }
    }
    GP4_GLOBAL(uint8_t, 0x00604156) = success ? 0xffu : 0u;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (fl & 0x8d5u);
}
GP4_IMPL(0x0042da35, FUN_0042da35, "regs(eax:u32,ecx:u32,edx:u32,ebp:u32,edi:ptr[0xb1]:bytes,ebx:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677,esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677) -> (eax,ebx,ecx,edx,esi,edi,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+state_0042da35")
