#!/usr/bin/env bash
# Formats YAML and source files in the repository.
# Uses .prettierignore to skip vendored/generated directories for YAML.
# Uses git ls-files for Source/ files so generated/untracked content (like /build)
# is excluded automatically.

set -euo pipefail

# Run from repo root (Tools/ is under repo root)
cd "$(dirname "$0")/.." || exit 1

# Use npx so users don't need to install prettier globally.
# This will install prettier locally in a cache if needed.
echo "Formatting YAML files with Prettier..."
npx prettier --write "**/*.{yml,yaml}"

if command -v clang-format >/dev/null 2>&1; then
  echo "Formatting tracked Source/ C/C++ files with clang-format..."
  git ls-files Source \
    | while IFS= read -r file; do
        [[ -z "$file" ]] && continue
        case "${file##*.}" in
          h|hh|hpp|hxx|c|cc|cpp|cxx|inl|inc|ipp)
            clang-format -i --style=file "$file"
            ;;
        esac
      done
else
  echo "Warning: clang-format not found on PATH. Skipping C/C++ formatting." >&2
fi

if command -v dotnet >/dev/null 2>&1; then
  echo "Formatting tracked Source/ C# projects with dotnet format..."
  git ls-files 'Source/**/*.csproj' | while IFS= read -r proj; do
    [[ -n "$proj" ]] && dotnet format "$proj"
  done
else
  echo "Warning: dotnet not found on PATH. Skipping C# formatting." >&2
fi
