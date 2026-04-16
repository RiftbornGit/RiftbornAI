param(
    [string]$PluginRootOverride,
    [string]$UpdateZipPath,
    [string]$TargetPluginDir,
    [int]$WaitForProcessId = 0,
    [string]$ProjectPath,
    [switch]$Relaunch
)

# RiftbornAI — one-command install for Windows.
# Run once after dropping the plugin into <YourProject>/Plugins/.
#
# Steps:
#   1. Verify Node.js is present.
#   2. Fetch runtime dependencies for the MCP server (npm install --production).
#   3. Register the MCP server with Claude Desktop, VS Code, Cursor, and Windsurf.
#
# Update mode:
#   - Wait for the Unreal Editor process to exit.
#   - Replace the existing plugin folder with a downloaded release zip.
#   - Re-run the same install steps and optionally relaunch the project.
#
# Does not modify any Unreal Engine install. Does not require admin.

$ErrorActionPreference = "Stop"

function Resolve-RiftbornPluginRoot {
    param([string]$ExplicitPath)

    if ($ExplicitPath) {
        return (Resolve-Path -LiteralPath $ExplicitPath).Path
    }

    return (Split-Path -Parent $PSScriptRoot)
}

function Enable-RiftbornAiInProject {
    # Adds RiftbornAI to the host .uproject's Plugins list. Required because
    # several of RiftbornAI's dependencies (ChaosMover, LearningAgents,
    # ContextualAnimation, MassCrowd, USDImporter, PoseSearch, etc.) are
    # experimental UE plugins with EnabledByDefault=false. UE only cascades
    # auto-enable through dependency chains when the parent plugin is itself
    # enabled at the project level.
    param(
        [string]$PluginRoot
    )

    # Find the host .uproject by walking up from the plugin folder.
    # Plugin layout: <Project>/Plugins/RiftbornAI/  →  parent's parent has the .uproject
    $projectsDir = Split-Path -Parent (Split-Path -Parent $PluginRoot)
    $projectFile = Get-ChildItem -LiteralPath $projectsDir -Filter "*.uproject" -File -ErrorAction SilentlyContinue | Select-Object -First 1

    if (-not $projectFile)
    {
        Write-Host "  No .uproject found in $projectsDir — skipping project enable step." -ForegroundColor Yellow
        Write-Host "  (Run install.ps1 from inside <YourProject>/Plugins/RiftbornAI/Setup/ to auto-enable.)" -ForegroundColor Yellow
        return
    }

    Write-Host "  Found project: $($projectFile.Name)" -ForegroundColor Gray

    try
    {
        $raw = Get-Content -LiteralPath $projectFile.FullName -Raw
        $proj = $raw | ConvertFrom-Json

        if (-not $proj.Plugins)
        {
            $proj | Add-Member -NotePropertyName "Plugins" -NotePropertyValue @() -Force
        }

        $existing = $proj.Plugins | Where-Object { $_.Name -eq "RiftbornAI" } | Select-Object -First 1
        if ($existing)
        {
            if (-not $existing.Enabled)
            {
                $existing.Enabled = $true
                $proj.Plugins = @($proj.Plugins)
                $proj | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $projectFile.FullName -Encoding UTF8
                Write-Host "  Enabled existing RiftbornAI entry in $($projectFile.Name)." -ForegroundColor Green
            }
            else
            {
                Write-Host "  RiftbornAI already enabled in $($projectFile.Name)." -ForegroundColor Green
            }
            return
        }

        $newEntry = [pscustomobject]@{
            Name    = "RiftbornAI"
            Enabled = $true
        }
        $proj.Plugins = @($proj.Plugins) + $newEntry
        $proj | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $projectFile.FullName -Encoding UTF8
        Write-Host "  Added RiftbornAI to $($projectFile.Name) Plugins." -ForegroundColor Green
    }
    catch
    {
        Write-Host "  Could not edit $($projectFile.Name): $($_.Exception.Message)" -ForegroundColor Yellow
        Write-Host "  Add this manually to the .uproject Plugins array:" -ForegroundColor Yellow
        Write-Host '    { "Name": "RiftbornAI", "Enabled": true }' -ForegroundColor Yellow
    }
}

