print("Lua script loaded ok!")

local frame_count = 0

-- This function gets called by C every frame
function tick()
    frame_count = frame_count + 1
    
    -- Print status every 60 frames
    if frame_count % 60 == 0 then
        print("Lua script running... frame: " .. frame_count)
    end
end
