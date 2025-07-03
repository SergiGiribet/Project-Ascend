class_name InputCamera
extends Node

static var direction: Vector3 = Vector3.ZERO
static var zoom_input: float = 0.0
static var h_rotation: float = 0.0

static func camera_movement_input(transform: Transform3D) -> Vector3:
	var input_2d = Input.get_vector("left", "right", "up", "down")
	var forward = transform.basis.z
	forward.y = 0
	forward = forward.normalized()

	var right = transform.basis.x
	right.y = 0
	right = right.normalized()

	return (right * input_2d.x + forward * input_2d.y).normalized()


static func camera_zoom_input() -> float:
	zoom_input = Input.get_axis("zoom_out", "zoom_in")
	return zoom_input

static func camera_rotation_input() -> float:
	h_rotation = Input.get_axis("left_rotation", "right_rotation")
	return h_rotation

static func is_movement_input() -> bool:
	return direction != Vector3.ZERO
