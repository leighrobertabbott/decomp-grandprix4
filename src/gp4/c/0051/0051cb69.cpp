#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; resets the five dwords 0x88fe18..0x88fe28 to -1, then (only when the byte 0x7ad869 is 0)
// copies the first dword of the 0x6975ec entry named by slot words 0x340, 0x33f, 0x341, 0x44c and
// 0x44d of 0x9e1b20 (0xffff = no entry, which stops the chain). popal restores every register but
// leaves the flags of the last flag writer (the cmp of an empty slot, else the add forming the
// entry address).
static __declspec(noinline) int load_slot(uint32_t index, uint32_t* dst, uint32_t* fl) {
    uint32_t off = GP4_ARRAY(uint16_t, 0x009e1b20)[index];
    if (off == 0xffffu) {
        *fl = gp4::flags::sub16(off, 0xffffu);
        return 0;
    }
    *fl = gp4::flags::add32(off, 0x006975ecu);
    *dst = *reinterpret_cast<uint32_t*>(GP4_ARRAY(uint8_t, 0x006975ec) + off);
    return 1;
}

// FUNCTION: GP4 0x0051cb69
void __cdecl FUN_0051cb69(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0088fe1c) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x0088fe18) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x0088fe20) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x0088fe24) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x0088fe28) = 0xffffffffu;
    uint8_t hw = GP4_GLOBAL(uint8_t, 0x007ad869);
    uint32_t fl = gp4::flags::sub8(hw, 0);
    if (hw == 0) {
        if (load_slot(0x340, &GP4_GLOBAL(uint32_t, 0x0088fe1c), &fl) &&
            load_slot(0x33f, &GP4_GLOBAL(uint32_t, 0x0088fe18), &fl) &&
            load_slot(0x341, &GP4_GLOBAL(uint32_t, 0x0088fe20), &fl) &&
            load_slot(0x44c, &GP4_GLOBAL(uint32_t, 0x0088fe24), &fl)) {
            load_slot(0x44d, &GP4_GLOBAL(uint32_t, 0x0088fe28), &fl);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, fl);
}
GP4_IMPL(0x0051cb69, FUN_0051cb69, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=dw_00532d48+slots_0051cb69")
