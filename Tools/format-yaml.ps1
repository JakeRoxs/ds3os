<#
.SYNOPSIS
Formats all YAML files in the repository using Prettier.

.DESCRIPTION
Uses npx to run Prettier (so no global install is required). It honors .prettierignore.
#>

param()

$here = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $here\.. | Out-Null

# Ensure npm is available
if (-not (Get-Command npm -ErrorAction SilentlyContinue)) {
    Write-Error "npm is not available on PATH. Install Node.js to use this script."
    exit 1
}

npx prettier --write "**/*.{yml,yaml}"

Pop-Location | Out-Null
