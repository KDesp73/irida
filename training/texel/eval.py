"""
PeSTO-style eval clone for Texel tuning.

Computes material + PST score with game-phase interpolation, matching
src/eval/pesto.c so we can tune mg_value and eg_value (and optionally K).
Eval is returned from White's perspective (positive = good for White).
"""

from __future__ import annotations

import numpy as np
from typing import Tuple

# Piece types: 0=pawn, 1=knight, 2=bishop, 3=rook, 4=queen, 5=king
# gamephase_inc[2*p+color]: 0,0 for pawns, 1,1 for knight,bishop,rook, 2,2 for queen, 4,4 for king
GAMEPHASE_INC = (0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 4, 4)

# PSTs from pesto.c (same order: a1..h1, a2..h2, ..., a8..h8; rank 0 = white back rank)
MG_PAWN = np.array([
    0, 0, 0, 0, 0, 0, 0, 0,
    98, 134, 61, 95, 68, 126, 34, -11,
    -6, 7, 26, 31, 65, 56, 25, -20,
    -14, 13, 6, 21, 23, 12, 17, -23,
    -27, -2, -5, 12, 17, 6, 10, -25,
    -26, -4, -4, -10, 3, 3, 33, -12,
    -35, -1, -20, -23, -15, 24, 38, -22,
    0, 0, 0, 0, 0, 0, 0, 0,
], dtype=np.int32)
EG_PAWN = np.array([
    0, 0, 0, 0, 0, 0, 0, 0,
    178, 173, 158, 134, 147, 132, 165, 187,
    94, 100, 85, 67, 56, 53, 82, 84,
    32, 24, 13, 5, -2, 4, 17, 17,
    13, 9, -3, -7, -7, -8, 3, -1,
    4, 7, -6, 1, 0, -5, -1, -8,
    13, 8, 8, 10, 13, 0, 2, -7,
    0, 0, 0, 0, 0, 0, 0, 0,
], dtype=np.int32)
MG_KNIGHT = np.array([
    -167, -89, -34, -49, 61, -97, -15, -107,
    -73, -41, 72, 36, 23, 62, 7, -17,
    -47, 60, 37, 65, 84, 129, 73, 44,
    -9, 17, 19, 53, 37, 69, 18, 22,
    -13, 4, 16, 13, 28, 19, 21, -8,
    -23, -9, 12, 10, 19, 17, 25, -16,
    -29, -53, -12, -3, -1, 18, -14, -19,
    -105, -21, -58, -33, -17, -28, -19, -23,
], dtype=np.int32)
EG_KNIGHT = np.array([
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25, -8, -25, -2, -9, -25, -24, -52,
    -24, -20, 10, 9, -1, -9, -19, -41,
    -17, 3, 22, 22, 22, 11, 8, -18,
    -18, -6, 16, 25, 16, 17, 4, -18,
    -23, -3, -1, 15, 10, -3, -20, -22,
    -42, -20, -10, -5, -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
], dtype=np.int32)
MG_BISHOP = np.array([
    -29, 4, -82, -37, -25, -42, 7, -8,
    -26, 16, -18, -13, 30, 59, 18, -47,
    -16, 37, 43, 40, 35, 50, 37, -2,
    -4, 5, 19, 50, 37, 37, 7, -2,
    -6, 13, 13, 26, 34, 12, 10, 4,
    0, 15, 15, 15, 14, 27, 18, 10,
    4, 15, 16, 0, 7, 21, 33, 1,
    -33, -3, -14, -21, -13, -12, -39, -21,
], dtype=np.int32)
EG_BISHOP = np.array([
    -14, -21, -11, -8, -7, -9, -17, -24,
    -8, -4, 7, -12, -3, -13, -4, -14,
    2, -8, 0, -1, -2, 6, 0, 4,
    -3, 9, 12, 9, 14, 10, 3, 2,
    -6, 3, 13, 19, 7, 10, -3, -9,
    -12, -3, 8, 10, 13, 3, -7, -15,
    -14, -18, -7, -1, 4, -9, -15, -27,
    -23, -9, -23, -5, -9, -16, -5, -17,
], dtype=np.int32)
MG_ROOK = np.array([
    32, 42, 32, 51, 63, 9, 31, 43,
    27, 32, 58, 62, 80, 67, 26, 44,
    -5, 19, 26, 36, 17, 45, 61, 16,
    -24, -11, 7, 26, 24, 35, -8, -20,
    -36, -26, -12, -1, 9, -7, 6, -23,
    -45, -25, -16, -17, 3, 0, -5, -33,
    -44, -16, -20, -9, -1, 11, -6, -71,
    -19, -13, 1, 17, 16, 7, -37, -26,
], dtype=np.int32)
EG_ROOK = np.array([
    13, 10, 18, 15, 12, 12, 8, 5,
    11, 13, 13, 11, -3, 3, 8, 3,
    7, 7, 7, 5, 4, -3, -5, -3,
    4, 3, 13, 1, 2, 1, -1, 2,
    3, 5, 8, 4, -5, -6, -8, -11,
    -4, 0, -5, -1, -7, -12, -8, -16,
    -6, -6, 0, 2, -9, -9, -11, -3,
    -9, 2, 3, -1, -5, -13, 4, -20,
], dtype=np.int32)
MG_QUEEN = np.array([
    -28, 0, 29, 12, 59, 44, 43, 45,
    -24, -39, -5, 1, -16, 57, 28, 54,
    -13, -17, 7, 8, 29, 56, 47, 57,
    -27, -27, -16, -16, -1, 17, -2, 1,
    -9, -26, -9, -10, -2, -4, 3, -3,
    -14, 2, -11, -2, -5, 2, 14, 5,
    -35, -8, 11, 2, 8, 15, -3, 1,
    -1, -18, -9, 10, -15, -25, -31, -50,
], dtype=np.int32)
EG_QUEEN = np.array([
    -9, 22, 22, 27, 27, 19, 10, 20,
    -17, 20, 32, 41, 58, 25, 30, 0,
    -20, 6, 9, 49, 47, 35, 19, 9,
    3, 22, 24, 45, 57, 40, 57, 36,
    -18, 28, 19, 47, 31, 34, 39, 23,
    -16, -27, 15, 6, 9, 17, 10, 5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43, -5, -32, -20, -41,
], dtype=np.int32)
MG_KING = np.array([
    -65, 23, 16, -15, -56, -34, 2, 13,
    29, -1, -20, -7, -8, -4, -38, -29,
    -9, 24, 2, -16, -20, 6, 22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49, -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
    1, 7, -8, -64, -43, -16, 9, 8,
    -15, 36, 12, -54, 8, -28, 24, 14,
], dtype=np.int32)
EG_KING = np.array([
    -74, -35, -18, -18, -11, 15, 4, -17,
    -12, 17, 14, 17, 17, 38, 23, 11,
    10, 17, 23, 15, 20, 45, 44, 13,
    -8, 22, 24, 27, 26, 33, 26, 3,
    -18, -4, 21, 24, 27, 23, 9, -11,
    -19, -3, 11, 21, 23, 16, 7, -9,
    -27, -11, 4, 13, 14, 4, -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43,
], dtype=np.int32)

