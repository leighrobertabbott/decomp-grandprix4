#include <gp4/gp4.h>

// Looks for the CD-ROM drive: the 26 drive-type bytes at 0x0081ea08 (drive letters 'A'..'Z', a
// value of 5 = DRIVE_CDROM) are scanned in order. On the first CD-ROM the track file
// "X:\sound\pc\tk3.st1" is opened with CreateFileA (GENERIC_READ, no sharing, OPEN_EXISTING,
// FILE_ATTRIBUTE_NORMAL) through the import thunk 0x005a48b8 and the handle is stored at
// [0x0081ea28], [0x0081ea2c] is set to 0xff, the size routine 0x0053644d and the close routine
// 0x0053646a run, the drive letter replaces the first character of "D:\grand prix 3" (0x0081e5d9)
// and 1 is returned; 0 when no drive is a CD-ROM. ecx and edi are saved and restored, and every
// caller only looks at eax.
// FUNCTION: GP4 0x005370bd
uint32_t __cdecl FUN_005370bd() {
    const uint8_t* driveType = GP4_ARRAY(uint8_t, 0x0081ea08);
    for (uint32_t letter = 0x41u; letter <= 0x5au; ++letter) {
        if (*driveType == 5u) {
            GP4_GLOBAL(uint32_t, 0x0081ea28) = gp4::stdcall_<0x005a48b8, uint32_t>(
                static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x0082972a))),
                0x80000000u, 0u, 0u, 3u, 0x80u, 0u);
            GP4_GLOBAL(uint8_t, 0x0081ea2c) = 0xffu;
            gp4::Regs size = {};
            gp4::call_regs(0x0053644d, size);
            gp4::Regs close = {};
            gp4::call_regs(0x0053646a, close);
            GP4_GLOBAL(uint8_t, 0x0081e5d9) = static_cast<uint8_t>(letter);
            return 1u;
        }
        ++driveType;
    }
    return 0u;
}
GP4_IMPL(0x005370bd, FUN_005370bd, "cdecl() -> u32; globals=fuzz:i32")
