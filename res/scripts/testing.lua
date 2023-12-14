
local global_forward = {x = 0, y = 0, z = -1}
local global_right = {x = 1, y = 0, z = 0}
local position = { x = 0, y = 0, z = 0 }
local direction = { x = 0, y = 0, z = 0 }

local pitch = 0
local yaw = 0

local sensitivity = 0.33
local camera_speed = 4
local capture_mouse = false



function update(delta)

  local rightX, rightY, rightZ = vec3_rotate(global_right.x, global_right.y, global_right.z, yaw, 0, 1, 0)
  local forwardX, forwardY, forwardZ = vec3_rotate(global_forward.x, global_forward.y, global_forward.z, yaw, 0, 1, 0)
  
  direction.x = 0
  direction.y = 0
  direction.z = 0

  if input_key_is_down_once(KEY_ESCAPE) then
    capture_mouse = not capture_mouse
  end
  
  input_mouse_set_lock(capture_mouse)
  if not capture_mouse then
    do return end
  end

  local dx, dy = input_mouse_get_delta()
  pitch = pitch - dy * sensitivity
  yaw = yaw - dx * sensitivity
  
  if input_key_is_down(KEY_W) then
    direction.x = direction.x + forwardX
    direction.y = direction.y + forwardY
    direction.z = direction.z + forwardZ
  end

  if input_key_is_down(KEY_S) then
    direction.x = direction.x - forwardX
    direction.y = direction.y - forwardY
    direction.z = direction.z - forwardZ
  end

  if input_key_is_down(KEY_D) then
    direction.x = direction.x + rightX
    direction.y = direction.y + rightY
    direction.z = direction.z + rightZ
  end

  if input_key_is_down(KEY_A) then
    direction.x = direction.x - rightX
    direction.y = direction.y - rightY
    direction.z = direction.z - rightZ
  end

  if input_key_is_down(KEY_SPACE) then
    direction.y = direction.y + 1
  end

  if input_key_is_down(KEY_LEFT_SHIFT) then
    direction.y = direction.y - 1
  end

  direction.x, direction.y, direction.z = vec3_normalize(direction.x, direction.y, direction.z)

  position.x = position.x + direction.x * camera_speed * delta
  position.y = position.y + direction.y * camera_speed * delta
  position.z = position.z + direction.z * camera_speed * delta
  vk_camera_update_params(position.x, position.y, position.z, pitch, yaw, 70, 0.1, 100.0)

end