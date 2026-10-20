#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static __forceinline void leave(gp4::Regs* r, uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx,
                                uint32_t ebp, uint32_t edi, uint32_t flags) {
    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->edx = edx;
    r->ebp = ebp;
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// FUNCTION: GP4 0x00406b6c
// Walks the 0x160-byte segment ring forward from edi for [0x00868558] + 2 records (wrapping at the
// lap end, the pit start and the pit end like 0x00406c19). 0x004067ea classifies each record; a
// record that reports CF is remembered in [0x006278d4] / [0x006278d8] and the next record that
// does not report CF resolves it through 0x004067b7 (slot [0x00629f04], ebx = 0): CF = 0 or a zero
// slot ends the search with CF = 1, otherwise the candidate with the smallest [0x00629edc] is kept
// in [0x006278dc] / [0x006278e0] / [0x006278e4]. When the walk ends, the best candidate is returned
// (edi = its record, eax = its slot, CF = 1) or, when there is none, CF = 0.
void __cdecl FUN_00406b6c(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x006278d4) = 0;
    GP4_GLOBAL(int32_t, 0x006278dc) = 0x7fffffff;
    GP4_GLOBAL(uint32_t, 0x006278e4) = 0;
    uint32_t count = GP4_GLOBAL(uint16_t, 0x00868558) + 2u;
    GP4_GLOBAL(uint8_t, 0x00627949) = 0;

    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;
    uint32_t edi = r->edi;

    for (;;) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.eax = eax;
        call.ebx = ebx;
        call.ecx = count;
        call.edx = edx;
        call.ebp = ebp;
        call.edi = edi;
        gp4::call_regs(0x004067ea, call);
        eax = call.eax;
        edx = call.edx;

        if ((call.eflags & 1u) != 0u) {
            GP4_GLOBAL(uint8_t, 0x00627949) = 0xff;
            GP4_GLOBAL(uint32_t, 0x006278d4) = edi;
            GP4_GLOBAL(uint32_t, 0x006278d8) = eax;
        } else if (GP4_GLOBAL(uint8_t, 0x00627949) != 0u) {
            ebp = edi;
            edi = GP4_GLOBAL(uint32_t, 0x006278d4);
            eax = GP4_GLOBAL(uint32_t, 0x006278d8);
            GP4_GLOBAL(uint32_t, 0x00629f04) = eax;
            GP4_GLOBAL(uint32_t, 0x00629f0c) = edi;
            if (eax == 0u) {
                leave(r, eax, ebx, count, edx, ebp, edi, 0x45u);   // or eax, eax; stc
                return;
            }
            ebx = 0;
            gp4::Regs resolve = *r;
            resolve.st_in = 0;
            resolve.st_out = 0;
            resolve.eax = eax;
            resolve.ebx = ebx;
            resolve.ecx = count;
            resolve.edx = edx;
            resolve.ebp = ebp;
            resolve.edi = edi;
            gp4::call_regs(0x004067b7, resolve);
            eax = resolve.eax;
            if ((resolve.eflags & 1u) == 0u) {
                leave(r, eax, ebx, count, edx, ebp, edi, (resolve.eflags & 0x8d4u) | 1u);   // stc
                return;
            }
            edx = GP4_GLOBAL(uint32_t, 0x00629edc);
            if (static_cast<int32_t>(edx) < GP4_GLOBAL(int32_t, 0x006278dc)) {
                GP4_GLOBAL(uint32_t, 0x006278dc) = edx;
                GP4_GLOBAL(uint32_t, 0x006278e0) = eax;
                GP4_GLOBAL(uint32_t, 0x006278e4) = edi;
            }
            edi = ebp;
            GP4_GLOBAL(uint8_t, 0x00627949) = 0;
        }

        edi += 0x160u;
        if (edi >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
                edi -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else if (edi >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                edi = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }
        count -= 1u;
        if (count == 0u) break;
    }

    const uint32_t best = GP4_GLOBAL(uint32_t, 0x006278e4);
    if (best == 0u) {
        leave(r, eax, ebx, 0u, edx, ebp, edi, 0x44u);   // test ..., -1 of zero; clc
        return;
    }
    leave(r, GP4_GLOBAL(uint32_t, 0x006278e0), ebx, 0u, edx, ebp, best,
          parityFlag(best) | ((best & 0x80000000u) != 0u ? 0x80u : 0u) | 1u);   // test; stc
}
GP4_IMPL(0x00406b6c, FUN_00406b6c, "regs(edi:rec[0x8943a0,0x160,0..14]) -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_00406ac7+ring_0040aec5+state_004067ea+scan_00406d4b+state_0040bce6")
