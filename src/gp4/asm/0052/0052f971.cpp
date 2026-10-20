#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t spanBytes(uint32_t end, uint32_t begin) {
    return end - begin;
}

// Called by 0x0052f7ad (edi = 0x007ab028, esi = the loaded block) when the block's leading word is
// 0x3a, instead of the plain copy of 0x007c2e70 - 0x007ab028 bytes: skips the leading word of both,
// clears the next 0xc4 destination bytes, copies the remaining 0x17e48 - 0xc6 bytes after them
// (`rep stosb` / `rep movsb`, forward), then moves sixteen dwords of the copied area
// (0x007ab0f4..0x007ab130) into the cleared area (0x007ab02c..0x007ab0a8) and sets the word at
// 0x007ad890 to 0x53. Leaves ecx = 0, esi/edi past the copy, eax = [0x007ab130]; the flags are
// those of `sub ecx, 0xc6`.
// FUNCTION: GP4 0x0052f971
void __cdecl FUN_0052f971(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi)) + 2;
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi)) + 2;

    for (uint32_t i = 0; i != 0xc4u; ++i) {
        *dst++ = 0;
    }
    const uint32_t total = spanBytes(0x007c2e70u, 0x007ab028u);
    const uint32_t count = total - 0xc6u;
    for (uint32_t i = 0; i != count; ++i) {
        *dst++ = *src++;
    }

    GP4_GLOBAL(uint32_t, 0x007ab02c) = GP4_GLOBAL(uint32_t, 0x007ab0f4);
    GP4_GLOBAL(uint32_t, 0x007ab03c) = GP4_GLOBAL(uint32_t, 0x007ab0f8);
    GP4_GLOBAL(uint32_t, 0x007ab034) = GP4_GLOBAL(uint32_t, 0x007ab0fc);
    GP4_GLOBAL(uint32_t, 0x007ab030) = GP4_GLOBAL(uint32_t, 0x007ab100);
    GP4_GLOBAL(uint32_t, 0x007ab038) = GP4_GLOBAL(uint32_t, 0x007ab104);
    GP4_GLOBAL(uint32_t, 0x007ab044) = GP4_GLOBAL(uint32_t, 0x007ab108);
    GP4_GLOBAL(uint32_t, 0x007ab040) = GP4_GLOBAL(uint32_t, 0x007ab10c);
    GP4_GLOBAL(uint32_t, 0x007ab048) = GP4_GLOBAL(uint32_t, 0x007ab110);
    GP4_GLOBAL(uint32_t, 0x007ab08c) = GP4_GLOBAL(uint32_t, 0x007ab114);
    GP4_GLOBAL(uint32_t, 0x007ab090) = GP4_GLOBAL(uint32_t, 0x007ab120);
    GP4_GLOBAL(uint32_t, 0x007ab094) = GP4_GLOBAL(uint32_t, 0x007ab11c);
    GP4_GLOBAL(uint32_t, 0x007ab098) = GP4_GLOBAL(uint32_t, 0x007ab124);
    GP4_GLOBAL(uint32_t, 0x007ab09c) = GP4_GLOBAL(uint32_t, 0x007ab118);
    GP4_GLOBAL(uint32_t, 0x007ab0a0) = GP4_GLOBAL(uint32_t, 0x007ab12c);
    GP4_GLOBAL(uint32_t, 0x007ab0a4) = GP4_GLOBAL(uint32_t, 0x007ab128);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x007ab130);
    GP4_GLOBAL(uint32_t, 0x007ab0a8) = last;
    GP4_GLOBAL(uint16_t, 0x007ad890) = 0x53;

    r->eax = last;
    r->ecx = 0u;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(total, 0xc6u));
}
GP4_IMPL(0x0052f971, FUN_0052f971, "regs(esi:ptr[0x17d88]:bytes, edi:u32{=0x7ab028}) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); wall=1800")
