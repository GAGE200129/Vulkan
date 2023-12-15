

local transform_component = nil
local accumulated_time = 0

function init(go)
  transform_component = gameobject_get_component(go, "transform")
  
end

function update(delta)
  accumulated_time = accumulated_time + delta;
  transform_set_position(transform_component, math.sin(math.rad(accumulated_time * 30)) * 5, 
    0, math.cos(math.rad(accumulated_time * 30)) * 5)

end