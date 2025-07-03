class_name InputOrders
extends Node

func handle_tap(pos):
	# Aquí pots fer raycast per veure si s'ha tocat una unitat
	print("👆 Toc a la posició: ", pos)

func handle_drag_select(start_pos, end_pos):
	# Aquí pots seleccionar unitats dins del rectangle
	print("📦 Selecció de caixa de ", start_pos, " a ", end_pos)
