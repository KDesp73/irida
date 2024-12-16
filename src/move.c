#include "board.h"
#include "move.h"
#include "piece.h"
#include "square.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <chess/piece.h>

#include <stdlib.h>

void BitboardToMoves(Bitboard bitboard, Square from, Move moves[])
{
    size_t index = 0;
    while (bitboard) {
        Square to = poplsb(&bitboard);
        moves[index++] = MoveEncode(from, to, PROMOTION_NONE, FLAG_NORMAL);
    }
}

Move SquaresToMove(square_t from, square_t to, uint8_t promotion, uint8_t flags)
{
    // Encode the 'rank' and 'file' as a 6-bit index (0-63)
    size_t from_index = (from.rank * BOARD_SIZE) + from.file; // Calculate 0-63 index from rank/file
    size_t to_index = (to.rank * BOARD_SIZE) + to.file;       // Calculate 0-63 index from rank/file

    // Build Move by packing from_index, to_index, promotion, and flags
    return (from_index & 0x3F) | 
           ((to_index & 0x3F) << 6) | 
           ((promotion & 0xF) << 12) | 
           ((flags & 0x3) << 16);
}

void MoveToSquares(Move move, square_t* from, square_t* to, uint8_t* promotion, uint8_t* flags)
{
    size_t from_index = move & 0x3F;               // Extract bits 0-5 for from index
    size_t to_index = (move >> 6) & 0x3F;          // Extract bits 6-11 for to index
    *promotion = (move >> 12) & 0xF;               // Extract bits 12-15 for promotion
    *flags = (move >> 16) & 0x3;                   // Extract bits 16-17 for flags

    // Convert the indices back to rank/file for both from and to squares
    from->rank = from_index / BOARD_SIZE;          // Rank = index / BOARD_SIZE
    from->file = from_index % BOARD_SIZE;          // File = index % BOARD_SIZE
    to->rank = to_index / BOARD_SIZE;              // Rank = index / BOARD_SIZE
    to->file = to_index % BOARD_SIZE;              // File = index % BOARD_SIZE

    // Optional: Fill in `name` or other fields if needed
    snprintf(from->name, sizeof(from->name), "%c%d", 'A' + (int)from->file, BOARD_SIZE - (int)from->rank);
    snprintf(to->name, sizeof(to->name), "%c%d", 'A' + (int)to->file, BOARD_SIZE - (int)to->rank);
}

Move MoveEncodeNames(const char* from, const char* to, uint8_t promotion, uint8_t flag)
{
    return MoveEncode(NameToSquare(from), NameToSquare(to), promotion, flag);
}

Move MoveEncode(Square from, Square to, uint8_t promotion, uint8_t flag)
{
    return (from & 0x3F) | 
           ((to & 0x3F) << 6) | 
           ((promotion & 0xF) << 12) | 
           ((flag & 0x7) << 16);
}

void MoveDecode(Move move, Square* from, Square* to, uint8_t* promotion, uint8_t* flag)
{
    *from = move & 0x3F;
    *to = (move >> 6) & 0x3F;
    *promotion = (move >> 12) & 0xF;
    *flag = (move >> 16) & 0x7;
}

Square GetFrom(Move move)
{
    Square from, to;
    uint8_t promotion, flags;
    MoveDecode(move, &from, &to, &promotion, &flags);

    return from;
}
Square GetTo(Move move)
{
    Square from, to;
    uint8_t promotion, flags;
    MoveDecode(move, &from, &to, &promotion, &flags);

    return to;
}
uint8_t GetPromotion(Move move)
{
    Square from, to;
    uint8_t promotion, flags;
    MoveDecode(move, &from, &to, &promotion, &flags);

    return promotion;
}
uint8_t GetFlag(Move move)
{
    Square from, to;
    uint8_t promotion, flags;
    MoveDecode(move, &from, &to, &promotion, &flags);

    return flags;
}

void MoveFreely(Board* board, Move move, Color color)
{
    Square from, to;
    uint8_t promotion, flags;
    MoveDecode(move, &from, &to, &promotion, &flags);

    uint64_t from_bb = 1ULL << from;
    uint64_t to_bb = 1ULL << to;

    // Find the piece that moved
    for (int piece = 0; piece < 6; piece++) {
        if (board->bitboards[color * 6 + piece] & from_bb) {
            // Move the piece
            board->bitboards[color * 6 + piece] ^= from_bb; // Remove from source
            if (promotion) {
                // Add promoted piece
                board->bitboards[color * 6 + promotion - 1] |= to_bb;
            } else {
                // Move to destination
                board->bitboards[color * 6 + piece] |= to_bb;
            }
            break;
        }
    }

    // Handle captures
    int opponent = 1 - color;
    for (int piece = 0; piece < 6; piece++) {
        if (board->bitboards[opponent * 6 + piece] & to_bb) {
            board->bitboards[opponent * 6 + piece] ^= to_bb;
            break;
        }
    }

    // Special moves
    if (flags == 1) {
        // Handle castling
        if (to == 6) { // Kingside castling
            board->bitboards[color * 6 + 0] ^= (1ULL << 7) | (1ULL << 5); // Move rook
        } else if (to == 2) { // Queenside castling
            board->bitboards[color * 6 + 0] ^= (1ULL << 0) | (1ULL << 3); // Move rook
        }
    } else if (flags == 2) {
        // Handle en passant
        board->bitboards[opponent * 6 + 5] ^= (1ULL << (to - (color ? 8 : -8)));
    }
}

