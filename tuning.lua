-- tuning.lua
--
-- Fine-Tuning handler for the evaluation and search function
--
-- written by KDesp73

return {
    pieces = {
        pawn = 100,
        knight = 300,
        bishop = 330,
        rook = 500,
        queen = 900,
        king = inf
    },
    thresholds = {
        middlegame = {
            moves = 15,
            pieces = 20
        },
        endgame = {
            moves = 70,
            pieces = 12
        }
    }
}
