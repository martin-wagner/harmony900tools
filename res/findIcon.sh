#!/usr/bin/env bash

QUERY="${1:-}"

if [[ -z "$QUERY" ]]; then
    echo "usage: $0  <name>"
    exit 1
fi

find . -type f -iname "*$QUERY*.png"
