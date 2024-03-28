
local global_forward = {x = 0, y = 0, z = -1}
local global_right = {x = 1, y = 0, z = 0}
local position = { x = 0, y = 0, z = 0 }
local delayed_position = { x = 0, y = 0, z = 0 }
local direction = { x = 0, y = 0, z = 0 }
local right_dir = { x = 0, y = 0, z = 0 }
local forward_dir = { x = 0, y = 0, z = 0 }
local cam_forward = { x = 0, y = 0, z = 0 }

local pitch = 0
local yaw = 0
local pitch_delayed = 0
local yaw_delayed = 0

local sensitivity = 0.33
local camera_speed = 4
local capture_mouse = false
local character_controller = nil
local transform = nil

function lerp(a,b,t)
  return a+(b-a)*t
end

function init(go)
  character_controller = gameobject_get_component(go, "character_controller")
  transform = gameobject_get_component(go, "transform")
end

function update(delta)

  right_dir.x, right_dir.y, right_dir.z = vec3_rotate(global_right.x, global_right.y, global_right.z, yaw, 0, 1, 0)
  forward_dir.x, forward_dir.y, forward_dir.z = vec3_rotate(global_forward.x, global_forward.y, global_forward.z, yaw, 0, 1, 0)
 
  if input_key_is_down_once(KEY_ESCAPE) then
    capture_mouse = not capture_mouse
  end
  
  direction.x, direction.y, direction.z = vec3_normalize(direction.x, direction.y, direction.z)
  character_controller_set_move_dir(character_controller, direction.x, direction.y, direction.z)

  position.x, position.y, position.z = transform_get_position(transform)
  delayed_position.x = lerp(delayed_position.x, position.x, delta * 10.0)
  delayed_position.y = lerp(delayed_position.y, position.y, delta * 10.0)
  delayed_position.z = lerp(delayed_position.z, position.z, delta * 10.0)

  cam_forward.x, cam_forward.y, cam_forward.z = vec3_rotate(global_forward.x, global_forward.y, global_forward.z, pitch, 1, 0, 0)
  cam_forward.x, cam_forward.y, cam_forward.z = vec3_rotate(cam_forward.x, cam_forward.y, cam_forward.z, yaw, 0, 1, 0)

  position.x = delayed_position.x - cam_forward.x * 4
  position.y = delayed_position.y - cam_forward.y * 4 + 2
  position.z = delayed_position.z - cam_forward.z * 4

  pitch_delayed = lerp(pitch_delayed, pitch, delta * 40)
  yaw_delayed = lerp(yaw_delayed, yaw, delta * 40)

  vk_camera_update_params(position.x, position.y, position.z, pitch_delayed, yaw_delayed, 70, 0.1, 100.0)

  input_mouse_set_lock(capture_mouse)
  if not capture_mouse then
    do return end
  end

  local dx, dy = input_mouse_get_delta()
  pitch = pitch - dy * sensitivity
  yaw = yaw - dx * sensitivity

  direction.x = 0
  direction.y = 0
  direction.z = 0

  
  if input_key_is_down(KEY_W) then
    direction.x = direction.x + forward_dir.x
    direction.y = direction.y + forward_dir.y
    direction.z = direction.z + forward_dir.z
  end

  if input_key_is_down(KEY_S) then
    direction.x = direction.x - forward_dir.x
    direction.y = direction.y - forward_dir.y
    direction.z = direction.z - forward_dir.z
  end

  if input_key_is_down(KEY_D) then
    direction.x = direction.x + right_dir.x
    direction.y = direction.y + right_dir.y
    direction.z = direction.z + right_dir.z
  end

  if input_key_is_down(KEY_A) then
    direction.x = direction.x - right_dir.x
    direction.y = direction.y - right_dir.y
    direction.z = direction.z - right_dir.z
  end

  if input_key_is_down(KEY_SPACE) then
    direction.y = direction.y + 1
  end

  if input_key_is_down(KEY_LEFT_SHIFT) then
    direction.y = direction.y - 1
  end

 

end