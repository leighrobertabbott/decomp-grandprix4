#include <gp4/gp4.h>

// pushal ... popal routine reading a stream of 16-bit words at esi: four words go to the dwords at
// 0x00602098, then a word that is 0xffff (stop, flags of the cmp = equal) or the first of two dwords
// 0x0062d284 / 0x0062d288, then a segment index (record of the 0x160-byte array at 0x008943a0) that
// becomes edi. Up to 0x3b pairs (bx, segment index) follow: each pair runs 0x0040cceb, which fills the
// byte +0x65 of the records [edi, ebp) with bl and ends in clc; the next record is then the start of the
// following range. A segment index of 0xffff ends the stream with ebp = [0x00866ecc] instead.
// Registers are restored by popal; the flags are those the last 0x0040cceb left (CF clear), or the
// `cmp ax, -1` flags of an immediate 0xffff.
// FUNCTION: GP4 0x0040cd00
void __cdecl FUN_0040cd00(gp4::Regs* r) {
    const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));

    for (uint32_t slot = 0; slot != 4; ++slot) {
        GP4_ARRAY(uint32_t, 0x00602098)[slot] = *words++;
    }

    const uint16_t head = *words++;
    if (head == 0xffffu) {
        // cmp ax, -1 with equal operands: ZF and PF set
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }
    GP4_GLOBAL(uint32_t, 0x0062d284) = head;
    GP4_GLOBAL(uint32_t, 0x0062d288) = *words++;

    uint32_t start = 0x008943a0u + static_cast<uint32_t>(*words++) * 0x160u;   // edi

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    for (uint32_t remaining = 0x3b; remaining != 0; --remaining) {
        const uint16_t fill = *words++;                       // bx
        const uint16_t next = *words++;
        call.ebx = (r->ebx & 0xffff0000u) | fill;
        call.edi = start;

        if (next == 0xffffu) {
            call.ebp = GP4_GLOBAL(uint32_t, 0x00866ecc);
            gp4::call_regs(0x0040cceb, call);
            break;
        }

        const uint32_t end = 0x008943a0u + static_cast<uint32_t>(next) * 0x160u;
        call.ebp = end;
        gp4::call_regs(0x0040cceb, call);
        if (call.eflags & 1u) {
            break;                                            // jb (never: 0x0040cceb ends in clc)
        }
        start = end;
    }

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0040cd00, FUN_0040cd00, "regs(esi:ptr[0x100]:layout=stream_0040cd00, ebx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0")
