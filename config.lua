-- config.lua
--
-- Configuration for the UCI interface and more
--
-- written by KDesp73


require("scripts.utils")

local config = {
    startpos = "",
    uciMode = true,
    debugMode = true,
    ponderMode = false,
    infiniteMode = false,
    depthLimit = 2,
    maxBookmoves = 10,

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
            type = UCI_SPIN,
            value = 20,
            params = {
                min = 0,
                max = 20,
            },
            default = "20"
        },
        {
            name = "SyzygyPath",
            type = UCI_STRING,
            value = "",
            default = ""
        },
        {
            name = "SyzygyProbeDepth",
            type = UCI_SPIN,
            value = 1,
            params = {
                min = 1,
                max = 100
            },
            default = "1"
        },
        {
            name = "Syzygy50MoveRule",
            type = UCI_CHECK,
            value = true,
            default = "true"
        },
        {
            name = "SyzygyProbeLimit",
            type = UCI_SPIN,
            value = 7,
            params = {
                min = 0,
                max = 7
            },
            default = "7"
        }
    }
}

-- PrintTable(config)

return config
