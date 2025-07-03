extends StateNode

@export var character : EnemyBasic
@export var animation_tree : AnimationTree

@export_category("Physics Friction")
@export var slow_down_speed : int = 1700

func _on_process(_delta : float) -> void:
	pass


func _on_physics_process(_delta : float) -> void:
	pass


func _on_unhandled_input(_event: InputEvent) -> void:
	pass


func _on_next_transitions() -> void:
	pass


func _on_enter() -> void:
	EnemyBasicAnimationTreeTransitions.play_idle_animation(animation_tree)


func _on_exit() -> void:
	pass
