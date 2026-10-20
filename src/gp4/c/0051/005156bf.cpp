#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original indexes the 4-byte records at 0x00893e10 with a register ([eax*4 + 0x893e1x]); the
// compiler would fold the field offsets into absolute addresses of other globals.
static __declspec(noinline) int8_t recordByte(const uint8_t* base, int32_t index) {
    return static_cast<int8_t>(base[index * 4]);
}

// Marks one entry of the 64-byte flag table 0x00866014 (index clamped to 0..0x3f).
static inline uint32_t markEntry(int32_t index) {
    int32_t i = index;
    if (i < 0) {
        i = 0;
    }
    const uint32_t arith = gp4::flags::sub32(static_cast<uint32_t>(i), 0x3fu);
    if (i > 0x3f) {
        i = 0x3f;
    }
    GP4_ARRAY(uint8_t, 0x00866014)[i] = 1;
    return arith;
}

// pushal/popal handler (entry 14 of the mode table 0x00817270 used by 0x005141b8). Saves the bytes
// 0x0088fd96/0x0088fd97 (restored at the end). When byte 0x00893cf9 is 0xff it runs 0x0051d160,
// clears it and follows the 4-byte record [0x0088ff38] (field +2: another record, whose field +1
// is a flag index) to mark 0x00866014[index]. Then, when field +0 of record [0x0088ff38] is not
// 0xff, it runs 0x0051d160 again (eax = the record index) and marks the record's own field +1.
// Bytes 0x00893cf6, 0x00893cf7 and 0x00954169 become 0xff. popal restores every register; the
// flags are those of the last compare executed.
// FUNCTION: GP4 0x005156bf
void __cdecl FUN_005156bf(gp4::Regs* r) {
    const uint8_t saved96 = GP4_GLOBAL(uint8_t, 0x0088fd96);
    const uint8_t saved97 = GP4_GLOBAL(uint8_t, 0x0088fd97);

    if (GP4_GLOBAL(uint8_t, 0x00893cf9) == 0xffu) {
        gp4::Regs c = *r;
        c.eax = (r->eax & 0xffff0000u) | (static_cast<uint32_t>(saved97) << 8) | saved96;
        c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(0xffu, 0xffu));
        gp4::call_regs(0x0051d160, c);
        GP4_GLOBAL(uint8_t, 0x00893cf9) = 0;
        const int32_t link = recordByte(GP4_ARRAY(uint8_t, 0x00893e12), GP4_GLOBAL(int32_t, 0x0088ff38));
        if (link != -1) {
            const int32_t index = recordByte(GP4_ARRAY(uint8_t, 0x00893e11), link);
            if (index != -1) {
                markEntry(index);
            }
        }
    }

    uint32_t arith;
    const int32_t current = GP4_GLOBAL(int32_t, 0x0088ff38);
    const uint8_t head = static_cast<uint8_t>(recordByte(GP4_ARRAY(uint8_t, 0x00893e10), current));
    arith = gp4::flags::sub8(head, 0xffu);
    if (head != 0xffu) {
        gp4::Regs c = *r;
        c.eax = static_cast<uint32_t>(current);
        c.eflags = gp4::flags::merge(r->eflags, arith);
        gp4::call_regs(0x0051d160, c);
        const int32_t index = recordByte(GP4_ARRAY(uint8_t, 0x00893e11), GP4_GLOBAL(int32_t, 0x0088ff38));
        arith = gp4::flags::sub32(static_cast<uint32_t>(index), 0xffffffffu);
        if (index != -1) {
            arith = markEntry(index);
        }
    }

    GP4_GLOBAL(uint8_t, 0x0088fd96) = saved96;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = saved97;
    GP4_GLOBAL(uint8_t, 0x00893cf6) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00893cf7) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005156bf, FUN_005156bf, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=layout=player_0040f8d9+disp_005141b8+recs_005156bf")
