class_name EnemyBasicAnimationTreeTransitions
extends Node

static func play_idle_animation(animation_tree : AnimationTree) -> void:
	animation_tree["parameters/Transition/transition_request"] = "state_idle"

static func play_walk_animation(animation_tree : AnimationTree) -> void:
	animation_tree["parameters/Transition/transition_request"] = "state_walk"
