#include <gp4/gp4.h>
#include <gp4/flags.h>

// Twin of 0x00559e7d for the second half of the ring records: the run is [esi, edi) (0x160-byte records),
// the points come from +0x10c distance, +0x110 angle, +0x114/+0x116 words << 6 of the first and the end
// record (arrays 0x852cb0.. and 0x852c90..), and the interpolation 0x00556791 runs on side ebx = 1.
// The borrowed globals (0x852cd4, 0x852ccc, 0x852d20, 0x852d24) are restored; pushal/popal returns every
// register, so only the flags (`or eax, eax` on the record count, or the callee's) are an output.
// FUNCTION: GP4 0x00559f4d
void __cdecl FUN_00559f4d(gp4::Regs* r) {
    const uint32_t first = r->esi;
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

        GP4_GLOBAL(uint32_t, 0x00852cb0) = GP4_FIELD(uint32_t, a, 0x10c);
        GP4_GLOBAL(uint32_t, 0x00852cb8) = GP4_FIELD(uint32_t, a, 0x110);
        GP4_GLOBAL(uint32_t, 0x00852cc0) = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, a, 0x114))) << 6;
        GP4_GLOBAL(uint32_t, 0x00852cc8) = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, a, 0x116))) << 6;
        GP4_GLOBAL(uint32_t, 0x00852c90) = GP4_FIELD(uint32_t, b, 0x10c);
        GP4_GLOBAL(uint32_t, 0x00852c98) = GP4_FIELD(uint32_t, b, 0x110);
        GP4_GLOBAL(uint32_t, 0x00852ca0) = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, b, 0x114))) << 6;
        const uint32_t lastWord = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, b, 0x116))) << 6;
        GP4_GLOBAL(uint32_t, 0x00852ca8) = lastWord;

        gp4::Regs c = *r;
        c.eax = lastWord;
        c.ebx = 1u;
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
GP4_IMPL(0x00559f4d, FUN_00559f4d, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:rec[0x8943a0,0x160,0..8], edi:rel[esi,0x160,-1..5], ebp:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=multiply_0040ce7b+lerp_00556791+ring_00555b28+geo_005558c6")
