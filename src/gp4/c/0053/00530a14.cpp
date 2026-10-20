#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline void SetFlags(gp4::Regs* r, uint32_t arith) {
    r->eflags = gp4::flags::merge(r->eflags, arith);
}

// Parses up to seven "<slot> <value>" decimal pairs from the text at esi into the record at ebx. The first number may
// carry bit 0x80 (kept as marker: bit clear stores 0xff in the byte at 0x82741e, bit set re-reads the slot number
// after masking with 0x7f). A slot above 4, a missing number or a value above 0x10000 marks the record with
// byte +5 = 1 and stops. Otherwise the value goes to the dword at +0x28 + slot*4 and the slot's flag byte from
// 0x8273de is or-ed into byte +6. After a normal end (end of text or seven pairs) with at least one pair parsed,
// the slot-order scanners 0x00530d28/0x00530d52 pick the dwords +0xc and +0x10, swapped when +0xc is above +0x10.
// FUNCTION: GP4 0x00530a14
void __cdecl FUN_00530a14(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    r->ecx = 0u;
    for (;;) {
        gp4::call_regs(0x0054c53f, *r);
        gp4::call_regs(0x0054c495, *r);
        if ((r->eflags & gp4::flags::CF) != 0u) {
            break;
        }
        SetFlags(r, gp4::flags::logic32(r->ecx));
        if (r->ecx == 0u) {
            SetFlags(r, gp4::flags::logic32(r->eax & 0x80u));
            if ((r->eax & 0x80u) == 0u) {
                GP4_GLOBAL(uint8_t, 0x0082741e) = 0xffu;
            } else {
                r->eax &= 0x7fu;
                SetFlags(r, gp4::flags::logic32(r->eax));
                gp4::call_regs(0x0054c53f, *r);
                gp4::call_regs(0x0054c495, *r);
                if ((r->eflags & gp4::flags::CF) != 0u) {
                    break;
                }
            }
        }
        SetFlags(r, gp4::flags::sub32(r->eax, 4u));
        if (r->eax > 4u) {
            rec[5] = 1u;
            return;
        }
        r->edx = r->eax;
        gp4::call_regs(0x0054c53f, *r);
        gp4::call_regs(0x0054c495, *r);
        if ((r->eflags & gp4::flags::CF) != 0u) {
            rec[5] = 1u;
            return;
        }
        SetFlags(r, gp4::flags::sub32(r->eax, 0x10000u));
        if (r->eax > 0x10000u) {
            rec[5] = 1u;
            return;
        }
        GP4_FIELD(uint32_t, rec, 0x28 + r->edx * 4u) = r->eax;
        const uint32_t slotFlags = GP4_ARRAY(uint8_t, 0x008273de)[r->edx];
        r->eax = (r->eax & 0xffffff00u) | slotFlags;
        const uint32_t merged = rec[6] | slotFlags;
        rec[6] = static_cast<uint8_t>(merged);
        SetFlags(r, gp4::flags::logic8(merged));
        SetFlags(r, gp4::flags::inc32(r->ecx, r->eflags));
        r->ecx += 1u;
        SetFlags(r, gp4::flags::sub32(r->ecx, 7u));
        if (r->ecx == 7u) {
            break;
        }
    }

    SetFlags(r, gp4::flags::logic32(r->ecx));
    if (r->ecx != 0u) {
        gp4::call_regs(0x00530d28, *r);
        gp4::call_regs(0x00530d52, *r);
        r->eax = GP4_FIELD(uint32_t, rec, 0xc);
        SetFlags(r, gp4::flags::sub32(r->eax, GP4_FIELD(uint32_t, rec, 0x10)));
        if (r->eax > GP4_FIELD(uint32_t, rec, 0x10)) {
            r->edx = GP4_FIELD(uint32_t, rec, 0x10);
            GP4_FIELD(uint32_t, rec, 0xc) = r->edx;
            GP4_FIELD(uint32_t, rec, 0x10) = r->eax;
        }
    }
}
GP4_IMPL(0x00530a14, FUN_00530a14, "regs(esi:ptr[0x40]:layout=text_00530a14, ebx:ptr[0x440]:u32{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3}, edx:u32) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
