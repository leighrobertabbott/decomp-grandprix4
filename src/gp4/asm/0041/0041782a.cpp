#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0041782a
// Copies per-car values from the four records of 0x006380d8 (ebx, set by 0x0041774e) into the
// record at esi and the per-slot arrays at 0x0066fb80/0x0066fbd8/0x0066fc30/0x0066fc88, and
// rounds two entries of the 0x60-byte-stride double table at 0x006020d8/0x00602108 into the
// record (+0x228/+0x22c). eax ends as (byte [esi+0x202]) * 0x60, edx as the slot index
// ((byte [esi+0x7c] & 0x3f) - 1, at most 0x15); the flags are those of that imul.
void __cdecl FUN_0041782a(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_FIELD(int16_t, rec, 0x62) = static_cast<int16_t>(GP4_GLOBAL(int32_t, 0x00623c34) >> 8);
    GP4_FIELD(int16_t, rec, 0x20a) = static_cast<int16_t>(GP4_GLOBAL(int32_t, 0x006258c8) >> 8);

    // sets ebx = 0x006380d8 and stores the two words at +0x27e / +0x278
    gp4::Regs call = *r;
    gp4::call_regs(0x0041774e, call);
    uint8_t* const cars = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(call.ebx));

    GP4_FIELD(uint32_t, rec, 0x36c) = GP4_FIELD(uint32_t, cars, 0x40);
    GP4_FIELD(uint32_t, rec, 0x370) = GP4_FIELD(uint32_t, cars, 0x340);
    GP4_FIELD(uint32_t, rec, 0x374) = GP4_FIELD(uint32_t, cars, 0x640);
    GP4_FIELD(uint32_t, rec, 0x378) = GP4_FIELD(uint32_t, cars, 0x940);

    int32_t slot = static_cast<int32_t>(GP4_FIELD(uint8_t, rec, 0x7c) & 0x3fu) - 1;
    if (slot > 0x15) slot = 0x15;

    GP4_ARRAY(uint32_t, 0x0066fb80)[slot] = GP4_FIELD(uint32_t, cars, 0x178);
    GP4_ARRAY(uint32_t, 0x0066fbd8)[slot] = GP4_FIELD(uint32_t, cars, 0x478);
    GP4_ARRAY(uint32_t, 0x0066fc30)[slot] = GP4_FIELD(uint32_t, cars, 0x778);
    GP4_ARRAY(uint32_t, 0x0066fc88)[slot] = GP4_FIELD(uint32_t, cars, 0xa78);

    const uint32_t offset = static_cast<uint32_t>(GP4_FIELD(uint8_t, rec, 0x202)) * 0x60u;
    const uint8_t* const table = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(offset));
    GP4_FIELD(int32_t, rec, 0x228) = gp4::x87::fistp32(*reinterpret_cast<const double*>(table + 0x006020d8));
    GP4_FIELD(int32_t, rec, 0x22c) = gp4::x87::fistp32(*reinterpret_cast<const double*>(table + 0x00602108));

    // imul eax, eax, 0x60 of a byte: no overflow (CF = OF = 0), AF = 0
    const uint32_t flags = parityFlag(offset) | (offset == 0u ? 0x40u : 0u);
    call.eax = offset;
    call.edx = static_cast<uint32_t>(slot);
    call.eflags = (r->eflags & ~0x8d5u) | flags;
    *r = call;
}
GP4_IMPL(0x0041782a, FUN_0041782a, "regs(esi:ptr[0x380]:bytes) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
