#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic op (AND / TEST): CF = OF = AF = 0.
static inline void setLogicFlags8(gp4::Regs* r, uint8_t res) {
    r->eflags = (r->eflags & ~0x8d5u) | parityFlag(res) | (res == 0u ? 0x40u : 0u) | (res & 0x80u);
}

// esi = car record, edi = word pointer used by the callees. Clears the steering/overtake bits
// 0x9f of the byte at +0x6f (six `and`s, net `&= 0x60`), then - unless +0x26b bit 3 or +0x25f bit 1
// is set - runs 0x0041464c (when +0x161 bit 1 is clear; carry set continues, carry clear skips),
// clears bit 1 of +0x203 and runs 0x004147d6 and 0x0041499b. 0x00413dd5 always runs; a carry out of
// it stores 0x80 in the byte at 0x00638d1c. Registers and flags are whatever the callees leave.
// FUNCTION: GP4 0x004149f4
void __cdecl FUN_004149f4(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    rec[0x6f] &= 0x60u;
    setLogicFlags8(r, rec[0x6f]);

    do {
        uint8_t t = static_cast<uint8_t>(rec[0x26b] & 8u);
        setLogicFlags8(r, t);
        if (t != 0u) break;

        t = static_cast<uint8_t>(rec[0x25f] & 2u);
        setLogicFlags8(r, t);
        if (t != 0u) break;

        t = static_cast<uint8_t>(rec[0x161] & 2u);
        setLogicFlags8(r, t);
        if (t == 0u) {
            gp4::call_regs(0x0041464c, *r);
            if ((r->eflags & 1u) == 0u) break;      // jae
        }

        rec[0x203] &= 0xfdu;
        setLogicFlags8(r, rec[0x203]);
        gp4::call_regs(0x004147d6, *r);
        gp4::call_regs(0x0041499b, *r);
    } while (false);

    gp4::call_regs(0x00413dd5, *r);
    if (r->eflags & 1u) {
        GP4_GLOBAL(uint8_t, 0x00638d1c) = 0x80;
    }
}
GP4_IMPL(0x004149f4, FUN_004149f4, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x35c]:layout=rec_00413dd5+rec_00414562+car_004149f4, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+words_008943a0+geom_008943a0+state_00413dd5+state_00413d01+list_0066fe3c+state_00413f28+state_0041416e+state_00426091+cache_00426091+opt_00625a53")
