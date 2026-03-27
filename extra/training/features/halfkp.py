"""
HalfKP feature set for NNUE (Stockfish-style).

Each "half" is from one king's perspective: 64 king squares x (64 squares x 10 piece types + 1 bias) = 41,024
features per half. Index = king_sq * 641 + (piece_sq * 10 + piece_type). Piece types 0-4 = white P,N,B,R,Q;
5-9 = black P,N,B,R,Q. Squares: a8=0..h1=63 (rank*8+file, rank 0=8th).
"""

from __future__ import annotations

# @module features.halfkp
# @desc HalfKP feature indices: 41024 per half (64*641). Index = king_sq*641 + piece_sq*10 + piece_type.

# 64 * (64*10 + 1) = 41024 per half
HALFKP_FEATURES_PER_HALF = 64 * 641  # 41024
HALFKP_STRIDE = 641  # 64*10 + 1 (BONA_PIECE_ZERO at 640)

# FEN piece char -> (color, piece_type). piece_type 0=P,1=N,2=B,3=R,4=Q per color.
# White: 0-4, Black: 5-9
_FEN_TO_PT = {
    "P": (0, 0), "N": (0, 1), "B": (0, 2), "R": (0, 3), "Q": (0, 4),
    "p": (1, 5), "n": (1, 6), "b": (1, 7), "r": (1, 8), "q": (1, 9),
}


# @method _fen_to_board
# @desc Parse FEN piece placement into list of (square, piece_type). Square 0-63, a8=0.
# @param fen FEN string (at least piece placement).
# @returns list[tuple[int,int]] List of (square, piece_type) for each non-king piece.
def _fen_to_board(fen: str) -> list[tuple[int, int]]:
    """Parse FEN piece placement (first field) into (square, piece_type). Square 0-63, a8=0."""
    parts = fen.split()
    if not parts:
        raise ValueError("Invalid FEN")
    board = []
    row, col = 0, 0
    for c in parts[0]:
        if c == "/":
            row += 1
            col = 0
            continue
        if c.isdigit():
            col += int(c)
            continue
        if c in _FEN_TO_PT:
            _, pt = _FEN_TO_PT[c]
            sq = row * 8 + col
            if c.upper() != "K":
                board.append((sq, pt))
            col += 1
    return board


# @method _find_kings
# @desc Return (white_king_sq, black_king_sq) in 0..63 from FEN.
# @param fen FEN string with both kings.
# @returns tuple[int,int] (white_king_square, black_king_square).
def _find_kings(fen: str) -> tuple[int, int]:
    """Return (white_king_sq, black_king_sq). Squares 0-63."""
    parts = fen.split()
    wk, bk = -1, -1
    row, col = 0, 0
    for c in parts[0]:
        if c == "/":
            row += 1
            col = 0
            continue
        if c.isdigit():
            col += int(c)
            continue
        sq = row * 8 + col
        if c == "K":
            wk = sq
        elif c == "k":
            bk = sq
        col += 1
    if wk < 0 or bk < 0:
        raise ValueError("FEN must contain both kings")
    return (wk, bk)


# @method halfkp_indices_from_fen
# @desc Compute HalfKP feature indices for a position. Returns (white_half_indices,
# black_half_indices), each list of 11 indices (bias + 10 non-king pieces) in [0, 41023].
# @param fen FEN string with both kings.
# @returns tuple[list[int],list[int]] (white_half_indices, black_half_indices).
def halfkp_indices_from_fen(fen: str) -> tuple[list[int], list[int]]:
    """
    Compute HalfKP feature indices for a position.

    Returns (white_half_indices, black_half_indices). Each list has 1 + 10 = 11 indices
    (bias + 10 non-king pieces). Indices are in [0, 41023] for white half and [0, 41023] for black half.
    """
    wk_sq, bk_sq = _find_kings(fen)
    board = _fen_to_board(fen)

    def half_indices(king_sq: int) -> list[int]:
        # Bias: king_sq * 641 + 640
        indices = [king_sq * HALFKP_STRIDE + 640]
        for (sq, pt) in board:
            # Index = king_sq * 641 + piece_sq * 10 + piece_type
            indices.append(king_sq * HALFKP_STRIDE + sq * 10 + pt)
        return indices

    return (half_indices(wk_sq), half_indices(bk_sq))


# @method halfkp_num_features
# @desc Total HalfKP input features (both halves): 82048.
# @returns int 2 * FEATURES_PER_HALF.
def halfkp_num_features() -> int:
    """Total number of HalfKP input features (both halves)."""
    return 2 * HALFKP_FEATURES_PER_HALF  # 82048


# @class HalfKP
# @desc Holds constants for the HalfKP feature set: FEATURES_PER_HALF (41024),
# STRIDE (641), TOTAL_FEATURES (82048).
class HalfKP:
    """Constants for HalfKP feature set."""
    FEATURES_PER_HALF = HALFKP_FEATURES_PER_HALF
    STRIDE = HALFKP_STRIDE
    TOTAL_FEATURES = halfkp_num_features()
