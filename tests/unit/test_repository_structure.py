from pathlib import Path


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
        "docs/REQUIREMENTS.ja.md",
        "docs/CORE_DESIGN.ja.md",
        "docs/API_SKETCHES.ja.md",
        "docs/CONFIGURATION.ja.md",
        "docs/DEVELOPMENT_PLAN.ja.md",
        "docs/RELEASE_CHECKLIST.ja.md",
        "tests/README.ja.md",
        "tests/TEST_PLAN.ja.md",
    ]

    missing = [path for path in required if not (ROOT / path).exists()]

    assert missing == []


def test_library_properties_names_public_header():
    library_properties = (ROOT / "library.properties").read_text(encoding="utf-8")

    assert "name=ESP32KeyBridge" in library_properties
    assert "includes=ESP32KeyBridge.h" in library_properties
