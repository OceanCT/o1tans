# Data Corpus Downloader for Windows (PowerShell)
# This script downloads and extracts the Calgary and Silesia corpora
# into the 'datasets' folder, similar to the original Bash script.

# Enable strict mode for error checking
$ErrorActionPreference = "Stop"

# --- Configuration ---
$SILESIA_URL = "https://mattmahoney.net/dc/silesia.zip"
$CALGARY_URL = "https://mattmahoney.net/dc/calgary.zip"
$SilesiaDir = "datasets/silesia"
$CalgaryDir = "datasets/calgary"
$SilesiaZipPath = Join-Path $SilesiaDir "silesia.zip"
$CalgaryZipPath = Join-Path $CalgaryDir "calgary.zip"
# ---------------------

Write-Host "Creating dataset directories..."
try {
    # -Force ensures directories are created if they don't exist, and no error occurs if they do.
    New-Item -ItemType Directory -Force -Path $SilesiaDir | Out-Null
    New-Item -ItemType Directory -Force -Path $CalgaryDir | Out-Null
    Write-Host "Directories created successfully."
} catch {
    Write-Error "Failed to create directories: $($_.Exception.Message)"
    exit 1
}

# --- Download Silesia Corpus ---
Write-Host "Downloading Silesia Corpus from Matt Mahoney mirror..."
try {
    # Invoke-WebRequest is the PowerShell cmdlet for downloading files.
    Invoke-WebRequest -Uri $SILESIA_URL -OutFile $SilesiaZipPath
    Write-Host "Silesia download complete."
} catch {
    Write-Error "Failed to download Silesia Corpus: $($_.Exception.Message)"
    exit 1
}

# --- Download Calgary Corpus ---
Write-Host "Downloading Calgary Corpus (alternate link)…"
try {
    Invoke-WebRequest -Uri $CALGARY_URL -OutFile $CalgaryZipPath
    Write-Host "Calgary download complete."
} catch {
    Write-Error "Failed to download Calgary Corpus: $($_.Exception.Message)"
    exit 1
}

# --- Extract Silesia ---
Write-Host "Extracting Silesia..."
try {
    # Expand-Archive is the native PowerShell command for decompressing ZIP files.
    # -Force overwrites existing files without prompting (like 'unzip -o').
    Expand-Archive -Path $SilesiaZipPath -DestinationPath $SilesiaDir -Force
    Write-Host "Silesia extracted successfully."
} catch {
    Write-Error "Failed to extract Silesia: $($_.Exception.Message)"
    exit 1
}

# --- Extract Calgary ---
Write-Host "Extracting Calgary..."
try {
    Expand-Archive -Path $CalgaryZipPath -DestinationPath $CalgaryDir -Force
    Write-Host "Calgary extracted successfully."
} catch {
    Write-Error "Failed to extract Calgary: $($_.Exception.Message)"
    exit 1
}

Write-Host ""
Write-Host "Done. Datasets are ready in the 'datasets' folder."