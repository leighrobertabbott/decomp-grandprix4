#include <gp4/gp4.h>
#include <gp4/flags.h>

// Replaces the link to node eax in its parent by edx (pushal/popal wrapper, sibling of 0x00536e63). Records
// are found as in 0x00536f64 (index below the signed base [0x00829710] -> static record 0x00829700, else
// ((index - base) & 0x1fff) * 16 + [0x00829278]); dword +0 is the parent link, +4 / +8 the child links
// (-2 = none). The parent's +8 link becomes edx when it points at the node, otherwise its +4 link does;
// a replacement other than -2 takes the node's parent link; the node's three links are then set to -2.
// Registers are restored; the flags are those of the last flag writer: `cmp edx, -2` (edx == -2) or the
// lookup of edx.

static __declspec(noinline) uint32_t record_address(uint32_t index) {
    if (static_cast<int32_t>(index) < static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00829710))) {
        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x00829700)));
    }
    return (((index - GP4_GLOBAL(uint32_t, 0x00829710)) & 0x1fffu) << 4) + GP4_GLOBAL(uint32_t, 0x00829278);
}

// Flags left by the lookup sequence `sub r,[base]; jl s; and; shl 4; add r,[table]`.
static __declspec(noinline) uint32_t lookup_flags(uint32_t index) {
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x00829710);
    if (static_cast<int32_t>(index) < static_cast<int32_t>(base)) {
        return gp4::flags::sub32(index, base);
    }
    const uint32_t scaled = ((index - base) & 0x1fffu) << 4;
    return gp4::flags::add32(scaled, GP4_GLOBAL(uint32_t, 0x00829278));
}

static inline uint32_t& field(uint32_t record, uint32_t off) {
    return *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(record + off));
}

// FUNCTION: GP4 0x00536db1
void __cdecl FUN_00536db1(gp4::Regs* r) {
    const uint32_t node_index = r->eax;
    const uint32_t replacement = r->edx;
    const uint32_t node = record_address(node_index);
    const uint32_t parent = record_address(field(node, 0));

    const uint32_t second = field(parent, 8);
    if (second != 0xfffffffeu && record_address(second) == node) {
        field(parent, 8) = replacement;
    } else {
        field(parent, 4) = replacement;
    }

    uint32_t arith;
    if (replacement != 0xfffffffeu) {
        const uint32_t target = record_address(replacement);
        field(target, 0) = field(node, 0);
        arith = lookup_flags(replacement);
    } else {
        arith = gp4::flags::sub32(replacement, 0xfffffffeu);
    }

    field(node, 0) = 0xfffffffeu;
    field(node, 4) = 0xfffffffeu;
    field(node, 8) = 0xfffffffeu;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00536db1, FUN_00536db1, "regs(eax:u32{0x1000,0x1001,0x1005,0x2fff,0x3000,0,0xffffffff}, ebx:u32, ecx:u32, edx:u32{0xfffffffe,0xfffffffe,0xfffffffe,0x1000,0x1005}, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=kids_00536db1+sent_00536e63")
