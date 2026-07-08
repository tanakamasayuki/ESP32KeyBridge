from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]


def test_required_project_files_exist():
    required = [
        "README.ja.md",
        "README.md",
        "library.properties",
        "keywords.txt",
        "CHANGELOG.md",
        "src/ESP32KeyBridge.h",
        "src/ESP32KeyBridge.cpp",
        "examples/README.ja.md",
        "examples/README.md",
        "docs/REQUIREMENTS.ja.md",
        "docs/CORE_DESIGN.ja.md",
        "docs/API_SKETCHES.ja.md",
        "docs/CONFIGURATION.ja.md",
        "docs/DEVELOPMENT_PLAN.ja.md",
        "docs/RELEASE_CHECKLIST.ja.md",
        "tests/README.ja.md",
        "tests/TEST_PLAN.ja.md",
        "tests/single/README.ja.md",
        "tests/peer/README.ja.md",
        "tests/peer/README.md",
        "tests/manual/README.ja.md",
        "tests/manual/README.md",
    ]

    missing = [path for path in required if not (ROOT / path).exists()]

    assert missing == []


def test_library_properties_names_public_header():
    library_properties = (ROOT / "library.properties").read_text(encoding="utf-8")

    assert "name=ESP32KeyBridge" in library_properties
    assert "includes=ESP32KeyBridge.h" in library_properties


def test_examples_readme_lists_current_examples():
    example_dirs = sorted(path.parent.name for path in (ROOT / "examples").glob("*/*.ino"))
    readme = (ROOT / "examples" / "README.ja.md").read_text(encoding="utf-8")
    core_section = readme.split("## Core Examples", 1)[1].split("## 追加予定", 1)[0]
    listed = sorted(re.findall(r"- `([^`]+)`:", core_section))

    assert listed == example_dirs


def test_examples_have_docs_and_compile_profiles():
    missing = []
    for ino in sorted((ROOT / "examples").glob("*/*.ino")):
        example_dir = ino.parent
        for required in ["README.ja.md", "sketch.yaml"]:
            if not (example_dir / required).exists():
                missing.append(str((example_dir / required).relative_to(ROOT)))

    assert missing == []
