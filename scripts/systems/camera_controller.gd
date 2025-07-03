class_name CameraController

extends Node3D

@onready var camera = Camera3D

@export var move_speed := 10.0
@export var zoom_speed := 20.0
@export var min_zoom := 5.0
@export var max_zoom := 30.0

func _physics_process(delta):
	var direction = InputCamera.camera_movement_input(global_transform)
	if direction != Vector3.ZERO:
		global_position += direction * move_speed * delta

	var zoom_delta = InputCamera.camera_zoom_input()
	if zoom_delta != 0:
		var new_y = clamp(camera.translation.y + zoom_delta * zoom_speed * delta, min_zoom, max_zoom)
		camera.translation.y = new_y
