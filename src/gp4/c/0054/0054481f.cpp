#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the pointer chain that starts at the cell 0x0084c711 five links deep (esi), clamps the count
// [0x0083157c] to 8 and stores `count - 1` at esi + 0x2a. A zero count instead clears that dword, runs the
// item-chain flag walk 0x0053e4db for eax = 0x1c over the chain head 0x0084c711 and sets the byte
// 0x007ab53c to 0xff. ecx is left 0; esi, eax and the flags are those of the path taken (`dec eax`, or the
// callee's last flag writer).
// FUNCTION: GP4 0x0054481f
void __cdecl FUN_0054481f(gp4::Regs* r) {
    uint32_t node = 0x0084c711u;
    for (uint32_t i = 5u; i != 0u; --i) {
        node = GP4_FIELD(uint32_t, node, 0);
    }

    uint32_t count = GP4_GLOBAL(uint32_t, 0x0083157c);
    if (count > 8u) {
        count = 8u;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.ecx = 0u;
    c.esi = node;
    if (count == 0u) {
        GP4_FIELD(uint32_t, node, 0x2a) = 0u;
        c.esi = 0x0084c711u;
        c.eax = 0x1cu;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(0u));
        gp4::call_regs(0x0053e4db, c);
        GP4_GLOBAL(uint8_t, 0x007ab53c) = 0xffu;
    } else {
        const uint32_t flags = gp4::flags::dec32(count, gp4::flags::logic32(count));
        c.eax = count - 1u;
        GP4_FIELD(uint32_t, node, 0x2a) = c.eax;
        c.eflags = gp4::flags::merge(c.eflags, flags);
    }
    r->eax = c.eax;
    r->ecx = c.ecx;
    r->esi = c.esi;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0054481f, FUN_0054481f, "regs() -> (eax, ecx, esi, cf, pf, af, zf, sf, of); gfields={0x0084c711:ptr[0x40]:{*bytes,0:ptr[0x40]:{*bytes,0:ptr[0x40]:{*bytes,0:ptr[0x40]:{*bytes,0:ptr[0x48]:{*bytes,0:u32{=0,0x838c94}}}}}}, 0x0083157c:u32{=0,1,2,8,9,0xffffffff}}")
