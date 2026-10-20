#include <gp4/gp4.h>
#include <gp4/flags.h>

// Moves the 16-byte record of index eax to index edx (pushal/popal wrapper): records are found as
// in 0x00536f64 (index below the signed base [0x00829710] -> static record 0x00829700, else
// ((index - base) & 0x1fff) * 16 + [0x00829278]). Dword +0 is a parent link, +4/+8 child links
// (-2 = none). The destination takes the source's three links, the parent's matching child link
// and both children's parent links are redirected to edx, then the source is cleared to -2.
// Registers are restored; the flags are those of the last flag writer (the final lookup).

static __declspec(noinline) uint32_t record_address(uint32_t index) {
    if ((int32_t)index < (int32_t)GP4_GLOBAL(uint32_t, 0x00829710)) {
        return (uint32_t)reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x00829700));
    }
    return (((index - GP4_GLOBAL(uint32_t, 0x00829710)) & 0x1fffu) << 4) + GP4_GLOBAL(uint32_t, 0x00829278);
}

// Flags left by the lookup sequence `sub r,[base]; jl s; and; shl 4; add r,[table]`.
static __declspec(noinline) uint32_t lookup_flags(uint32_t index) {
    uint32_t base = GP4_GLOBAL(uint32_t, 0x00829710);
    if ((int32_t)index < (int32_t)base) {
        return gp4::flags::sub32(index, base);
    }
    uint32_t scaled = ((index - base) & 0x1fffu) << 4;
    return gp4::flags::add32(scaled, GP4_GLOBAL(uint32_t, 0x00829278));
}

static inline uint32_t& field(uint32_t record, uint32_t off) {
    return *(uint32_t*)(uintptr_t)(record + off);
}

// FUNCTION: GP4 0x00536e63
void __cdecl FUN_00536e63(gp4::Regs* r) {
    const uint32_t from_index = r->eax;
    const uint32_t to_index = r->edx;
    const uint32_t from = record_address(from_index);
    const uint32_t to = record_address(to_index);

    uint32_t parent_index = field(from, 0);
    field(to, 0) = parent_index;
    const uint32_t parent = record_address(parent_index);
    const uint32_t first_child = field(parent, 4);
    if (first_child != 0xfffffffeu && record_address(first_child) == from) {
        field(parent, 4) = to_index;
    } else {
        field(parent, 8) = to_index;
    }

    const uint32_t left = field(from, 4);
    field(to, 4) = left;
    if (left != 0xfffffffeu) {
        field(record_address(left), 0) = to_index;
    }

    const uint32_t right = field(from, 8);
    field(to, 8) = right;
    uint32_t arith;
    if (right != 0xfffffffeu) {
        field(record_address(right), 0) = to_index;
        arith = lookup_flags(right);
    } else {
        arith = gp4::flags::sub32(right, 0xfffffffeu);
    }

    field(from, 0) = 0xfffffffeu;
    field(from, 4) = 0xfffffffeu;
    field(from, 8) = 0xfffffffeu;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00536e63, FUN_00536e63, "regs(eax:u32, edx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00829278+sent_00536e63")
