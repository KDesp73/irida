
def translate_move(fen, uci_move):
    # Mapping for piece characters in FEN
    piece_map = {
        'p': 'Pawn', 'n': 'Knight', 'b': 'Bishop', 
        'r': 'Rook', 'q': 'Queen', 'k': 'King'
    }
    
    # UCI moves are always [from_col][from_row][to_col][to_row]
    from_sq = uci_move[:2]
    to_sq = uci_move[2:4]
    
    # Simple FEN parser to find the piece at from_sq
    rows = fen.split(' ')[0].split('/')
    col_map = {'a':0, 'b':1, 'c':2, 'd':3, 'e':4, 'f':5, 'g':6, 'h':7}
    
    from_col = col_map[from_sq[0]]
    from_row = 8 - int(from_sq[1])
    
    # Expand the FEN row (handle numbers like '8' or '3p4')
    expanded_row = ""
    for char in rows[from_row]:
        if char.isdigit():
            expanded_row += " " * int(char)
        else:
            expanded_row += char
            
    piece_char = expanded_row[from_col].lower()
    piece_name = piece_map.get(piece_char, "Piece")
    
    return f"{piece_name} from {from_sq} to {to_sq}"
