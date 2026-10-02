"""Fail-closed structural and runtime-exception gates, without game files."""
import sqlite3
import struct
import unittest
from pathlib import Path
from unittest.mock import patch

from gp4re import shape, verify
from gp4re.binary import Binary, Section
from gp4re.emu import Impl


class GateSoundnessTests(unittest.TestCase):
    def setUp(self):
        self.con = sqlite3.connect(":memory:")
        self.con.row_factory = sqlite3.Row
        self.con.executescript("""
            CREATE TABLE functions(addr INTEGER, n_insn INTEGER, x87 INTEGER, n_cond INTEGER);
            CREATE TABLE edges(src INTEGER, dst INTEGER);
            CREATE TABLE func_imports(addr INTEGER, import TEXT);
            CREATE TABLE func_globals(addr INTEGER, gaddr INTEGER);
            CREATE TABLE strings(gaddr INTEGER, text TEXT);
            INSERT INTO functions VALUES (4198400, 1, 0, 0);
        """)
        self.rec = Impl(0x401000, 0x10001000, "FUN_00401000", "cdecl() -> void")

    def tearDown(self):
        self.con.close()

    def audit(self, **changes):
        facts = {"callees": set(), "imports": set(), "globals": set(), "strings": set(), "x87": 0, "n_cond": 0}
        facts.update(changes)
        with patch.object(shape, "recon_facts", return_value=facts):
            return shape.audit(self.con, None, None, self.rec, [self.rec])

    def test_tiny_function_cannot_add_a_callee(self):
        self.assertEqual(self.audit(callees={0x402000})["tier"], "C")

    def test_nontrivial_function_cannot_drop_one_callee(self):
        self.con.execute("UPDATE functions SET n_insn=4")
        self.con.execute("INSERT INTO edges VALUES (?, ?)", (self.rec.addr, 0x402000))
        self.assertEqual(self.audit()["tier"], "C")

    def test_tiny_function_cannot_add_an_import(self):
        self.assertEqual(self.audit(imports={"kernel32.dll!Sleep"})["tier"], "C")

    def test_self_delegation_is_an_extra_callee(self):
        self.assertEqual(self.audit(callees={self.rec.addr})["tier"], "C")

    def test_direct_call_back_to_original_is_discovered(self):
        def binary(base, code):
            image = bytearray(0x2000)
            image[0x1000:0x1000 + len(code)] = code
            section = Section(".text", base + 0x1000, 0x1000, 0x1000, 0x60000020, 0, True)
            return Binary(Path("synthetic.exe"), b"", base, base + 0x1000, image, [section])
        gp4 = binary(0x400000, b"\xc3")
        call = b"\xe8" + struct.pack("<i", self.rec.addr - (self.rec.impl + 5)) + b"\xc3"
        dll = binary(0x10000000, call)
        facts = shape.recon_facts(dll, gp4, self.rec, [self.rec])
        self.assertEqual(facts["callees"], {self.rec.addr})

    def test_runtime_skip_requires_named_reachable_import(self):
        self.assertTrue(verify.skip_problems(self.con, self.rec.addr, "unspecified"))
        self.assertTrue(verify.skip_problems(self.con, self.rec.addr, "win32 Sleep"))
        self.con.execute("INSERT INTO edges VALUES (?, ?)", (self.rec.addr, 0x402000))
        self.con.execute("INSERT INTO edges VALUES (?, ?)", (0x402000, self.rec.addr))
        self.con.execute("INSERT INTO func_imports VALUES (?, ?)", (0x402000, "kernel32.dll!Sleep"))
        self.assertFalse(verify.skip_problems(self.con, self.rec.addr, "win32 Sleep"))
        self.assertTrue(verify.skip_problems(self.con, self.rec.addr, "win32 ExitProcess"))
        self.assertFalse(verify.skip_problems(self.con, self.rec.addr, None))


if __name__ == "__main__":
    unittest.main()