function Test-RiftbornMcpRuntime {
    param([string]$PluginRoot)

    $McpEntry = Join-Path $PluginRoot "mcp-server\dist\index.js"
    if (-not (Test-Path -LiteralPath $McpEntry)) {
        Write-Host "  MCP entrypoint missing at $McpEntry" -ForegroundColor Yellow
        return $false
    }

    $oldAuth = $env:RIFTBORN_AUTH_TOKEN
    $oldApi = $env:RIFTBORN_API_KEY
    $oldDev = $env:RIFTBORN_DEV_TOKEN
    try {
        $env:RIFTBORN_AUTH_TOKEN = ""
        $env:RIFTBORN_API_KEY = ""
        $env:RIFTBORN_DEV_TOKEN = ""
        & node $McpEntry --self-test --json | Out-Null
        return ($LASTEXITCODE -eq 0)
    }
    finally {
        $env:RIFTBORN_AUTH_TOKEN = $oldAuth
        $env:RIFTBORN_API_KEY = $oldApi
        $env:RIFTBORN_DEV_TOKEN = $oldDev
    }
}

function Invoke-RiftbornAiInstall {
    param([string]$PluginRoot)

    Write-Host ""
    Write-Host "=== RiftbornAI Setup ===" -ForegroundColor Cyan
    Write-Host "Plugin root: $PluginRoot"
    Write-Host ""

    Write-Host "[0/3] Enabling RiftbornAI in host project..." -ForegroundColor Cyan
    Enable-RiftbornAiInProject -PluginRoot $PluginRoot
    Write-Host ""

    try {
        $NodeVersion = & node --version 2>$null
        Write-Host "[1/3] Node.js detected: $NodeVersion" -ForegroundColor Green
    }
    catch {
        Write-Host "[1/3] Node.js NOT FOUND." -ForegroundColor Red
        Write-Host "      Install Node.js 18+ from https://nodejs.org and re-run this script."
        throw
    }

    Write-Host "[2/3] Verifying MCP server runtime..." -ForegroundColor Green
    $McpDir = Join-Path $PluginRoot "mcp-server"
    if (-not (Test-Path -LiteralPath $McpDir)) {
        throw "mcp-server/ not found at $McpDir"
    }

    if (Test-RiftbornMcpRuntime -PluginRoot $PluginRoot) {
        Write-Host "  Shipped MCP runtime already ready — skipping npm install." -ForegroundColor Green
    }
    else {
        Write-Host "  MCP runtime incomplete — installing production dependencies..." -ForegroundColor Yellow
        Push-Location $McpDir
        try {
            & npm install --omit=dev --ignore-scripts --no-audit --no-fund
            if ($LASTEXITCODE -ne 0) { throw "npm install failed (exit $LASTEXITCODE)" }
        }
        finally {
            Pop-Location
        }

        if (-not (Test-RiftbornMcpRuntime -PluginRoot $PluginRoot)) {
            throw "MCP server self-test failed after runtime install"
        }
    }

    Write-Host "[3/3] Registering MCP server with installed clients..." -ForegroundColor Green
    $Installer = Join-Path $PluginRoot "Scripts\install-mcp.mjs"
    if (-not (Test-Path -LiteralPath $Installer)) {
        throw "Scripts/install-mcp.mjs not found — cannot register automatically."
    }

    & node $Installer
    if ($LASTEXITCODE -ne 0) { throw "install-mcp.mjs failed (exit $LASTEXITCODE)" }

    Write-Host ""
    Write-Host "Done. Restart Claude Desktop / VS Code / Cursor / Windsurf to pick up the change." -ForegroundColor Green
    Write-Host "Open your Unreal project — the RiftbornAI tab appears under Window menu."
}

function Get-RiftbornExtractedPluginRoot {
    param([string]$ExtractRoot)

    if (Test-Path -LiteralPath (Join-Path $ExtractRoot "RiftbornAI.uplugin")) {
        return $ExtractRoot
    }

    $Candidates = Get-ChildItem -LiteralPath $ExtractRoot -Directory -ErrorAction Stop
    foreach ($Candidate in $Candidates) {
        $CandidatePlugin = Join-Path $Candidate.FullName "RiftbornAI.uplugin"
        if (Test-Path -LiteralPath $CandidatePlugin) {
            return $Candidate.FullName
        }
    }

    throw "Could not find an extracted RiftbornAI plugin root in $ExtractRoot"
}

