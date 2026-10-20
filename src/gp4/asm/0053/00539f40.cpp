#include <gp4/gp4.h>
#include <gp4/flags.h>

// Loads a 4-word record at esi sign-extended: ebx = word 0, eax = word 1, ebp = word 2, ecx = word 3,
// then esi += 8. The flags are those of `add esi, 8`.
// FUNCTION: GP4 0x00539f40
void __cdecl FUN_00539f40(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    const int16_t* words = reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(esi));
    r->ebx = static_cast<uint32_t>(static_cast<int32_t>(words[0]));
    r->eax = static_cast<uint32_t>(static_cast<int32_t>(words[1]));
    r->ebp = static_cast<uint32_t>(static_cast<int32_t>(words[2]));
    r->ecx = static_cast<uint32_t>(static_cast<int32_t>(words[3]));
    r->esi = esi + 8u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 8u));
}
GP4_IMPL(0x00539f40, FUN_00539f40, "regs(esi:ptr[0x40]:bytes) -> (eax, ebx, ecx, ebp, esi, cf, pf, af, zf, sf, of)")
