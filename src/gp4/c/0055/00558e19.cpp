#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00558e19
// Appends the 0x60-byte record at esi to the table at 0x9559f8; the entry count is the dword at
// 0x9559f0 (capacity 0x200, an `int3` assertion fires when it would overflow). Returns the new
// entry's index in eax. esi, edi and ecx are saved and restored; the flags are those of the
// `add edi, offset` that forms the destination address.
void __cdecl FUN_00558e19(gp4::Regs* r) {
    const uint32_t* src = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t next = GP4_GLOBAL(uint32_t, 0x009559f0) + 1u;
    if (next >= 0x200u) {
        __debugbreak();
    }
    GP4_GLOBAL(uint32_t, 0x009559f0) = next;

    const uint32_t index = next - 1u;
    const uint32_t offset = index * 0x60u;
    uint32_t* dst = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(0x009559f8u + offset));
    for (int i = 0; i < 0x18; ++i) {
        dst[i] = src[i];
    }

    r->eax = index;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(0x009559f8u, offset));
}
GP4_IMPL(0x00558e19, FUN_00558e19, "regs(esi:ptr[0x60]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=layout=tab_00558e19")
