#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit `sub left, right`.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | ((((left ^ right) & (left ^ result)) >> 4) & 0x800u);
}

static inline void setLow16(uint32_t& reg, uint32_t value) {
    reg = (reg & 0xffff0000u) | (value & 0xffffu);
}

static inline uint8_t* at(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

static inline void call(uint32_t addr, gp4::Regs& c) {
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(addr, c);
}

// FUNCTION: GP4 0x00408d6d
// Race-start placement of the 22 cars: starting at the segment record returned by 0x0051f1da (edi),
// each car record (0x66fef0 + k*0x41c) is initialised by 0x00408c3e (and 0x00408742 outside the
// negative mode of byte 0x7c2ea9), and the grid offset word 0x6f2c20 steps back by 0x6f2c22, moving
// edi one 0x160-byte segment back per 0x400 (wrapping below 0x891504 by 0x866ec4, or clamping at
// 0x891500). Then 0x0043b51c runs for the last car and, for the 22 cars of the order list 0x66fe3c,
// the lap distance +0x68 (segment word +0x1a & 0x7ff, through 0x00405985 when bit 1 of +0x8d is set,
// wrapped by 0x868558) and +0xe2 = (+0x68 << 6) + lane are set before 0x0040db6c. The flags are those
// of the final `sub word [0x62ad64], 1`.
void __cdecl FUN_00408d6d(gp4::Regs* r) {
    gp4::Regs c = *r;

    GP4_GLOBAL(uint16_t, 0x0066d88a) = 0u;
    call(0x0051f1da, c);
    c.edi = c.eax;
    c.esi = 0x0066fef0u;
    GP4_GLOBAL(uint16_t, 0x0062ad64) = 0u;
    do {
        if (GP4_GLOBAL(int8_t, 0x007c2ea9) < 0) {
            c.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x007c2eba)));
            if ((GP4_GLOBAL(uint8_t, 0x0062ad64) & 1u) != 0u) {
                c.eax = 0u - c.eax;
            }
            c.ecx = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x006f2c20)));
            c.edx = 0u;
            call(0x00408c3e, c);
            if (GP4_GLOBAL(int8_t, 0x007c2ea9) < 0) {
                uint8_t* car = at(c.esi);
                car[0xce] = static_cast<uint8_t>(car[0xce] | 0x80u);
            }
        } else {
            c.eax = 0u;
            c.ecx = 0u;
            c.edx = 0u;
            call(0x00408c3e, c);
            call(0x00408742, c);
        }

        uint16_t offset = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x006f2c20) - GP4_GLOBAL(uint16_t, 0x006f2c22));
        while (static_cast<int16_t>(offset) < 0) {
            offset = static_cast<uint16_t>(offset + 0x400u);
            if (c.edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
                c.edi -= 0x160u;
                if (c.edi < GP4_GLOBAL(uint32_t, 0x00891504)) {
                    c.edi += GP4_GLOBAL(uint32_t, 0x00866ec4);
                }
            } else {
                c.edi -= 0x160u;
                if (c.edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    c.edi = GP4_GLOBAL(uint32_t, 0x00891500);
                }
            }
        }
        setLow16(c.eax, offset);
        GP4_GLOBAL(uint16_t, 0x006f2c20) = offset;
        c.esi += 0x41cu;
        GP4_GLOBAL(uint16_t, 0x0062ad64) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x0062ad64) + 1u);
    } while (GP4_GLOBAL(uint16_t, 0x0062ad64) != 0x16u);

    c.ebx = 0x0067553cu;
    call(0x0043b51c, c);
    c.ebx = 0x0066fe3cu;
    GP4_GLOBAL(uint16_t, 0x0062ad64) = 0x15u;
    uint32_t flags;
    for (;;) {
        c.esi = static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(at(c.ebx)))) + 0x0066fef0u;
        c.ebx += 2u;
        uint8_t* car = at(c.esi);
        c.edi = *reinterpret_cast<const uint32_t*>(car + 0x1c);
        const uint16_t position = static_cast<uint16_t>(*reinterpret_cast<const uint16_t*>(at(c.edi) + 0x1a) & 0x7ffu);
        const uint16_t lane = static_cast<uint16_t>(static_cast<int16_t>(static_cast<int8_t>(car[0x67])));
        setLow16(c.eax, position);
        setLow16(c.ecx, lane);
        car[0x21d] = static_cast<uint8_t>(car[0x21d] & 0xfbu);
        if ((car[0x8d] & 2u) != 0u) {
            car[0x21d] = static_cast<uint8_t>(car[0x21d] | 4u);
            call(0x00405985, c);
            if (static_cast<int16_t>(c.eax) >= GP4_GLOBAL(int16_t, 0x00868558)) {
                setLow16(c.eax, c.eax - GP4_GLOBAL(uint16_t, 0x00868558));
            }
        }
        car = at(c.esi);
        uint16_t lap = static_cast<uint16_t>(static_cast<uint16_t>(c.eax) - GP4_GLOBAL(uint16_t, 0x0066d88a));
        if (static_cast<int16_t>(lap) < 0) {
            lap = static_cast<uint16_t>(lap + GP4_GLOBAL(uint16_t, 0x00868558));
        }
        if (static_cast<int16_t>(lap) >= GP4_GLOBAL(int16_t, 0x00868558)) {
            lap = static_cast<uint16_t>(lap - GP4_GLOBAL(uint16_t, 0x00868558));
        }
        *reinterpret_cast<uint16_t*>(car + 0x68) = lap;
        const uint16_t key = static_cast<uint16_t>((lap << 6) + static_cast<uint16_t>(c.ecx));
        setLow16(c.eax, key);
        *reinterpret_cast<uint16_t*>(car + 0xe2) = key;
        call(0x0040db6c, c);

        const uint16_t count = GP4_GLOBAL(uint16_t, 0x0062ad64);
        flags = subFlags16(count, 1u);
        GP4_GLOBAL(uint16_t, 0x0062ad64) = static_cast<uint16_t>(count - 1u);
        if ((static_cast<uint16_t>(count - 1u) & 0x8000u) != 0u) {
            break;
        }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408d6d, FUN_00408d6d, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); cases=200; maxinsn=40000000; wall=3600; globals=layout=cars_0066fef0+list_0066fe3c+lanes_0040da62+pend_0040daeb+state_00412af5+state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+state_004094e3+div_00403bb6+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+curves_0062ecb0+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+scale_0066ec60+keys_00408c3e+start_004098fd+grid_00408d6d")
