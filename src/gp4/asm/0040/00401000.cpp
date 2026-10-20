#include <gp4/gp4.h>

// FUNCTION: GP4 0x00401000
// Signed 32 x 32 -> 64-bit multiply built from 16-bit pieces: a = dx:ax, b = cx:bx (high words in dx and cx).
// The magnitudes are multiplied (four 16-bit "mul"s with carry fix-ups) and the 64-bit product is negated when
// the high words differ in sign. The product comes back as ax = bits 0..15, dx = bits 16..31, bx = bits 32..47,
// cx = bits 48..63 (upper halves of the registers keep their value). esi keeps its upper half and gets
// low word dx_in ^ cx_in, edi receives the incoming ecx (pop esi / pop edi of the pushed values); ebp and esp
// are restored. The closing "add esp, 0x20" makes the flags a function of the stack pointer: in the oracle
// they are PF only (pinned oracle model, as 0x00409c44).
void __cdecl FUN_00401000(gp4::Regs* r) {
    const uint32_t a = (r->edx << 16) | (r->eax & 0xffffu);
    const uint32_t b = (r->ecx << 16) | (r->ebx & 0xffffu);

    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(a))
        * static_cast<int64_t>(static_cast<int32_t>(b)));

    const uint32_t incomingEcx = r->ecx;
    const uint32_t signWord = (r->edx ^ r->ecx) & 0xffffu;

    r->eax = (r->eax & 0xffff0000u) | static_cast<uint32_t>(product & 0xffffu);
    r->edx = (r->edx & 0xffff0000u) | static_cast<uint32_t>((product >> 16) & 0xffffu);
    r->ebx = (r->ebx & 0xffff0000u) | static_cast<uint32_t>((product >> 32) & 0xffffu);
    r->ecx = (r->ecx & 0xffff0000u) | static_cast<uint32_t>((product >> 48) & 0xffffu);
    r->esi = (r->esi & 0xffff0000u) | signWord;
    r->edi = incomingEcx;
    r->eflags = r->eflags & ~0x8d5u;
}
GP4_IMPL(0x00401000, FUN_00401000, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of)")
