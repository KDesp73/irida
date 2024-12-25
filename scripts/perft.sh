#!/usr/bin/env bash

depth="$1"
fen="$2"
moves="$3"

./engine perft "$depth" "$fen" "$moves"
