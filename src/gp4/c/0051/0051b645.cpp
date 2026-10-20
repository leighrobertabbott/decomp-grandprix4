#include <gp4/gp4.h>
#include <gp4/flags.h>

// Nearest-point search: scans the 6-byte {x, y, z} int16 entries of the table at [0x96a3d4] for indices 254..1 and
// leaves the index of the entry closest (squared distance, unsigned 32-bit) to the point (0x933a14, 0x933a16,
// 0x933a18) in the byte 0x933a20 (0 when none) with the best distance in 0x933a74. When the byte 0x95405e is not
// 0xff every index is measured; when it is 0xff only the indices whose flag byte at 0x954060 + index is 0xff are
// measured (the others are skipped). The routine returns the index in al; the upper bits of eax keep the last
// squared z difference computed (else the incoming eax). The flags are those of the final `dec ecx` (1 -> 0) with
// the CF left by the last iteration: the unsigned compare of the distance against the best so far, or 1 when the
// index was skipped by its flag byte (cmp byte, 0xff with a byte below 0xff).
// FUNCTION: GP4 0x0051b645
void __cdecl FUN_0051b645(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t carry = 0u;

    GP4_GLOBAL(uint32_t, 0x00933a74) = 0x7fffffffu;
    GP4_GLOBAL(uint8_t, 0x00933a20) = 0u;
    for (uint32_t index = 0xfeu; index != 0u; --index) {
        if (GP4_GLOBAL(uint8_t, 0x0095405e) == 0xffu && GP4_ARRAY(uint8_t, 0x00954060)[index] != 0xffu) {
            carry = 1u;
            continue;
        }
        const int16_t* entry = reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(index * 6u + GP4_GLOBAL(uint32_t, 0x0096a3d4)));
        const int32_t dx = static_cast<int16_t>(static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00933a14) - static_cast<uint16_t>(entry[0])));
        const uint32_t sx = static_cast<uint32_t>(dx * dx);
        const int32_t dy = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(entry[1]) - GP4_GLOBAL(uint16_t, 0x00933a16)));
        const uint32_t sy = static_cast<uint32_t>(dy * dy);
        const int32_t dz = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(entry[2]) - GP4_GLOBAL(uint16_t, 0x00933a18)));
        const uint32_t sz = static_cast<uint32_t>(dz * dz);
        eax = sz;
        const uint32_t distance = sx + sy + sz;
        const uint32_t best = GP4_GLOBAL(uint32_t, 0x00933a74);
        carry = (distance < best) ? 1u : 0u;
        if (distance < best) {
            GP4_GLOBAL(uint32_t, 0x00933a74) = distance;
            GP4_GLOBAL(uint8_t, 0x00933a20) = static_cast<uint8_t>(index);
        }
    }

    r->eax = (eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00933a20);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, carry));
}
GP4_IMPL(0x0051b645, FUN_0051b645, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rgb_0051b1f9+pts_0051b645")
