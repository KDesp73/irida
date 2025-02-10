-- tuning.lua
--
-- Fine-Tuning handler for the evaluation and search function
--
-- written by KDesp73

return {
    pieces = {
        pawn = 1,
        knight = 3,
        bishop = 3.3,
        rook = 5,
        queen = 9,
        king = inf
    },
    thresholds = {
        middlegame = {
            moves = 20,
            pieces = 20
        },
        endgame = {
            moves = 60,
            pieces = 12
        }
    }
}
