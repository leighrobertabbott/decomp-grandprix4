#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_4d7e50 {
    uint8_t Unknown00[0x1c];
    float Points1c[40][3];
    uint16_t PointCount1fc;
};
#pragma pack(pop)

// Parses the crowd data text (loaded by 0x4d8020 from "samples\crowddata\%s.crd") into at most 40
// points at +0x1c, the count in the word at +0x1fc. Each line (ended by the string 0x63e31c) holds
// six fields split by the string 0x660730: field 1 goes through strtol (base 10, result unused), field
// 2 is skipped, fields 3..5 are atof'd into x, y, z and field 6 (up to the character before the line
// end, or the end of the text) is atof'd and dropped. Stops at the end of the text, a missing
// separator, a 41st point or the last line.
// FUNCTION: GP4 0x004d7e50
void __fastcall FUN_004d7e50(Unknown_4d7e50* self, void* /*edx*/, char* text, int length) {
    gp4::cdecl_<0x005ae990, void>();
    char* const end = text + length;
    self->PointCount1fc = 0;
    char* p = text;
    while (p < end) {
        char* const lineEnd = gp4::cdecl_<0x005db400, char*>(p, GP4_ARRAY(const char, 0x0063e31c));
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        for (int field = 1; field < 7; field++) {
            char* const fieldStart = p;
            if (field == 6) {
                if (lineEnd != nullptr) {
                    p = lineEnd - 1;
                } else {
                    p = end;
                }
            } else {
                p = gp4::cdecl_<0x005db400, char*>(p, GP4_ARRAY(const char, 0x00660730));
            }
            if (p == nullptr) {
                return;
            }
            const uint32_t count = static_cast<uint32_t>(p - fieldStart);
            char token[200];
            for (uint32_t i = 0; i < count; i++) {
                token[i] = fieldStart[i];
            }
            token[count] = 0;
            switch (field) {
            case 1:
                gp4::cdecl_<0x005ddbb4, long>(token, static_cast<char**>(nullptr), 10);
                break;
            case 3:
                x = static_cast<float>(gp4::cdecl_<0x005a7aa0, double>(token));
                break;
            case 4:
                y = static_cast<float>(gp4::cdecl_<0x005a7aa0, double>(token));
                break;
            case 5:
                z = static_cast<float>(gp4::cdecl_<0x005a7aa0, double>(token));
                break;
            case 6:
                gp4::cdecl_<0x005a7aa0, double>(token);
                break;
            }
            p++;
        }
        if (self->PointCount1fc >= 40) {
            return;
        }
        self->Points1c[self->PointCount1fc][0] = x;
        self->Points1c[self->PointCount1fc][1] = y;
        self->Points1c[self->PointCount1fc][2] = z;
        self->PointCount1fc++;
        if (lineEnd == nullptr) {
            return;
        }
        p = lineEnd + 1;
    }
}
GP4_IMPL(0x004d7e50, FUN_004d7e50, "thiscall(ptr[0x208]:bytes, ptr[0x360]:{0*40/16:u32{=0x2c322c31,0x2c302c37,0x2c392c2d,0x2c782c30}, 0x280:u32{=0x2c322c31,0x00000031}, 4*41/16:u32{=0x2c352e33,0x2c2e312d,0x2c336531,0x2c303532,0x2c35322e}, 8*41/16:u32{=0x352c342d,0x322c3031,0x392c352e,0x2d2c6538}, 12*39/16:u32{=0x0a0d362c,0x0a0d392c}, 0x27c:u32{=0x0a0d362c,0x0000362c}, 0x28c:u32{=0x0a0d392c}}, i32{=0,1,16,33,100,624,637,641,672,700,836}) -> void")
