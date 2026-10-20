#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original calls 0x00530022 from two separate places (the fill arm and the clear arm); keep one
// call site per arm.
static __declspec(noinline) uint32_t RebindAfterFill(gp4::Regs* c, uint32_t flags) {
    c->eflags = gp4::flags::merge(c->eflags, flags);
    gp4::call_regs(0x00530022, *c);
    return c->eflags;
}

static __declspec(noinline) uint32_t RebindAfterClear(gp4::Regs* c, uint32_t flags) {
    c->eflags = gp4::flags::merge(c->eflags, flags);
    gp4::call_regs(0x00530022, *c);
    return c->eflags;
}

// Twin of 0x0053026a for the second state-word pair of the current render-state record ebx (selected by
// 0x00531803): the record is "full" when (dword +0x10 & 0x99e7ff3c) == 0x99e7ff3c and
// (dword +0x14 & 0x14) == 0x14. The mode is in edx and the flag byte at esi + 0x1f.
//   edx == 1: bit 7 of the flag byte := full.
//   edx == 2: a full record is cleared (dword +0x10 &= 0x661800c3, dword +0x14 &= ~0x14, bit 7 cleared),
//             any other record is filled (dword +0x10 |= 0x99e7ff3c, dword +0x14 |= 0x14, bit 7 set);
//             either way the static menu items are rebound by 0x00530022.
// edx is cleared on exit; eax keeps the last masked value and the flags are those of the last flag writer.
// FUNCTION: GP4 0x005302d9
void __cdecl FUN_005302d9(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00531803, c);

    uint32_t* rec = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(c.ebx));
    uint8_t* flagByte = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi + 0x1fu));
    uint32_t eax = c.eax;
    uint32_t flags;
    if (c.edx == 1u) {
        *flagByte = static_cast<uint8_t>(*flagByte & 0x7fu);
        flags = gp4::flags::logic8(*flagByte);
        eax = rec[4] & 0x99e7ff3cu;
        flags = gp4::flags::sub32(eax, 0x99e7ff3cu);
        if (eax == 0x99e7ff3cu) {
            eax = rec[5] & 0x14u;
            flags = gp4::flags::sub32(eax, 0x14u);
            if (eax == 0x14u) {
                *flagByte = static_cast<uint8_t>(*flagByte | 0x80u);
                flags = gp4::flags::logic8(*flagByte);
            }
        }
    } else {
        flags = gp4::flags::sub32(c.edx, 2u);
        if (c.edx == 2u) {
            bool full = false;
            eax = rec[4] & 0x99e7ff3cu;
            flags = gp4::flags::sub32(eax, 0x99e7ff3cu);
            if (eax == 0x99e7ff3cu) {
                eax = rec[5] & 0x14u;
                flags = gp4::flags::sub32(eax, 0x14u);
                full = (eax == 0x14u);
            }
            if (full) {
                rec[4] &= 0x661800c3u;
                flags = gp4::flags::logic32(rec[4]);
                rec[5] &= 0xffffffebu;
                flags = gp4::flags::logic32(rec[5]);
                *flagByte = static_cast<uint8_t>(*flagByte & 0x7fu);
                flags = gp4::flags::logic8(*flagByte);
                flags = RebindAfterClear(&c, flags);
            } else {
                rec[4] |= 0x99e7ff3cu;
                flags = gp4::flags::logic32(rec[4]);
                rec[5] |= 0x14u;
                flags = gp4::flags::logic32(rec[5]);
                *flagByte = static_cast<uint8_t>(*flagByte | 0x80u);
                flags = gp4::flags::logic8(*flagByte);
                flags = RebindAfterFill(&c, flags);
            }
        }
    }
    c.eax = eax;
    c.edx = 0u;
    *r = c;
    r->eflags = gp4::flags::merge(c.eflags, flags);
}
GP4_IMPL(0x005302d9, FUN_005302d9, "regs(eax:u32, edx:u32{=0,1,2,3}, esi:ptr[0x40]:bytes) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=sel_00531859+rsrec_007ad638")
