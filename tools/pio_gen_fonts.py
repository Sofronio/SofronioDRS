"""PlatformIO pre-build hook: regenerate the subset fonts.

The generated header is committed, so a missing U8g2 library (fresh clone,
first dependency install) is not fatal: generation is skipped and the
committed subset is used.
"""
Import("env")

import os
import sys

project_dir = env.subst("$PROJECT_DIR")
env_name = env.subst("$PIOENV")
sys.path.insert(0, os.path.join(project_dir, "tools"))

try:
    from gen_subset_fonts import generate
    generate(project_dir, env_name)
except Exception as exc:  # noqa: BLE001 - never break the build on font tooling
    print("[fonts] subset generation skipped: %s" % exc)
