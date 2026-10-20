#include <gp4/gp4.h>

// One 0x20-byte entry of the table at 0x00627a2c: a cumulative weight and three (offset, scale)
// pairs of Q14 ranges.
struct Unknown_0043d008_Entry {
    int32_t Weight;
    int32_t Offset1;
    int32_t Scale1;
    int32_t Offset2;
    int32_t Scale2;
    int32_t Offset3;
    int32_t Scale3;
    int32_t Unknown1C;
};

struct Unknown_0043d008_Slot {
    uint8_t Unknown00[0x33];
    uint8_t Unknown33;
    uint8_t Unknown34;
    uint8_t Unknown35;
    uint8_t Unknown36;
};

// Low byte returned by the 0x00401462 generator.
static inline uint32_t nextRandomByte(const gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00401462, call);
    return call.eax & 0xffu;
}

// A random byte scaled by 64, multiplied by `scale` (Q14), offset, then run through 0x00410934 with the
// circuit number in edx. Returns the byte result; `flags` receives the callee's flags.
static inline uint8_t randomInRange(const gp4::Regs* r, int32_t scale, int32_t offset, uint32_t* flags) {
    const uint32_t draw = nextRandomByte(r) << 6;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(draw)) * static_cast<int64_t>(scale);
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + static_cast<uint32_t>(offset);
    call.edx = GP4_GLOBAL(uint32_t, 0x007ad894);
    gp4::call_regs(0x00410934, call);
    *flags = call.eflags & 0x8d5u;
    return static_cast<uint8_t>(call.eax);
}

// FUNCTION: GP4 0x0043d008
// ebx = slot (pushal / popal: every register is preserved). Fills bytes +0x33..+0x36 from the
// random generator and the table at 0x00627a2c, driven by the word DAT_007ad8a2: below 0x2d the
// bytes are the 0x6e defaults and +0x33 is cleared (0x0043d187 draws); between 0x2d and 0x36 an
// increasing chance (0x20 per step of 256) does the same through 0x0043d158; otherwise a table entry
// is picked by a weighted draw (at most 1, 2 or 3 as the word grows) and its three ranges give the
// bytes. The flags are those of the last callee.
void __cdecl FUN_0043d008(gp4::Regs* r) {
    Unknown_0043d008_Slot* slot = reinterpret_cast<Unknown_0043d008_Slot*>(static_cast<uintptr_t>(r->ebx));
    slot->Unknown34 = 0x6e;
    slot->Unknown35 = 0x6e;
    slot->Unknown36 = 0x6e;

    const int16_t level = GP4_GLOBAL(int16_t, 0x007ad8a2);
    uint32_t flags;
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    if (level < 0x2d) {
        gp4::call_regs(0x0043d187, call);
        slot->Unknown33 = 0;
        r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
        return;
    }

    const uint32_t draw = nextRandomByte(r);
    if (level < 0x37) {
        int32_t steps = static_cast<int32_t>(static_cast<int16_t>(static_cast<uint16_t>(level - 0x2d)));
        if (steps < 0) {
            steps = 0;
        }
        int32_t chance = steps << 5;
        if (chance > 0x100) {
            chance = 0x100;
        }
        chance = 0x100 - chance;
        if (static_cast<int32_t>(draw) < chance) {
            gp4::call_regs(0x0043d158, call);
            slot->Unknown33 = 0;
            r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
            return;
        }
    }

    uint32_t limit = 3;
    if (level < 0x5a) {
        limit = 2;
        if (level < 0x43) {
            limit = 1;
        }
    }

    const uint32_t pick = (nextRandomByte(r) * 100u) >> 8;
    const Unknown_0043d008_Entry* entry = reinterpret_cast<const Unknown_0043d008_Entry*>(0x00627a2cu);
    uint32_t cumulative = 0;
    uint32_t index = 1;
    for (;;) {
        cumulative += static_cast<uint32_t>(entry->Weight);
        if (static_cast<int32_t>(pick) <= static_cast<int32_t>(cumulative)) {
            break;
        }
        if (index == limit) {
            break;
        }
        ++index;
        ++entry;
        if (index == 4u) {
            --index;
            --entry;
            break;
        }
    }
    slot->Unknown33 = static_cast<uint8_t>(index);

    slot->Unknown34 = randomInRange(r, entry->Scale1, entry->Offset1, &flags);
    slot->Unknown35 = randomInRange(r, entry->Scale2, entry->Offset2, &flags);
    slot->Unknown36 = randomInRange(r, entry->Scale3, entry->Offset3, &flags);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043d008, FUN_0043d008, "regs(ebx:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0042595c+trk_007ad894+scale_0062eade")
