#include <gp4/gp4.h>
#include <gp4/flags.h>

// Ring-record interpolation driver for the run of 0x160-byte records [ebx, edi): publishes the run
// (first record 0x852ccc, end record 0x852cd4, record count 0x852d20 = max((edi - ebx) / 0x160, 0)), loads
// the start point (+0x100 distance, +0x104 angle, +0x108/+0x10a words << 6) of the first record into
// 0x852cac/0x852cb4/0x852cbc/0x852cc4 and the end point of the end record into 0x852c8c/0x852c94/
// 0x852c9c/0x852ca4, then runs the side-0 interpolation 0x00556791 (ebx = 0). The four globals it
// borrowed (0x852cd4, 0x852ccc, 0x852d20, 0x852d24) are restored; pushal/popal returns every register,
// so only the flags (the `or eax, eax` on the count, or the callee's when it runs) are an output.
// FUNCTION: GP4 0x00559e7d
void __cdecl FUN_00559e7d(gp4::Regs* r) {
    const uint32_t first = r->ebx;
    const uint32_t last = r->edi;

    const uint32_t savedCursor = GP4_GLOBAL(uint32_t, 0x00852d24);
    const uint32_t savedCount = GP4_GLOBAL(uint32_t, 0x00852d20);
    const uint32_t savedFirst = GP4_GLOBAL(uint32_t, 0x00852ccc);
    const uint32_t savedLast = GP4_GLOBAL(uint32_t, 0x00852cd4);

    GP4_GLOBAL(uint32_t, 0x00852ccc) = first;
    GP4_GLOBAL(uint32_t, 0x00852cd4) = last;

    const int32_t diff = static_cast<int32_t>(last - first);
    int32_t count = diff / 0x160;
    const int32_t rem = diff % 0x160;
    if (count < 0) {
        count = 0;
    }
    GP4_GLOBAL(int32_t, 0x00852d20) = count;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(static_cast<uint32_t>(count)));

    if (count != 0) {
        uint8_t* a = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(first));
        uint8_t* b = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(last));

        GP4_GLOBAL(uint32_t, 0x00852cac) = GP4_FIELD(uint32_t, a, 0x100);
        GP4_GLOBAL(uint32_t, 0x00852cb4) = GP4_FIELD(uint32_t, a, 0x104);
        GP4_GLOBAL(uint32_t, 0x00852cbc) = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, a, 0x108))) << 6;
        GP4_GLOBAL(uint32_t, 0x00852cc4) = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, a, 0x10a))) << 6;
        GP4_GLOBAL(uint32_t, 0x00852c8c) = GP4_FIELD(uint32_t, b, 0x100);
        GP4_GLOBAL(uint32_t, 0x00852c94) = GP4_FIELD(uint32_t, b, 0x104);
        GP4_GLOBAL(uint32_t, 0x00852c9c) = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, b, 0x108))) << 6;
        const uint32_t lastWord = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, b, 0x10a))) << 6;
        GP4_GLOBAL(uint32_t, 0x00852ca4) = lastWord;

        gp4::Regs c = *r;
        c.eax = lastWord;
        c.ebx = 0u;
        c.edx = static_cast<uint32_t>(rem);
        c.ebp = 0x160u;
        gp4::call_regs(0x00556791, c);
        r->eflags = c.eflags;
    }

    GP4_GLOBAL(uint32_t, 0x00852cd4) = savedLast;
    GP4_GLOBAL(uint32_t, 0x00852ccc) = savedFirst;
    GP4_GLOBAL(uint32_t, 0x00852d20) = savedCount;
    GP4_GLOBAL(uint32_t, 0x00852d24) = savedCursor;
}
GP4_IMPL(0x00559e7d, FUN_00559e7d, "regs(eax:u32, ebx:rec[0x8943a0,0x160,0..8], ecx:u32, edx:u32, esi:u32, edi:rel[ebx,0x160,-1..5], ebp:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=multiply_0040ce7b+lerp_00556791+ring_00555b28+geo_005558c6")
