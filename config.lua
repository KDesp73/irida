require("scripts.utils")

local config = {
    startpos = "Hello World",
    uciMode = true,
    debugMode = true,
    ponderMode = false,
    infiniteMode = false,
    depthLimit = 4,

    options = {
        {
            name = "Hash",
            type = UCI_SPIN,
            value = 64,
            params = {
                min = 1,
                max = 2048
            },
            default = "64"
        },
        {
            name = "Ponder",
            type = UCI_CHECK,
            value = false,
            default = "false"
        },
        {
            name = "Skill Level",
            type = UCI_COMBO,
            value = "Intermediate",
            params = { "Beginner", "Intermediate", "Advanced", "Master" },
            default = "Intermediate"
        },
        {
            name = "Engine Name",
            type = UCI_STRING,
            value = "MyChessEngine",
            default = "MyChessEngine"
        }
    }
}

-- PrintTable(config)

return config
