class_name CameraController

extends Camera3D

@onready var camera: Camera3D = self


@export var move_speed := 10.0
@export var zoom_speed := 150.0
@export var min_fov := 30.0 
@export var max_fov := 90.0
@export var rotation_speed := 60.0

var zoom_delta := 0.0


func _unhandled_input(event):
	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_WHEEL_UP:
			zoom_delta = -1.0
		elif event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
			zoom_delta = 1.0

func _physics_process(delta):
	var direction = InputCamera.camera_movement_input(global_transform)
	if direction != Vector3.ZERO:
		global_position += direction * move_speed * delta
		

	if zoom_delta != 0:
		camera.fov = clamp(camera.fov + zoom_delta * zoom_speed * delta, min_fov, max_fov)
		zoom_delta = 0.0


	# Rotació horitzontal
	var rot_input = InputCamera.camera_rotation_input()
	if rot_input != 0:
		rotate_y(deg_to_rad(rot_input * rotation_speed * delta))
