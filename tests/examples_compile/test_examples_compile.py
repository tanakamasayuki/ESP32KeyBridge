import os
import shutil
import subprocess
from pathlib import Path

import pytest


REPO_ROOT = Path(__file__).resolve().parents[2]
EXAMPLES_DIR = REPO_ROOT / "examples"


def example_dirs():
    selected = os.environ.get("ESP32KEYBRIDGE_EXAMPLES")
    examples = sorted(path.parent for path in EXAMPLES_DIR.glob("*/*.ino"))
    if not selected:
        return examples

    names = {name.strip() for name in selected.split(",") if name.strip()}
    return [path for path in examples if path.name in names]


def test_selected_examples_exist():
    selected = os.environ.get("ESP32KEYBRIDGE_EXAMPLES")
    if not selected:
        return

    names = {name.strip() for name in selected.split(",") if name.strip()}
    available = {path.parent.name for path in EXAMPLES_DIR.glob("*/*.ino")}
    missing = sorted(names - available)
    assert missing == []


@pytest.mark.parametrize("example_dir", example_dirs(), ids=lambda path: path.name)
def test_example_compile(example_dir):
    if shutil.which("arduino-cli") is None:
        pytest.skip("arduino-cli is not available")

    result = subprocess.run(
        ["arduino-cli", "compile", "--profile", "s3", str(example_dir)],
        cwd=REPO_ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    assert result.returncode == 0, result.stdout
