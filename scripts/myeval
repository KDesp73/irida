#!/usr/bin/env bash

ENGINE="./build/bin/engine"

print_eval() {
    local fen="$1"
    local depth="$2"

    result=$("$ENGINE" eval "$fen")
    echo "$result"
}

# Default depth
depth=15
fen=""
file=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --depth) depth="$2"; shift ;;
        --file) file="$2"; shift ;;
        *) fen="$1" ;;
    esac
    shift
done

# Main logic
if [[ -n "$file" ]]; then
    if [[ ! -f "$file" ]]; then
        echo "Error: File '$file' not found." >&2
        exit 1
    fi
    while IFS= read -r line; do
        [[ -z "$line" ]] && continue
        print_eval "$line" "$depth"
    done < "$file"
elif [[ -n "$fen" ]]; then
    print_eval "$fen" "$depth"
else
    echo "Usage: $0 [--depth N] [--file file] [FEN]" >&2
    exit 1
fi

