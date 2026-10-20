#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies bytes +1..+4 of the 6-byte record eax of the table at 0x891570 to the four bytes at 0x88fd94.
// eax ends as the record address with its low byte replaced by byte +4; edx is saved and restored. The flags
// are those of the address computation `add eax, 0x891570` (the imul's are overwritten).
// FUNCTION: GP4 0x00514def
void __cdecl FUN_00514def(gp4::Regs* r) {
    const uint32_t offset = r->eax * 6u;
    const uint32_t address_flags = gp4::flags::add32(offset, 0x00891570u);
    const uint8_t* record = GP4_ARRAY(const uint8_t, 0x00891570) + offset;

    GP4_GLOBAL(uint8_t, 0x0088fd94) = record[1];
    GP4_GLOBAL(uint8_t, 0x0088fd95) = record[2];
    GP4_GLOBAL(uint8_t, 0x0088fd96) = record[3];
    GP4_GLOBAL(uint8_t, 0x0088fd97) = record[4];

    r->eax = ((offset + 0x00891570u) & 0xffffff00u) | record[4];
    r->eflags = gp4::flags::merge(r->eflags, address_flags);
}
GP4_IMPL(0x00514def, FUN_00514def, "regs(eax:u32[0..40]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
