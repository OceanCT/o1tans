# download calgary and silesia datasets into the datasets folder and decompress them 
#!/bin/bash
set -e

mkdir -p datasets/silesia
mkdir -p datasets/calgary

echo "Downloading Silesia Corpus from Matt Mahoney mirror..."
SILESIA_URL="https://mattmahoney.net/dc/silesia.zip"
wget -O datasets/silesia/silesia.zip "$SILESIA_URL" || { echo "Failed to download Silesia"; exit 1; }

echo "Downloading Calgary Corpus (alternate link)…"
CALGARY_URL="https://mattmahoney.net/dc/calgary.zip"
wget -O datasets/calgary/calgary.zip "$CALGARY_URL" || { echo "Failed to download Calgary"; exit 1; }

echo "Extracting Silesia..."
unzip -o datasets/silesia/silesia.zip -d datasets/silesia

echo "Extracting Calgary..."
unzip -o datasets/calgary/calgary.zip -d datasets/calgary

echo "Done."