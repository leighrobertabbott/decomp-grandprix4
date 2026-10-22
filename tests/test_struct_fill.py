"""Inline buffer fills in specs: ptr[N]:{off:type, ...} with nested pointers (gp4re/spec.py struct_fill)."""
from __future__ import annotations

import struct
import unittest
from unittest.mock import patch

from pathlib import Path

from gp4re.binary import Binary, Section
from gp4re.emu import CaseGen, Impl, differential
from gp4re.spec import SpecError, parse

from test_emu_soundness import INIT, TARGET, images   # tests/ is on sys.path

CFG = {"init_calls": [INIT], "max_cases": 8, "time_budget_s": 2,
       "min_block_coverage": 0.90, "min_branch_coverage": 0.70, "ni_cases": 1}


class StructFillTests(unittest.TestCase):
    def test_parse_and_bounds(self):
        parse("thiscall(ptr[0x20]:{8:ptr[0x40]:bytes, 0xc:u32[0..15]}) -> void")
        parse("cdecl(ptr[0x10]:{0:nptr[16]:com=IDirectInputDevice8A}) -> void")
        parse("cdecl(ptr[0x60]:{*bytes, 0xc*20/4:ptr[8]:{0:u32{=0x5f0758}}}) -> void")
        with self.assertRaises(SpecError):
            parse("cdecl(ptr[0x10]:{0xe:u32}) -> void")          # runs past the buffer
        with self.assertRaises(SpecError):
            parse("cdecl(ptr[0x10]:{4:rel[eax,4,0..1]}) -> void")

    def test_generated_buffer(self):
        spec = parse("cdecl(ptr[0x20]:{8:ptr[0x40]:{0:u32{=7}}, 0xc:u32[0..15], 0x10*2/8:u8{=0xab}}) -> void")
        for seed in range(20):
            g = CaseGen(spec, seed)
            kind, p = g.value(spec.args[0])
            outer = dict(g.buffers)[p]
            inner_p, idx = struct.unpack_from("<II", outer, 8)
            self.assertLessEqual(idx, 15)
            self.assertEqual(outer[0x10], 0xAB)
            self.assertEqual(outer[0x18], 0xAB)
            self.assertEqual(outer[:8], bytes(8))                # unlisted bytes are zero
            self.assertEqual(struct.unpack_from("<I", dict(g.buffers)[inner_p], 0)[0], 7)

    def test_biased_pointer(self):
        spec = parse("thiscall(ptr[0x10]:{4:ptr[0x80]+4:{0:u32{=3}}}) -> void")
        g = CaseGen(spec, 5)
        _, p = g.value(spec.args[0])
        bufs = dict(g.buffers)
        q = struct.unpack_from("<I", bufs[p], 4)[0]
        self.assertIn(q - 4, bufs)                                   # points 4 bytes into its buffer
        self.assertEqual(struct.unpack_from("<I", bufs[q - 4], 0)[0], 3)
        with self.assertRaises(SpecError):
            parse("cdecl(ptr[4]+4) -> void")

    def run_pair(self, original: bytes, reconstruction: bytes, spec: str) -> dict:
        gp4, dll = images(original, reconstruction)
        with patch("gp4re.config.oracle", side_effect=CFG.__getitem__):
            return differential(gp4, dll, Impl(TARGET, 0x10001000, "synthetic", spec), parse(spec),
                                fpucw=0x037F, cases=4, function_starts={TARGET, INIT})

    def test_nested_pointer_is_dereferenced_and_compared(self):
        # mov eax,[esp+4]; mov eax,[eax+8]; mov eax,[eax+K]; ret
        load = lambda k: bytes.fromhex("8b442404 8b4008 8b40") + bytes([k]) + b"\xC3"
        spec = "cdecl(ptr[0x10]:{8:ptr[0x20]:bytes}) -> i32"
        self.assertNotEqual(self.run_pair(load(4), load(4), spec)["status"], "fail")
        self.assertEqual(self.run_pair(load(4), load(8), spec)["status"], "fail")



class GfieldsTests(unittest.TestCase):
    """gfields={addr:type,...}: inline fixed-address game data (applied after globals=)."""
    DATA = 0x501000

    def images(self, original: bytes, reconstruction: bytes):
        gp4, dll = images(original, reconstruction)
        code = Section(".text", TARGET, 0xFF000, 0xFF000, 0x60000020, 0, True)
        data = Section(".data", self.DATA, 0x27000, 0x27000, 0xC0000040, 0, True)
        return Binary(Path("synthetic-gp4.exe"), b"", gp4.base, TARGET, gp4.image, [code, data]), dll

    def run_pair(self, original: bytes, reconstruction: bytes, spec: str) -> dict:
        gp4, dll = self.images(original, reconstruction)
        with patch("gp4re.config.oracle", side_effect=CFG.__getitem__):
            return differential(gp4, dll, Impl(TARGET, 0x10001000, "synthetic", spec), parse(spec),
                                fpucw=0x037F, cases=4, function_starts={TARGET, INIT})

    @staticmethod
    def load(k: int) -> bytes:   # mov eax,[0x501000]; mov eax,[eax+k]; ret
        return bytes.fromhex("a100105000 8b40") + bytes([k]) + bytes([0xC3])

    def test_parse(self):
        s = parse("cdecl() -> i32; gfields={0x501000:ptr[0x10]:{4:u32{=7,8}}, 0x501010*4/4:u8{=0,1}}")
        self.assertEqual([(a, n) for a, n, _, _ in s.gfields], [(0x501000, 1), (0x501010, 4)])
        with self.assertRaises(SpecError):
            parse("cdecl() -> void; gfields=0x501000:u32")
        with self.assertRaises(SpecError):
            parse("cdecl() -> void; gfields={*bytes, 0x501000:u32}")

    def test_global_pointer_is_generated_and_compared(self):
        spec = "cdecl() -> i32; gfields={0x501000:ptr[0x10]:{4:u32{=7,8}, 8:u32{=1,2}}}"
        self.assertNotEqual(self.run_pair(self.load(4), self.load(4), spec)["status"], "fail")
        self.assertEqual(self.run_pair(self.load(4), self.load(8), spec)["status"], "fail")

    def test_address_outside_writable_data_is_refused(self):
        res = self.run_pair(self.load(4), self.load(4), "cdecl() -> i32; gfields={0x401800:u32}")
        self.assertEqual(res["status"], "inconclusive")
        self.assertIn("gfields", res["reason"])


if __name__ == "__main__":
    unittest.main()
