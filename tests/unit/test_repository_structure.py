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
        "docs/DATA_MODEL.ja.md",
        "docs/USE_CASES.ja.md",
        "docs/DECISIONS.ja.md",
        "docs/API_SKETCHES.ja.md",
        "docs/CONFIGURATION.ja.md",
        "docs/DEVELOPMENT_PLAN.ja.md",
        "docs/RELEASE_CHECKLIST.ja.md",
        "tests/README.ja.md",
        "tests/TEST_PLAN.ja.md",
        "tests/unit/core_smoke/core_smoke.ino",
        "tests/unit/core_smoke/sketch.yaml",
        "tests/unit/core_smoke/test_core_smoke.py",
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


def readme_section(readme, title):
    return readme.split(f"## {title}", 1)[1].split("\n## ", 1)[0]


def test_examples_readmes_list_current_examples():
    groups = {
        "Practical Examples": "practical",
        "Usage Examples": "usage",
    }
    for readme_name in ["README.ja.md", "README.md"]:
        readme = (ROOT / "examples" / readme_name).read_text(encoding="utf-8")
        for title, group in groups.items():
            example_dirs = sorted(
                path.parent.name for path in (ROOT / "examples" / group).glob("*/*.ino")
            )
            listed = sorted(re.findall(r"- `([^`]+)`:", readme_section(readme, title)))

            assert listed == example_dirs, (readme_name, group)


def test_examples_are_grouped_with_docs_and_compile_profiles():
    missing = []
    for ino in sorted((ROOT / "examples").rglob("*.ino")):
        example_dir = ino.parent
        assert example_dir.parent.name in {"practical", "usage"}, str(example_dir)
        for required in ["README.ja.md", "sketch.yaml"]:
            if not (example_dir / required).exists():
                missing.append(str((example_dir / required).relative_to(ROOT)))

    assert missing == []
