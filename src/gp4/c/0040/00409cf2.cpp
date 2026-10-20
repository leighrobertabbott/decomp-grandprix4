#include <gp4/gp4.h>

// One Newton step of the root: the 64-bit radicand (high dword, low dword) divided by the estimate
// with the 16-bit-register divider 0x00409e1a (dividend cx:bx:dx:ax, divisor si:di, quotient dx:ax).
static __declspec(noinline) uint32_t divideRadicand(const gp4::Regs* entry, uint32_t high, uint32_t low,
                                                    uint32_t estimate) {
    gp4::Regs call = *entry;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = (entry->eax & 0xffff0000u) | (low & 0xffffu);
    call.edx = (entry->edx & 0xffff0000u) | (low >> 16);
    call.ebx = (entry->ebx & 0xffff0000u) | (high & 0xffffu);
    call.ecx = (entry->ecx & 0xffff0000u) | (high >> 16);
    call.edi = (entry->edi & 0xffff0000u) | (estimate & 0xffffu);
    call.esi = (entry->esi & 0xffff0000u) | (estimate >> 16);
    gp4::call_regs(0x00409e1a, call);
    return ((call.edx & 0xffffu) << 16) | (call.eax & 0xffffu);
}

static __declspec(noinline) void smallRoot(gp4::Regs& call) { gp4::call_regs(0x00409c44, call); }

// FUNCTION: GP4 0x00409cf2
// Integer square root of the 64-bit unsigned value cx:bx:dx:ax (ax lowest), returned in eax (edx = the
// high word of eax). A value below 2^32 (cx = bx = 0) goes to the 32-bit root 0x00409c44. Otherwise
// the value is shifted left two bits at a time until cx >= 0x4000 (count pairs), halved, and five
// Newton steps estimate = estimate / 2 + value / estimate start from the high dword plus 0x80000000;
// the estimate is shifted right by count. bx and cx (low words) leave holding the halved value's
// high words. The closing `add esp, 0x20` makes the arithmetic flags a function of the stack pointer:
// all clear in the oracle (the same prologue as 0x00409e29).
void __cdecl FUN_00409cf2(gp4::Regs* r) {
    uint16_t cx = static_cast<uint16_t>(r->ecx);
    uint16_t bx = static_cast<uint16_t>(r->ebx);

    if (cx == 0u && bx == 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        smallRoot(call);
        r->eax = call.eax;
        r->edx = call.edx;
        r->eflags = r->eflags & ~0x8d5u;
        return;
    }

    uint64_t value = (static_cast<uint64_t>(cx) << 48) | (static_cast<uint64_t>(bx) << 32)
        | (static_cast<uint64_t>(static_cast<uint16_t>(r->edx)) << 16) | static_cast<uint16_t>(r->eax);
    uint32_t count = 0;
    while (static_cast<uint16_t>(value >> 48) < 0x4000u) {
        count++;
        value <<= 2;
    }
    value >>= 1;

    const uint32_t high = static_cast<uint32_t>(value >> 32);
    const uint32_t low = static_cast<uint32_t>(value);
    uint32_t estimate = high + 0x80000000u;
    for (int step = 4; step >= 0; --step) {
        estimate = (estimate >> 1) + divideRadicand(r, high, low, estimate);
    }

    const uint32_t root = estimate >> (count & 31u);
    r->eax = root;
    r->edx = root >> 16;
    r->ebx = (r->ebx & 0xffff0000u) | (high & 0xffffu);
    r->ecx = (r->ecx & 0xffff0000u) | (high >> 16);
    r->eflags = r->eflags & ~0x8d5u;
}
GP4_IMPL(0x00409cf2, FUN_00409cf2, "regs(eax:u32, ebx:u32[0..3], ecx:u32[0..3], edx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of)")
