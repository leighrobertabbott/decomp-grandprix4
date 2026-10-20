#include <gp4/gp4.h>

// Record initialised by 0x005317a5 (bytes not listed are left untouched).
struct Unknown_005317a5 {
    uint32_t Unknown00[6];   // 0x00..0x14: all 0xffffffff
    uint8_t Unknown18[4];    // untouched
    uint8_t Unknown1C;       // 3
    uint8_t Unknown1D;       // 0xff
    uint8_t Unknown1E;       // 0
    uint8_t Unknown1F;       // 0xff
    uint8_t Unknown20[4];    // untouched
    uint8_t Unknown24;       // 0xff
    uint8_t Unknown25;       // untouched
    uint8_t Unknown26;       // 0
    uint8_t Unknown27;       // 0
    uint8_t Unknown28;       // 0
    uint8_t Unknown29;       // 0xff
    uint8_t Unknown2A;       // 0
    uint8_t Unknown2B;       // 0
};

// Sets the record at ebx to its "unset" state: six 0xffffffff dwords, then the byte fields.
// FUNCTION: GP4 0x005317a5
void __cdecl FUN_005317a5(gp4::Regs* r) {
    Unknown_005317a5* rec = reinterpret_cast<Unknown_005317a5*>(r->ebx);
    for (int i = 0; i < 6; ++i) {
        rec->Unknown00[i] = 0xffffffffu;
    }
    rec->Unknown1C = 3u;
    rec->Unknown1D = 0xffu;
    rec->Unknown1E = 0u;
    rec->Unknown1F = 0xffu;
    rec->Unknown24 = 0xffu;
    rec->Unknown26 = 0u;
    rec->Unknown27 = 0u;
    rec->Unknown28 = 0u;
    rec->Unknown29 = 0xffu;
    rec->Unknown2A = 0u;
    rec->Unknown2B = 0u;
}
GP4_IMPL(0x005317a5, FUN_005317a5, "regs(ebx:ptr[0x40]:bytes) -> ()")
