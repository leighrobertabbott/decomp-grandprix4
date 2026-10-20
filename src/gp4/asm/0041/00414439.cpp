#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit logic result (or / test): CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

struct Unknown_00414439_Position {
    int32_t UnknownX;
    int32_t UnknownZ;
};

struct Unknown_00414439_Record {
    int32_t UnknownX;
    int32_t UnknownZ;
    uint8_t Unknown08[0x120];
    Unknown_00414439_Position* Target128;
};

// Bearing (eax, through the arctangent routine 0x00401315) and distance (edx, integer square root
// of dx*dx + dz*dz through 0x0040113f) from the position at the start of the record in esi to the
// position its pointer at +0x128 refers to. CF = 1 (eax and edx untouched) when there is no target;
// otherwise CF = 0 and the other flags are those left by the square root. ebp, ebx and ecx are saved.
// FUNCTION: GP4 0x00414439
void __cdecl FUN_00414439(gp4::Regs* r) {
    const Unknown_00414439_Record* self = reinterpret_cast<const Unknown_00414439_Record*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00414439_Position* target = self->Target128;
    if (target == nullptr) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags(0u) | 1u;
        return;
    }

    const uint32_t dx = static_cast<uint32_t>(target->UnknownX) - static_cast<uint32_t>(self->UnknownX);
    const uint32_t dz = static_cast<uint32_t>(target->UnknownZ) - static_cast<uint32_t>(self->UnknownZ);

    gp4::Regs bearing = *r;
    bearing.eax = dx;
    bearing.edx = dz;
    gp4::call_regs(0x00401315, bearing);

    const uint64_t squareX = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(dx)) * static_cast<int64_t>(static_cast<int32_t>(dx)));
    const uint64_t squareZ = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(dz)) * static_cast<int64_t>(static_cast<int32_t>(dz)));
    const uint64_t sum = squareX + squareZ;

    gp4::Regs root = *r;
    root.eax = static_cast<uint32_t>(sum);
    root.edx = static_cast<uint32_t>(sum >> 32);
    gp4::call_regs(0x0040113f, root);

    r->eax = bearing.eax;
    r->edx = root.eax;
    r->eflags = (r->eflags & ~0x8d5u) | (root.eflags & 0x8d4u);
}
GP4_IMPL(0x00414439, FUN_00414439, "regs(eax:u32, edx:u32, esi:ptr[0x130]:layout=ptr_00414439) -> (eax, edx, cf, pf, af, zf, sf, of)")
