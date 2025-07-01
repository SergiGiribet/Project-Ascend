class_name EnemyBasic
extends CharacterBody3D

@export var speed: float = 2.0
@export var health: int = 100

func patrol(delta):
	var points = get_patrol_points()
	if points.is_empty(): return
	var target = points[0] # simplificado
	move_towards(target, speed, delta)

func attack_player():
	print("🗡️ Attacking!")

func flee_from_player():
	print("🏃 Fleeing!")
	velocity = -transform.basis.z * speed * 1.5
	move_and_slide()

func is_safe() -> bool:
	return true

func can_see_player() -> bool:
	return false

func get_patrol_points() -> Array:
	return [$PatrolPoint1.global_position, $PatrolPoint2.global_position]

func move_towards(target: Vector3, speed: float, delta: float):
	var dir = (target - global_position).normalized()
	velocity = dir * speed
	move_and_slide()