void MovePrint(Move move)
{
    Square from, to;
    uint8_t promotion, flags;
    MoveDecode(move, &from, &to, &promotion, &flags);

    char square_from[3], square_to[3];
    SquareToName(square_from, from);
    SquareToName(square_to, to);
    printf("Move: %s -> %s, Promotion: %d, Flags: %d\n",
            square_from, square_to,
            promotion, flags);
}

_Bool MoveIsValid(const Board* board, Move move, Color color)
{
    Square from, to;
    uint8_t promotion, flags;
    MoveDecode(move, &from, &to, &promotion, &flags);
    uint64_t from_bb = 1ULL << from;
    uint64_t to_bb = 1ULL << to;

    for (int piece = 0; piece < 6; piece++) {
        if (board->bitboards[color * 6 + piece] & from_bb) {
            return 1;
        }
    }

    return 0;
}

char PromotionToChar(uint8_t promotion)
{
    switch (promotion) {
    case PROMOTION_QUEEN: return 'Q';
    case PROMOTION_ROOK: return 'R';
    case PROMOTION_BISHOP: return 'B';
    case PROMOTION_KNIGHT: return 'N';
    case PROMOTION_NONE: 
    default:
          return '\0';
    }
}

uint8_t CharToPromotion(char promotion)
{
    switch (promotion) {
    case 'Q': return PROMOTION_QUEEN;
    case 'R': return PROMOTION_ROOK;
    case 'B': return PROMOTION_BISHOP;
    case 'N': return PROMOTION_KNIGHT;
    default:
          return PROMOTION_NONE;
    }
}


// _Bool MoveMake(Board* board, Move move)
// {
//     Square from, to;
//     uint8_t promotion, flag;
//     MoveDecode(move, &from, &to, &promotion, &flag);
//
//     Piece from_before = PieceAt(board, from);
//     int color = from_before.color;
//
//     size_t piece_count_before = NumberOfPieces(board, PIECE_COLOR_NONE); // Count all pieces before the move
//
//     uint8_t castling_rights_to_revoke = UpdateCastlingRights(board, from);
//     Square enpassant_square = UpdateEnpassantSquare(board, move);
//
//     // Execute the move
//     if(king_is_castling(board, from, to)){ 
//         if(!king_can_castle(board, from, to)){
//             board->state.error = ERROR_INVALID_MOVE;
//             return 0;
//         }
//         king_castle(board, from, to);
//     } else if(pawn_is_enpassanting(board, from, to)) {
//         if(!pawn_can_enpassant(board, from, to)){
//             board->state.error = ERROR_INVALID_MOVE;
//             return 0;
//         }
//         pawn_enpassant(board, from, to);
//     } else if(pawn_is_promoting(board, from, to)) {
//         if(!pawn_promote(board, from, to, promotion)) {
//             board->state.error = ERROR_INVALID_MOVE;
//             return 0;
//         }
//     } else {
//         MoveFreely(board, from, to);
//     }
//
//     revoke_castling_rights(&board->state, castling_rights_to_revoke);
//     board->enpassant_square = enpassant_square;
//
//     size_t piece_count_after = NumberOfPieces(board, PIECE_COLOR_NONE);
//
//     if(board->state.turn == PIECE_COLOR_BLACK) board->state.fullmove++;
//
//     UpdateHalfmove(board, move, piece_count_before, piece_count_after, from_before.type);
//
//     board->state.turn = !board->state.turn;
//
//     // Check for the posibility of a result
//     if(board->state.halfmove >= 50) board->state.result = RESULT_DRAW_DUE_TO_50_MOVE_RULE;
//     if(IsCheckmate(board)) board->state.result = (color == PIECE_COLOR_WHITE)
//                                             ? RESULT_WHITE_WON
//                                             : RESULT_BLACK_WON;
//     if(IsStalemate(board)) board->state.result = RESULT_STALEMATE;
//     if(IsInsufficientMaterial(board)) board->state.result = RESULT_DRAW_DUE_TO_INSUFFICIENT_MATERIAL;
//     if(IsThreefoldRepetition(board)) board->state.result = RESULT_DRAW_BY_REPETITION;
//
//     return 1;
// }
