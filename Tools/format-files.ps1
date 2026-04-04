<#
.SYNOPSIS
Formats YAML and source files in the repository.

.DESCRIPTION
Uses npx to run Prettier for YAML (so no global install is required), then formats
tracked Source/ C/C++ files via clang-format and Source/ C# projects via dotnet format.
Using git ls-files ensures generated/untracked content (like /build) is excluded.
`Source/ThirdParty` is explicitly excluded from Source formatting steps.
#>

param()

$here = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $here\.. | Out-Null

try {
    # Ensure npm is available
    if (-not (Get-Command npm -ErrorAction SilentlyContinue)) {
        Write-Error "npm is not available on PATH. Install Node.js to use this script."
        exit 1
    }

    Write-Host "Formatting YAML files with Prettier..."
    npx prettier --write "**/*.{yml,yaml}"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    if (Get-Command clang-format -ErrorAction SilentlyContinue) {
        Write-Host "Formatting tracked Source/ C/C++ files with clang-format..."
        $clangExt = @('.h', '.hh', '.hpp', '.hxx', '.c', '.cc', '.cpp', '.cxx', '.inl', '.inc', '.ipp')
        $clangFiles = git ls-files Source | Where-Object {
            if ($_ -like 'Source/ThirdParty/*') { return $false }
            $ext = [System.IO.Path]::GetExtension($_).ToLowerInvariant()
            $clangExt -contains $ext
        }
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

        foreach ($file in $clangFiles) {
            clang-format -i --style=file $file
            if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        }
    }
    else {
        Write-Warning "clang-format not found on PATH. Skipping C/C++ formatting."
    }

    if (Get-Command dotnet -ErrorAction SilentlyContinue) {
        Write-Host "Formatting tracked Source/ C# projects with dotnet format..."
        $csprojFiles = git ls-files "Source/**/*.csproj" | Where-Object { $_ -notlike 'Source/ThirdParty/*' }
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

        foreach ($proj in $csprojFiles) {
            dotnet format $proj
            if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        }
    }
    else {
        Write-Warning "dotnet not found on PATH. Skipping C# formatting."
    }
}
finally {
    Pop-Location | Out-Null
}
