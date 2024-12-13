#include "board.h"
#include "move.h"
#include "piece.h"
#include "square.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <chess/piece.h>

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

_Bool PieceCanMove(const Board* board, Square from, Square to, Flag* flag)
{
    Piece piece = PieceAt(board, from);

    if (piece.type == ' ') {
        return 0;
    }
    Color color = PieceAt(board, from).color;

    *flag = FLAG_NORMAL;
    switch (tolower(piece.type)) {
        case 'p':
            return CanMovePawn(board, from, to, color, flag);
        case 'n':
            return CanMoveKnight(board, from, to, color);
        case 'b':
            return CanMoveBishop(board, from, to, color);
        case 'r':
            return CanMoveRook(board, from, to, color);
        case 'q':
            return CanMoveQueen(board, from, to, color);
        case 'k':
            return CanMoveKing(board, from, to, color, flag);
        default:
            return 0; // Unsupported piece type
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

Bitboard GenerateKnightMoves(const Board* board, Square from, Color color)
{
    Bitboard moves = 0;
    int inverted = (7 - (from/ 8)) * 8 + (from% 8);
    int rank = inverted / 8;
    int file = inverted % 8; // Current file

    for (size_t i = 0; i < KNIGHT_OFFSETS_COUNT; i++) {
        int to = inverted + KNIGHT_OFFSETS[i];

        // Ensure the target square is within bounds
        if (to >= 0 && to < 64) {
            int to_rank = (to / 8); // Reversed rank for target square
            int to_file = to % 8; // Target file

            // Check that the move doesn't wrap around the board
            if (abs(to_rank - rank) == 2 && abs(to_file - file) == 1) {
                moves |= (1ULL << to); // Add valid move to bitboard
            } else if (abs(to_rank - rank) == 1 && abs(to_file - file) == 2) {
                moves |= (1ULL << to); // Add valid move to bitboard
            }
        }
        if (IsSquareOccupiedBy(board, to, !color)) break;
    }

    return moves;
}

Bitboard GenerateRookMoves(const Board* board, Square from, Color color)
{
    Bitboard moves = 0;
    int square = (7 - (from/8))*8 + (from%8);
    int start_rank = square / 8;
    int start_file = square % 8;

    // Loop through each possible direction
    for (size_t i = 0; i < ROOK_OFFSETS_COUNT; i++) {
        int rank = start_rank;
        int file = start_file;

        while (1) {
            // Apply the offset
            rank += ROOK_OFFSETS[i] / 8;
            file += ROOK_OFFSETS[i] % 8;

            if (rank < 0 || rank >= 8 || file < 0 || file >= 8) break;

            Square to = rank * 8 + file;

            moves |= (1ULL << to);

            if (IsSquareOccupiedBy(board, to, !color)) break;
        }
    }

    return moves;
}

Bitboard GenerateBishopMoves(const Board* board, Square from, Color color)
{
    Bitboard moves = 0;
    int inverted = (7 - (from / 8)) * 8 + (from % 8);
    int rank = inverted / 8; // Current rank
    int file = inverted % 8; // Current file

    // Directions for diagonal movement: top-left, top-right, bottom-left, bottom-right
    int directions[] = {-9, 9, -7, 7}; 

    for (size_t i = 0; i < 4; i++) {
        int direction = directions[i];
        int current_square = inverted;

        // Move in the current direction
        while (true) {
            current_square += direction;

            // Check if the move is still within bounds (0 to 63)
            if (current_square < 0 || current_square >= 64) break;

            int to_rank = current_square / 8;
            int to_file = current_square % 8;

            // Ensure the move doesn't go out of bounds on the board's edges
            if ((direction == -9 && to_file == 7) || 
                (direction == 9 && to_file == 0) || 
                (direction == -7 && to_file == 0) || 
                (direction == 7 && to_file == 7)) {
                break;
            }

            // Add valid move to bitboard
            moves |= (1ULL << current_square);

            // Stop if a piece of the same color is encountered (obstruction)
            if (IsSquareOccupiedBy(board, current_square, color)) {
                break; // Stop further movement in this direction
            }

            // Stop if a piece of the opposite color is encountered (capture opportunity)
            if (IsSquareOccupiedBy(board, current_square, !color)) {
                // Add the square as a valid capture move
                moves |= (1ULL << current_square);
                break; // Stop further movement after a capture
            }
        }
    }

    return moves;
}

Bitboard GenerateQueenMoves(const Board* board, Square from, Color color)
{
    return GenerateBishopMoves(board, from, color)
         | GenerateRookMoves(board, from, color); 
}

Bitboard GenerateKingMoves(const Board* board, Square from, Color color)
{
    Bitboard moves = 0;
    int inverted = (7 - (from/ 8)) * 8 + (from% 8);
    int rank = inverted / 8; // Current rank
    int file = inverted % 8; // Current file

    for (size_t i = 0; i < KING_OFFSETS_COUNT; i++) {
        int8_t to = inverted + KING_OFFSETS[i];

        // Ensure the target square is within bounds
        if (to >= 0 && to < 64) {
            int to_rank = to / 8;
            int to_file = to % 8;

            if (IsSquareOccupiedBy(board, to, color)) break;

            if (abs(to_rank - rank) <= 1 && abs(to_file - file) <= 1) {
                moves |= (1ULL << to); // Add valid move to bitboard
            }
        }
    }

    return moves;
}

Bitboard GeneratePawnMoves(const Board* board, Square from, Color color)
{
    Bitboard moves = 0;
    int rank = 7 - (from / 8); // Current rank
    int file = from % 8; // Current file
    int direction = (color) ? 1 : -1;

    // Move one square forward
    moves |= (1ULL << ((rank + direction) * 8 + file));

    // Move two squares forward if on the initial rank and no obstruction
    if (rank == ((color) ? 1 : 6)) {
        int twoSquareRank = rank + 2 * direction;
        int twoSquare = twoSquareRank * 8 + file;

        // Check if the square in front of the pawn is unoccupied
        if (!IsSquareOccupiedBy(board, twoSquare, PIECE_COLOR_WHITE) &&
            !IsSquareOccupiedBy(board, twoSquare, PIECE_COLOR_BLACK)) {
            moves |= (1ULL << twoSquare); // Add two-square move if no obstruction
        }
    }

    // Attacks (diagonal captures)
    Square attack1 = (rank + direction) * 8 + file + 1;
    Square attack2 = (rank + direction) * 8 + file - 1;

    // Add attack moves (captures or en passant)
    if (IsSquareOccupiedBy(board, attack1, !color) || SR(attack1) == board->enpassant_square)
        moves |= (1ULL << attack1);
    if (IsSquareOccupiedBy(board, attack2, !color) || SR(attack2) == board->enpassant_square)
        moves |= (1ULL << attack2);

    return moves;
}

Bitboard GenerateLegalMovesBitboard(const Board* board, Color color)
{
    Bitboard piecesBitboard = (color == PIECE_COLOR_WHITE) ? GetWhite(board) : GetBlack(board);
    Bitboard result = 0;

    for (size_t i = 0; i < 64; i++) {
        if (piecesBitboard & (1ULL << i)) {
            Square from = (Square)i;
            Piece piece = PieceAt(board, from);

            Bitboard possibleMoves = 0;

            switch (piece.type) {
                case 'P': // White Pawn
                case 'p': // Black Pawn
                    possibleMoves = GeneratePawnMoves(board, from, piece.color); // color already passed
                    break;
                case 'N': // White Knight
                case 'n': // Black Knight
                    possibleMoves = GenerateKnightMoves(board, from, piece.color); // color already passed
                    break;
                case 'B': // White Bishop
                case 'b': // Black Bishop
                    possibleMoves = GenerateBishopMoves(board, from, piece.color); // color already passed
                    break;
                case 'R': // White Rook
                case 'r': // Black Rook
                    possibleMoves = GenerateRookMoves(board, from, piece.color); // color already passed
                    break;
                case 'Q': // White Queen
                case 'q': // Black Queen
                    possibleMoves = GenerateQueenMoves(board, from, piece.color); // color already passed
                    break;
                case 'K': // White King
                case 'k': // Black King
                    possibleMoves = GenerateKingMoves(board, from, piece.color); // color already passed
                    break;
                default:
                    continue; // Ignore invalid pieces
            }

            // Ensure the move doesn't land on a friendly piece
            possibleMoves &= ~(color == PIECE_COLOR_WHITE ? GetWhite(board) : GetBlack(board));

            result |= possibleMoves;
        }
    }

    return result;
}

void GenerateLegalMoves(const Board* board, Color color, Move* moves, size_t* move_count)
{
    *move_count = 0;
    
    uint64_t piecesBitboard = (color == PIECE_COLOR_WHITE) ? GetWhite(board) : GetBlack(board);

    for (size_t i = 0; i < 64; i++) {
        if (piecesBitboard & (1ULL << i)) {
            Square from = (Square)i;
            Piece piece = PieceAt(board, from);

            Bitboard possibleMoves = 0;
            
            switch (piece.type) {
                case 'P': // White Pawn
                case 'p': // Black Pawn
                    possibleMoves = GeneratePawnMoves(board, from, piece.color);
                    break;
                case 'N': // White Knight
                case 'n': // Black Knight
                    possibleMoves = GenerateKnightMoves(board, from, piece.color);
                    break;
                case 'B': // White Bishop
                case 'b': // Black Bishop
                    possibleMoves = GenerateBishopMoves(board, from, piece.color);
                    break;
                case 'R': // White Rook
                case 'r': // Black Rook
                    possibleMoves = GenerateRookMoves(board, from, piece.color);
                    break;
                case 'Q': // White Queen
                case 'q': // Black Queen
                    possibleMoves = GenerateQueenMoves(board, from, piece.color);
                    break;
                case 'K': // White King
                case 'k': // Black King
                    possibleMoves = GenerateKingMoves(board, from, piece.color);
                    break;
                default:
                    continue; // Ignore invalid pieces
            }

            for (size_t j = 0; j < 64; j++) {
                if (possibleMoves & (1ULL << j)) {
                    Square to = (Square)j;

                    // Temporarily make the move
                    Board tempBoard = *board;
                    MoveFreely(&tempBoard, from, to);

                    // Check if the move puts the king in check
                    if (IsKingInCheck(tempBoard, color)) {
                        continue; // Skip illegal move
                    }

                    // Add legal move to the list
                    moves[*move_count] = MoveEncode(from, to, PROMOTION_NONE, FLAG_NORMAL);
                    (*move_count)++;
                }
            }
        }
    }
}
