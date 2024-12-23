#include "bitboard.h"
#include "board.h"
#include "move.h"
#include "hashing.h"
#include "history.h"
#include "masks.h"
#include "piece.h"
#include "square.h"
#include "zobrist.h"

#include <ctype.h>
#include <io/logging.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

bool MoveCmpStrict(Move m1, Move m2)
{
    return MoveCmp(m1, m2) && GetFlag(m1) == GetFlag(m2);
}


bool MoveCmp(Move m1, Move m2)
{
    return (
        GetFrom(m1) == GetFrom(m2) &&
        GetTo(m1) == GetTo(m2) &&
        GetPromotion(m1) == GetPromotion(m2));
}

void MoveSetFlag(Move* move, Flag flag)
{
    *move &= ~(0x7 << 16);
    *move |= ((flag & 0x7) << 16);
}
void MoveSetPromotion(Move* move, Promotion promotion)
{
    *move &= ~(0xF << 12);
    *move |= ((promotion & 0xF) << 12); 
}

void BoardPrintMove(const Board* board, Move move)
{
    MOVE_DECODE(move);
    BoardPrint(board, src, dst, 64);
}

Undo MakeUndo(const Board* board, Move move)
{
    char capture = PieceAt(board, GetTo(move)).type;
    return (Undo) {
        .move = move,
        .castling = board->castling_rights,
        .enpassant = board->enpassant_square,
        .fiftyMove=  board->halfmove,
        .captured = capture
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

Bitboard MovesToBitboard(Moves moves)
{
    Bitboard bb = 0ULL;

    for(size_t i = 0; i < moves.count; i++){
        bb |= BB(GetTo(moves.list[i]));
    }

    return bb;
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
    return move & 0x3F;
}
Square GetTo(Move move)
{
    return (move >> 6) & 0x3F;
}
uint8_t GetPromotion(Move move)
{
    return (move >> 12) & 0xF;
}
uint8_t GetFlag(Move move)
{
    return (move >> 16) & 0x7;
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
}

void MoveToString(Move move, char* buffer)
{
    Square from, to;
    uint8_t promotion, flag;
    MoveDecode(move, &from, &to, &promotion, &flag);

    char fromName[3], toName[3];
    SquareToName(fromName, from);
    SquareToName(toName, to);

    char p[2] = "";
    switch (promotion) {
        case PROMOTION_QUEEN:
            strcpy(p, "q"); break;
        case PROMOTION_ROOK:
            strcpy(p, "r"); break;
        case PROMOTION_BISHOP:
            strcpy(p, "b"); break;
        case PROMOTION_KNIGHT:
            strcpy(p, "n"); break;
        case PROMOTION_NONE:
            p[0] = '\0'; break; // Ensure it's an empty string
    }

    sprintf(buffer, "%s%s%s", fromName, toName, p); 
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

size_t getBitboardIndexFromPiece(char piece)
{
    switch (piece) {
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

size_t getBitboardIndexFromSquare(Board* board, Square square)
{
    Piece piece = PieceAt(board, square);
    return getBitboardIndexFromPiece(piece.type);
}

bool IsCastle(const Board* board, Move* move)
{
    Square from, to;
    uint8_t promotion, flag;
    MoveDecode(*move, &from, &to, &promotion, &flag);

    Piece piece = PieceAt(board, from);
    if (tolower(piece.type) != 'k' || piece.color != board->turn) {
        return false;
    }

    // Castling is a king move of exactly two squares horizontally
    int rowFrom = from / 8;
    int colFrom = from % 8;
    int rowTo = to / 8;
    int colTo = to % 8;

    if (rowFrom != rowTo) { // King must stay in the same rank
        return false;
    }

    int colDiff = colTo - colFrom;
    if (abs(colDiff) != 2) {
        return false;
    }

    MoveSetFlag(move, FLAG_CASTLING);
    return true;
}

#define CASTLE_ROOK(dst_k, dst_q)\
    if (dst == dst_k) { \
        board->grid[COORDS(dst-1)] = dst_k < 8 ? 'R' : 'r'; \
        board->grid[COORDS(dst+1)] = EMPTY_SQUARE; \
        board->bitboards[color * 6 + INDEX_BLACK_ROOK] ^= (1ULL << (dst+1)) | (1ULL << (dst-1)); \
    } else if (dst == dst_q) { \
        board->grid[COORDS(dst+1)] = dst_q < 8 ? 'R' : 'r'; \
        board->grid[COORDS(dst-2)] = EMPTY_SQUARE; \
        board->bitboards[color * 6 + INDEX_BLACK_ROOK] ^= (1ULL << (dst-2)) | (1ULL << (dst+1)); \
    }


bool Castle(Board* board, Move move)
{
    MOVE_DECODE(move);
    Color color = board->turn;

    if(flag != FLAG_CASTLING) return false;
    Piece piece = PieceAt(board, src);

    int colFrom = src % 8;
    int colTo = dst % 8;

    int colDiff = colTo - colFrom;

    if (colDiff == 2) { // Short castling (kingside)
        if (
            !(HasCastlingRights(board, CASTLE_WHITE_KINGSIDE) && piece.color)
            &&
            !(HasCastlingRights(board, CASTLE_BLACK_KINGSIDE) && !piece.color)
        ) return false;
        if(
            IsSquareAttacked(board, (color) ? 6 : 62, !color)
            ||
            IsSquareAttacked(board, (color) ? 5 : 61, !color)
        ) return false;
    } else if (colDiff == -2) { // Long castling (queenside)
        if (
            !(HasCastlingRights(board, CASTLE_WHITE_QUEENSIDE) && piece.color)
            &&
            !(HasCastlingRights(board, CASTLE_BLACK_QUEENSIDE) && !piece.color)
        ) return false;
        if(
            IsSquareAttacked(board, (color) ? 3 : 59, !color)
            ||
            IsSquareAttacked(board, (color) ? 2 : 58, !color)
        ) return false;
    }


    CASTLE_ROOK(6, 2);
    CASTLE_ROOK(62, 58)

    return true;
}

bool IsEnpassant(const Board* board, Move* move)
{
    Square from, to;
    uint8_t promotion, flag;

    // Decode the move into its components
    MoveDecode(*move, &from, &to, &promotion, &flag);

    // Ensure the piece moving is a pawn
    int color = board->turn;
    Bitboard pawnBB = board->bitboards[color * 6 + INDEX_BLACK_PAWN];
    if (!(pawnBB & (1ULL << from))) {
        return false; // Not a pawn move
    }

    // Ensure it's a diagonal move (en passant is always diagonal)
    int file_diff = abs((to % 8) - (from % 8));
    int rank_diff = abs((to / 8) - (from / 8));
    if (file_diff != 1 || rank_diff != 1) {
        return false; // Not a diagonal move
    }

    // Check if the destination square matches the en passant square
    if (to != board->enpassant_square) {
        return false; // Not en passant
    }

    MoveSetFlag(move, FLAG_ENPASSANT);
    return true;
}
bool Enpassant(Board* board, Move move)
{
    MOVE_DECODE(move);
    Color color = board->turn;

    if (flag != FLAG_ENPASSANT) return false;

    if (dst != board->enpassant_square) return false;

    Square captured_square = color == COLOR_WHITE ? dst - 8 : dst + 8;

    // Remove the captured pawn
    board->bitboards[!color * 6 + INDEX_BLACK_PAWN] &= ~(1ULL << captured_square);
    board->grid[COORDS(captured_square)] = ' ';

    // Update the halfmove clock
    board->halfmove = 0;

    return true;
}
bool IsPromotion(Board* board, Move* move)
{
    Square from = GetFrom(*move);
    Square to = GetTo(*move);
    Color color = PieceAt(board, from).color;

    if(color && (Rank(from) != 6 || Rank(to) != 7)) return false;
    if(!color && (Rank(from) != 1 || Rank(to) != 0)) return false;

    // if(File(from) != File(to))
    //     MoveSetFlag(move, FLAG_PROMOTION_WITH_CAPTURE);
    // else 
        MoveSetFlag(move, FLAG_PROMOTION);
    return true;
}

bool IsDoublePawnPush(Board* board, Move move)
{
    Square from = GetFrom(move);
    Piece piece = PieceAt(board, from);
    if(!IS_PAWN(piece)) return false;

    Bitboard doublePushes = PawnDoublePushMask(from, piece.color);

    return doublePushes & BB(GetTo(move));
}

bool MakeMove(Board* board, Move move)
{
    Piece piece = PieceAt(board, GetFrom(move));
    if(piece.color != board->turn) return false;
    if(piece.type == COLOR_NONE) return false;

    uint8_t castling = UpdateCastlingRights(board, move);
    Square enpassant;
    if(IsDoublePawnPush(board, move)){
        enpassant = GetFrom(move) + ((piece.color) ? 8 : -8);
    } else {
        enpassant = 64;
    }

    bool succ = true;
    if(IsCastle(board, &move)){
        succ = Castle(board, move);
    } else if(IsEnpassant(board, &move)){
        succ = Enpassant(board, move);
    } 

    if(!succ) return false;


    IsPromotion(board, &move);

    HistoryAddUndo(&board->history, board, move);

    MoveFreely(board, move, board->turn);

    board->castling_rights = castling;
    board->enpassant_square = enpassant;

    if (board->turn == COLOR_BLACK) {
        board->fullmove++;
    }


    UpdateHashTable(&board->history.positions, CalculateZobristHash(board));
    board->turn = !board->turn;

    return true;
}

void UnmakeMove(Board* board)
{
    Undo undo = LoadLastUndo(board);

    MOVE_DECODE(undo.move);

    Bitboard specialBB = UndoMove(&board->bitboards[getBitboardIndexFromSquare(board, dst)], undo.move);
    board->grid[COORDS(src)] = board->grid[COORDS(dst)];
    board->grid[COORDS(dst)] = EMPTY_SQUARE;
    
    int color = !board->turn;

    if(flag == FLAG_PROMOTION){
        switch (promotion) {
        case PROMOTION_QUEEN:
            board->bitboards[color*6 + INDEX_BLACK_QUEEN] &= ~specialBB;
            break;
        case PROMOTION_ROOK:
            board->bitboards[color*6 + INDEX_BLACK_ROOK] &= ~specialBB;
            break;
        case PROMOTION_BISHOP:
            board->bitboards[color*6 + INDEX_BLACK_BISHOP] &= ~specialBB;
            break;
        case PROMOTION_KNIGHT:
            board->bitboards[color*6 + INDEX_BLACK_KNIGHT] &= ~specialBB;
            break;
        }
        board->bitboards[color*6 + INDEX_BLACK_PAWN] |= 1ULL << src;
        board->grid[COORDS(src)] = (color) ? 'P' : 'p';
    } else if(flag == FLAG_CASTLING){
        if(File(dst) > File(src)){
            board->grid[COORDS(dst+1)] = (color) ? 'R' : 'r';
            board->grid[COORDS(dst-1)] = EMPTY_SQUARE;
        } else {
            board->grid[COORDS(dst+1)] = (color) ? 'R' : 'r';
            board->grid[COORDS(dst-2)] = EMPTY_SQUARE;
        }

        board->bitboards[color*6 + INDEX_BLACK_ROOK] ^= specialBB;
    } else if(flag == FLAG_ENPASSANT) {
        board->bitboards[!color*6 + INDEX_BLACK_PAWN] |= specialBB;
        board->grid[COORDS(dst + ((color) ? -8 : 8))] = (color) ? 'p' : 'P';
    } 

    if (undo.captured != EMPTY_SQUARE) {
        int capturedPieceIndex = getBitboardIndexFromPiece(undo.captured);

        board->bitboards[capturedPieceIndex] |= (1ULL << dst);
        board->grid[COORDS(dst)] = undo.captured;
    }

    if(board->turn == COLOR_WHITE){
        board->fullmove--;
    }

    HistoryRemove(&board->history);

    board->turn = !board->turn;
}

