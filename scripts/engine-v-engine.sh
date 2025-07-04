#!/usr/bin/env bash

ENGINE_VERSION1="$1"
ENGINE_VERSION2="$2"
OUTPUT="./bin/past/$ENGINE_VERSION1-v-$ENGINE_VERSION2.out"

touch "$OUTPUT"
cutechess-cli -engine cmd="./bin/past/engine-$ENGINE_VERSION1" -engine cmd="./bin/past/engine-$ENGINE_VERSION2" -each proto=uci tc=inf -concurrency 2 -rounds 5 -openings file=./bin/books/book.bin > "$OUTPUT"
