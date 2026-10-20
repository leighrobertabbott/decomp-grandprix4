#include <gp4/gp4.h>
#include <gp4/flags.h>

// Decodes the first byte of the order list at 0x00675958: bit 7 is the "flag" (0xff/0 stored at
// 0x008222ee), the low six bits minus one index the 22 dword keys at 0x00675970. When the index
// is below 0x16 the index goes to the word 0x007c2f1e and the key to the dword 0x007c2f24.
// Hand-written register code: dl = the 0xff/0 flag (edx high bits kept); eax = the index
// ((b & 0x3f) - 1) or, when it is in range, the key; esi = &key table[index] only in range
// (otherwise unchanged); flags are those of the last instruction, `add esi, 0x675970` in range
// and `cmp eax, 0x16` out of range.
// FUNCTION: GP4 0x00548e2f
void __cdecl FUN_00548e2f(gp4::Regs* r) {
    uint32_t b = GP4_GLOBAL(uint8_t, 0x00675958);
    uint8_t flag = (b & 0x80u) ? 0xffu : 0u;
    GP4_GLOBAL(uint8_t, 0x008222ee) = flag;
    uint32_t idx = (b & 0x3fu) - 1u;

    r->edx = (r->edx & 0xffffff00u) | flag;
    if (idx < 0x16u) {
        GP4_GLOBAL(uint16_t, 0x007c2f1e) = (uint16_t)idx;
        uint32_t addr = (idx << 2) + 0x00675970u;
        uint32_t key = GP4_GLOBAL(uint32_t, addr);
        GP4_GLOBAL(uint32_t, 0x007c2f24) = key;
        r->esi = addr;
        r->eax = key;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(idx << 2, 0x00675970u));
    } else {
        r->eax = idx;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(idx, 0x16u));
    }
}
GP4_IMPL(0x00548e2f, FUN_00548e2f, "regs(edx:u32, esi:u32) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=layout=order_00675958")
