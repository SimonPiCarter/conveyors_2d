extends Node2D
@onready var overlay = $overlay

@export var world_size = 24

var offset = Vector2(0,0)

func update_overlay(mode : BuildMode.Mode, horizontal, negative, flipped):
	var flipped_str = "flipped_" if flipped else ""
	#reset offset
	offset = Vector2(0,0)
	if mode == BuildMode.Mode.LINE:
		if horizontal:
			if negative:
				overlay.sprite_frames = preload("res://godot/frames/belt/belt_left_v2.tres")
			else:
				overlay.sprite_frames = preload("res://godot/frames/belt/belt_right_v2.tres")
		else:
			if negative:
				overlay.sprite_frames = preload("res://godot/frames/belt/belt_up_v2.tres")
			else:
				overlay.sprite_frames = preload("res://godot/frames/belt/belt_down_v2.tres")
	elif mode == BuildMode.Mode.SPLITTER:
		if horizontal:
			if negative:
				if flipped:
					offset = Vector2(-24,0)
				else:
					offset = Vector2(-24,-24)
				overlay.sprite_frames = load("res://godot/frames/belt/splitter_left_"+flipped_str+"v2.tres")
			else:
				if not flipped:
					offset = Vector2(0,-24)
				overlay.sprite_frames = load("res://godot/frames/belt/splitter_right_"+flipped_str+"v2.tres")
		else:
			if negative:
				if flipped:
					offset = Vector2(0,-24)
				else:
					offset = Vector2(-24,-24)
				overlay.sprite_frames = load("res://godot/frames/belt/splitter_up_"+flipped_str+"v2.tres")
			else:
				if not flipped:
					offset = Vector2(-24,0)
				overlay.sprite_frames = load("res://godot/frames/belt/splitter_down_"+flipped_str+"v2.tres")
	elif mode == BuildMode.Mode.MERGER:
		if horizontal:
			if negative:
				if flipped:
					offset = Vector2(-24,0)
				else:
					offset = Vector2(-24,-24)
				overlay.sprite_frames = load("res://godot/frames/belt/merger_left_"+flipped_str+"v2.tres")
			else:
				if not flipped:
					offset = Vector2(0,-24)
				overlay.sprite_frames = load("res://godot/frames/belt/merger_right_"+flipped_str+"v2.tres")
		else:
			if negative:
				if flipped:
					offset = Vector2(0,-24)
				else:
					offset = Vector2(-24,-24)
				overlay.sprite_frames = load("res://godot/frames/belt/merger_up_"+flipped_str+"v2.tres")
			else:
				if not flipped:
					offset = Vector2(-24,0)
				overlay.sprite_frames = load("res://godot/frames/belt/merger_down_"+flipped_str+"v2.tres")

func set_overlay_enable(enabled):
	overlay.visible = enabled

func _input(event):
	if event is InputEventMouseMotion:
		var x = int(event.global_position.x/world_size)
		var y = int(event.global_position.y/world_size)
		overlay.position.x = x * world_size + offset.x
		overlay.position.y = y * world_size + offset.y
