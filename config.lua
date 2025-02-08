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
        -- {
        --     name = "Skill Level",
        --     type = UCI_COMBO,
        --     value = "Intermediate",
        --     params = { "Beginner", "Intermediate", "Advanced", "Master" },
        --     default = "Intermediate"
        -- }
    }
}

-- PrintTable(config)

return config
