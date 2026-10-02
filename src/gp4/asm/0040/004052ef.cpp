#include <gp4/gp4.h>

// FUNCTION: GP4 0x004052ef
void __cdecl FUN_004052ef(gp4::Regs* r) {
    const int32_t input = static_cast<int32_t>(r->edx);
    const int32_t signed_scale = static_cast<int32_t>(r->ecx);
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    const int64_t square = static_cast<int64_t>(input) * input;
    const uint32_t squared_term = static_cast<uint32_t>(static_cast<uint64_t>(square) >> 15);
    const uint32_t magnitude = signed_scale < 0 ? 0u - r->ecx : r->ecx;
    const uint16_t factor = GP4_FIELD(const uint16_t, record, 0xb2);
    const uint32_t first = static_cast<uint32_t>(static_cast<int64_t>(static_cast<int32_t>(magnitude)) * factor);
    const int64_t scaled = static_cast<int64_t>(static_cast<int32_t>(first)) * GP4_GLOBAL(int32_t, 0x006f2bec);
    const uint32_t scaled_term = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 14);
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(scaled_term)) * static_cast<int32_t>(squared_term);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 12);
    const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 11);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    // Unicorn 2.1.4 gen_shiftd_rm_T1 / compute_all_sar retains the
    // concatenated product shifted by count-1 for OF. SHRD's AF and OF
    // are architecturally undefined at count 12; this is the pinned oracle
    // model, rather than a guarantee for every physical x86 processor.
    const uint32_t flags = ((low >> 11) & 1u) | ((~parity & 1u) << 2)
        | (result == 0 ? (1u << 6) : 0u) | (result & (1u << 31) ? (1u << 7) : 0u)
        | (((previous ^ result) >> 31) << 11);
    r->eax = result;
    r->ebp = squared_term;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004052ef, FUN_004052ef, "regs(ecx:i32, edx:i32, esi:ptr[0xbc]:bytes) -> (eax,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32")
