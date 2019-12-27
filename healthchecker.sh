#!/usr/bin/env bash

# Bash options for strict error checking
set -o errexit -o errtrace -o pipefail -o nounset

RC="$(curl \
    --silent \
    --no-buffer \
    --write-out '%{http_code}' \
    --header "Connection: Upgrade" \
    --header "Upgrade: websocket" \
    --header "Host: 127.0.0.1:8888" \
    --header "Origin: 127.0.0.1" \
    http://127.0.0.1:8888/kurento)"

if [[ "$RC" == "500" ]]; then
    exit 0
else
    exit 1
fi