MG_PST = (MG_PAWN, MG_KNIGHT, MG_BISHOP, MG_ROOK, MG_QUEEN, MG_KING)
EG_PST = (EG_PAWN, EG_KNIGHT, EG_BISHOP, EG_ROOK, EG_QUEEN, EG_KING)

# char -> (piece_type 0..5, color 0=white 1=black); -1 for empty
PIECE_MAP = {
    'P': (0, 0), 'N': (1, 0), 'B': (2, 0), 'R': (3, 0), 'Q': (4, 0), 'K': (5, 0),
    'p': (0, 1), 'n': (1, 1), 'b': (2, 1), 'r': (3, 1), 'q': (4, 1), 'k': (5, 1),
}


def flip_sq(sq: int) -> int:
    return sq ^ 56


def build_tables(mg_value: np.ndarray, eg_value: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    """Build mg_table[12][64] and eg_table[12][64] like pesto_init."""
    mg_table = np.zeros((12, 64), dtype=np.int32)
    eg_table = np.zeros((12, 64), dtype=np.int32)
    for p in range(6):
        for sq in range(64):
            mg_table[2 * p + 0][sq] = mg_value[p] + MG_PST[p][sq]
            eg_table[2 * p + 0][sq] = eg_value[p] + EG_PST[p][sq]
            fsq = flip_sq(sq)
            mg_table[2 * p + 1][sq] = mg_value[p] + MG_PST[p][fsq]
            eg_table[2 * p + 1][sq] = eg_value[p] + EG_PST[p][fsq]
    return mg_table, eg_table


def fen_to_board(fen: str) -> Tuple[list[list[str]], bool]:
    """Parse FEN; return (board 8x8, white_to_move). Board[r][f] = piece char or '.'."""
    parts = fen.split()
    board_str = parts[0]
    white_to_move = len(parts) < 2 or parts[1].lower() == 'w'
    board = [['.'] * 8 for _ in range(8)]
    r, f = 0, 0
    for c in board_str:
        if c == '/':
            r += 1
            f = 0
            continue
        if c.isdigit():
            f += int(c)
            continue
        if r < 8 and f < 8:
            board[r][f] = c
        f += 1
    return board, white_to_move


def eval_fen(fen: str, mg_value: np.ndarray, eg_value: np.ndarray) -> float:
    """
    Eval position from White's perspective (positive = good for White).
    Uses only material + PST with game-phase interpolation.
    """
    board, _ = fen_to_board(fen)
    mg_table, eg_table = build_tables(mg_value, eg_value)
    mg_acc = [0, 0]
    eg_acc = [0, 0]
    game_phase = 0
    for r in range(8):
        for f in range(8):
            c = board[r][f]
            if c == '.' or c not in PIECE_MAP:
                continue
            pt, col = PIECE_MAP[c]
            pc = 2 * pt + col
            sq = r * 8 + f
            mg_acc[col] += mg_table[pc][sq]
            eg_acc[col] += eg_table[pc][sq]
            game_phase += GAMEPHASE_INC[pc]
    game_phase = min(game_phase, 24)
    eg_phase = 24 - game_phase
    mg_score = mg_acc[0] - mg_acc[1]
    eg_score = eg_acc[0] - eg_acc[1]
    return (mg_score * game_phase + eg_score * eg_phase) / 24.0