function Invoke-RiftbornAiUpdate {
    param(
        [string]$ZipPath,
        [string]$PluginRoot,
        [int]$WaitForProcessId,
        [string]$ProjectPath,
        [bool]$ShouldRelaunch
    )

    if (-not (Test-Path -LiteralPath $ZipPath)) {
        throw "Update zip not found at $ZipPath"
    }

    if (-not (Test-Path -LiteralPath $PluginRoot)) {
        throw "Target plugin directory not found at $PluginRoot"
    }

    Write-Host ""
    Write-Host "=== RiftbornAI Update ===" -ForegroundColor Cyan
    Write-Host "Package: $ZipPath"
    Write-Host "Target:  $PluginRoot"
    Write-Host ""

    if ($WaitForProcessId -gt 0) {
        Write-Host "Waiting for Unreal Editor process $WaitForProcessId to exit..." -ForegroundColor Yellow
        Wait-Process -Id $WaitForProcessId -ErrorAction SilentlyContinue
    }

    $TempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("RiftbornAI-Update-" + [guid]::NewGuid().ToString("N"))
    $ExtractRoot = Join-Path $TempRoot "Extracted"
    $PluginParent = Split-Path -Parent $PluginRoot
    $BackupRoot = Join-Path $PluginParent ("RiftbornAI.backup-" + (Get-Date -Format "yyyyMMdd-HHmmss"))

    New-Item -ItemType Directory -Force -Path $ExtractRoot | Out-Null

    try {
        Write-Host "Extracting release package..." -ForegroundColor Green
        Expand-Archive -LiteralPath $ZipPath -DestinationPath $ExtractRoot -Force

        $StagedPluginRoot = Get-RiftbornExtractedPluginRoot -ExtractRoot $ExtractRoot

        Write-Host "Backing up current plugin..." -ForegroundColor Green
        Move-Item -LiteralPath $PluginRoot -Destination $BackupRoot

        try {
            Write-Host "Installing updated plugin files..." -ForegroundColor Green
            Move-Item -LiteralPath $StagedPluginRoot -Destination $PluginRoot
            Invoke-RiftbornAiInstall -PluginRoot $PluginRoot
        }
        catch {
            Write-Host "Update failed. Restoring previous plugin..." -ForegroundColor Yellow
            if (Test-Path -LiteralPath $PluginRoot) {
                Remove-Item -LiteralPath $PluginRoot -Recurse -Force -ErrorAction SilentlyContinue
            }
            if (Test-Path -LiteralPath $BackupRoot) {
                Move-Item -LiteralPath $BackupRoot -Destination $PluginRoot -ErrorAction Stop
            }
            throw
        }

        if (Test-Path -LiteralPath $BackupRoot) {
            Remove-Item -LiteralPath $BackupRoot -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
    finally {
        if (Test-Path -LiteralPath $TempRoot) {
            Remove-Item -LiteralPath $TempRoot -Recurse -Force -ErrorAction SilentlyContinue
        }
    }

    Write-Host ""
    Write-Host "RiftbornAI update applied successfully." -ForegroundColor Green

    if ($ShouldRelaunch -and $ProjectPath) {
        Write-Host "Relaunching project..." -ForegroundColor Green
        Start-Process -FilePath $ProjectPath | Out-Null
    }
}

$PluginRoot = if ($TargetPluginDir) {
    Resolve-RiftbornPluginRoot -ExplicitPath $TargetPluginDir
} else {
    Resolve-RiftbornPluginRoot -ExplicitPath $PluginRootOverride
}

if ($UpdateZipPath) {
    Invoke-RiftbornAiUpdate `
        -ZipPath (Resolve-Path -LiteralPath $UpdateZipPath).Path `
        -PluginRoot $PluginRoot `
        -WaitForProcessId $WaitForProcessId `
        -ProjectPath $ProjectPath `
        -ShouldRelaunch:$Relaunch.IsPresent
    return
}

Invoke-RiftbornAiInstall -PluginRoot $PluginRoot
