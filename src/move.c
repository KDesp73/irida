#include "bitboard.h"
#include "board.h"
#include "move.h"
#include "masks.h"
#include "piece.h"
#include "square.h"
#include "zobrist.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

Undo MakeUndo(const Board* board, Move move)
{
    return (Undo) {
        .move = move,
        .castling = board->castling_rights,
        .enpassant = board->enpassant_square,
        .fiftyMove=  board->halfmove
    };
}

Moves BitboardToMoves(Bitboard bitboard, Square from)
{
    Moves moves = {0};
    while (bitboard) {
        Square to = poplsb(&bitboard);
        moves.list[moves.count++] = MoveEncode(from, to, PROMOTION_NONE, FLAG_NORMAL);
    }
    return moves;
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

    if(promotion == PROMOTION_NONE){
        board->grid[COORDS(to)] = board->grid[COORDS(from)];
    } else {
        char p = PromotionToChar(promotion);
        board->grid[COORDS(to)] = (color) ? p : tolower(p);
    }
    board->grid[COORDS(from)] = EMPTY_SQUARE;

    uint64_t from_bb = 1ULL << from;
    uint64_t to_bb = 1ULL << to;

    // Find the piece that moved
    for (int piece = 0; piece < 6; piece++) {
        if (board->bitboards[color * 6 + piece] & from_bb) {
            // Move the piece
            board->bitboards[color * 6 + piece] ^= from_bb; // Remove from source
            if (promotion) {
                // Add promoted piece
                board->bitboards[color * 6 + promotion] |= to_bb;
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

void MoveToString(Move move, char* buffer)
{
    Square from, to;
    uint8_t promotion, flag;
    MoveDecode(move, &from, &to, &promotion, &flag);
    char fromName[3], toName[3];
    SquareToName(fromName, from);
    SquareToName(toName, to);
    sprintf(buffer, "%s%s", fromName, toName); 
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

size_t getBitboardIndexFromSquare(Board* board, Square square)
{
    Piece piece = PieceAt(board, square);
    
    switch (piece.type) {
        case 'p': return 0;
        case 'n': return 1;
        case 'b': return 2;
        case 'r': return 3;
        case 'q': return 4;
        case 'k': return 5;
        case 'P': return 6;
        case 'N': return 7;
        case 'B': return 8;
        case 'R': return 9;
        case 'Q': return 10;
        case 'K': return 11;
        default:  return -1; // Invalid piece
    }
}

bool Castle(Board* board, Move move)
{
    MOVE_DECODE(move);

    if(flag != FLAG_CASTLING) return false;
    
}

bool Enpassant(Board* board, Move move);

void MakeMove(Board* board, Move move)
{
    Piece piece = PieceAt(board, GetFrom(move));
    if(piece.color != board->turn) return;
    if(piece.type == COLOR_NONE) return;

    MoveFreely(board, move, board->turn);

    board->turn = !board->turn;
}

void UnmakeMove(Board* board, Move move)
{
    Square from, to;
    uint8_t promotion, flag;
    MoveDecode(move, &from, &to, &promotion, &flag);

    Bitboard promotionBB = UndoMove(&board->bitboards[getBitboardIndexFromSquare(board, from)], move);
    int color = board->turn;

    switch (promotion) {
    case PROMOTION_QUEEN:
        board->bitboards[color*6 + INDEX_BLACK_QUEEN] &= ~promotionBB;
        break;
    case PROMOTION_ROOK:
        board->bitboards[color*6 + INDEX_BLACK_ROOK] &= ~promotionBB;
        break;
    case PROMOTION_BISHOP:
        board->bitboards[color*6 + INDEX_BLACK_BISHOP] &= ~promotionBB;
        break;
    case PROMOTION_KNIGHT:
        board->bitboards[color*6 + INDEX_BLACK_KNIGHT] &= ~promotionBB;
        break;
    }

    board->turn = !board->turn;
}

