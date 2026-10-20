#include <gp4/gp4.h>
#include <gp4/flags.h>

// One (source pointer, byte count) pair of the copy table at 0x008306d0; a null source ends the table.
struct Unknown_0054045b_Entry {
    const uint8_t* Source00;
    uint32_t Count04;
};

// FUNCTION: GP4 0x0054045b
// Concatenates the blocks of the copy table at 0x008306d0 to the buffer in edi (`rep movsb` per entry,
// forward byte copy). Leaves ebx at the terminating entry, esi = 0, edi past the last byte written and
// ecx = 0 (unchanged when the table is empty); the flags are those of the final `or esi, esi`.
void __cdecl FUN_0054045b(gp4::Regs* r) {
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t count = r->ecx;
    const Unknown_0054045b_Entry* entry = GP4_ARRAY(const Unknown_0054045b_Entry, 0x008306d0);
    while (entry->Source00 != nullptr) {
        const uint8_t* src = entry->Source00;
        count = entry->Count04;
        while (count != 0u) {
            *dst = *src;
            ++src;
            ++dst;
            --count;
        }
        ++entry;
    }
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
    r->esi = 0u;
    r->ecx = count;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x0054045b, FUN_0054045b, "regs(ecx:u32, edi:ptr[0x1000]:bytes) -> (ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=blocks_0054045b")
