#!/usr/bin/env bash
# Set up some variables
os_type=$(uname)

if [ "$os_type" == 'Linux' ]; then
	echo "[INFO] Detected OS: Linux"
	chromium_binary="Chromium/chrome"
elif [ "$os_type" == 'Darwin' ]; then
	echo "[INFO] Detected OS: Mac OS X"
	chromium_binary="Chromium/Chromium.app/Contents/MacOS/Chromium"
else
	echo "[ERROR] Unsupported OS type: $os_type"
fi

if [ "$1" == '--use-default-profile' ]; then
	# Run ShadowBlock with user's default Chromium profile
	echo "[INFO] Running ShadowBlock with your default Chromium profile..."
	$chromium_binary --no-sandbox
else
	echo "[INFO] Running ShadowBlock with a new profile..."
	$chromium_binary --no-sandbox --user-data-dir=shadowblock_profile
fi