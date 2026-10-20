#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags of a logic result (`or`, `test`): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// One of the 16 entries (stride 0x2c) at 0x0063decc.
struct Unknown_0063decc {
    uint32_t Unknown00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    uint32_t Unknown0c;
    uint32_t Unknown10;
    uint32_t Unknown14;
    uint32_t Unknown18;
    uint32_t Unknown1c;
    uint32_t Unknown20;
    uint32_t Unknown24;
    uint32_t Unknown28;
};

// FUNCTION: GP4 0x00423160
// Registers a new entry in the 16-entry table at 0x0063decc for the pair (dword esi+0x48, byte
// [0x00609744]+0x6d), unless an active entry (dword +0 or +4 nonzero) already holds the same pair
// in its +0x18/+0x1c fields (in either order). A free entry (0x0041891a) receives
// min((sqrt(eax) * [0x00626944]) >> 14, 0x4000) in +0x28, the pair in +0x18/+0x1c, dword
// [0x00609744]+0x4c in +0x24, the dwords 0, 1, 2 and 0xc of the block esi+0x60 points to in
// +0xc/+0x10/+0x14/+0x20, and bit 0 of dword +0. pushal/popal keep every register; the flags are
// the last flag writer's: the matching compare (ZF|PF), 0x0041891a's when the table is full, or
// the final "or".
void __cdecl FUN_00423160(gp4::Regs* r) {
    const uint32_t weight = r->eax;
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00609744)));
    const uint32_t key = GP4_FIELD(uint32_t, source, 0x48);
    const uint32_t other = GP4_FIELD(uint8_t, record, 0x6d);
    Unknown_0063decc* entries = GP4_ARRAY(Unknown_0063decc, 0x0063decc);
    uint32_t flags;

    for (uint32_t index = 0; index != 0x10u; ++index) {
        const Unknown_0063decc& entry = entries[index];
        if (entry.Unknown04 == 0u && entry.Unknown00 == 0u) {
            continue;
        }
        if (key == entry.Unknown18) {
            if (other == entry.Unknown1c) {
                r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
                return;
            }
        } else if (key == entry.Unknown1c) {
            if (other == entry.Unknown18) {
                r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
                return;
            }
        }
    }

    gp4::Regs find = *r;
    gp4::call_regs(0x0041891a, find);
    if ((find.eflags & 1u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | (find.eflags & 0x8d5u);
        return;
    }
    Unknown_0063decc* slot = reinterpret_cast<Unknown_0063decc*>(static_cast<uintptr_t>(find.eax));

    gp4::Regs root = *r;
    root.eax = weight;
    gp4::call_regs(0x004010f3, root);
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(root.eax))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00626944));
    int32_t scaled = static_cast<int32_t>(static_cast<uint64_t>(product) >> 14);
    if (scaled > 0x4000) {
        scaled = 0x4000;
    }
    slot->Unknown28 = static_cast<uint32_t>(scaled);
    slot->Unknown18 = GP4_FIELD(uint32_t, source, 0x48);
    slot->Unknown1c = GP4_FIELD(uint8_t, record, 0x6d);
    slot->Unknown24 = GP4_FIELD(uint32_t, record, 0x4c);
    const uint32_t* block = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, source, 0x60)));
    slot->Unknown0c = block[0];
    slot->Unknown10 = block[1];
    slot->Unknown14 = block[2];
    slot->Unknown20 = block[0xc];
    slot->Unknown00 |= 1u;
    flags = logicFlags32(slot->Unknown00);

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00423160, FUN_00423160, "regs(eax:u32, esi:ptr[0x68]:layout=key_00423160) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_0041891a+match_00423160")
