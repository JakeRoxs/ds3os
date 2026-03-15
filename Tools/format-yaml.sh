#!/usr/bin/env bash
# Formats all YAML files in the repository using Prettier.
# Uses .prettierignore to skip vendored/generated directories.

set -euo pipefail

# Run from repo root (Tools/ is under repo root)
cd "$(dirname "$0")/.." || exit 1

# Use npx so users don't need to install prettier globally.
# This will install prettier locally in a cache if needed.

npx prettier --write "**/*.{yml,yaml}"
