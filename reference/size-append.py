#!/usr/bin/env python3
"""Emit the existing compression trailer without eager make-time subprocesses."""
import os,struct,sys
sys.stdout.buffer.write(struct.pack('<I',sum(os.lstat(p).st_size for p in sys.argv[1:]) & 0xffffffff))
