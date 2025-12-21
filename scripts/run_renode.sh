#!/bin/bash
set -euo pipefail

# Resolve workspace root cross-platform. If running inside container, /workspace is used.
WS_DEFAULT="/workspace"
if [[ -d "$WS_DEFAULT" ]]; then
  WS_ROOT="$WS_DEFAULT"
else
  # Fallback to current directory when running on host
  WS_ROOT="$PWD"
fi

# Change to workspace root so Python peripheral paths resolve correctly
cd "$WS_ROOT"

PLATFORM="$WS_ROOT/simulation/renode/frdm_mcxn947.repl"
SCRIPT="$WS_ROOT/simulation/renode/TRL3_mcxn947_zephyr.resc"

if [[ ! -f "$PLATFORM" ]]; then
  echo "ERROR: Platform file not found: $PLATFORM" >&2
  exit 1
fi

if [[ ! -f "$SCRIPT" ]]; then
  echo "ERROR: Renode script not found: $SCRIPT" >&2
  exit 1
fi

echo "Running Renode with:" 
echo "  Platform: $PLATFORM"
echo "  Script  : $SCRIPT"

# Prefer renode-cli if available; fall back to renode.
if command -v renode-cli >/dev/null 2>&1; then
  renode-cli -e "machine LoadPlatformDescription @$PLATFORM; s @$SCRIPT"
else
  renode -e "machine LoadPlatformDescription @$PLATFORM; s @$SCRIPT"
fi
