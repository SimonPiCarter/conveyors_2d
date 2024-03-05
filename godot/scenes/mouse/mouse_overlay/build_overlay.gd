extends Node2D
@onready var overlay = $overlay

@export var world_size = 24

var offset = Vector2(12,12)

var x = 0
var y = 0

func update_overlay(mode : BuildMode.Mode, horizontal, negative, flipped):
	overlay.sprite_frames = BeltFrameHelper.getBeltFrame(mode, horizontal, negative, flipped)

func set_overlay_enable(enabled):
	overlay.visible = enabled

func _input(event):
	if event is InputEventMouseMotion:
		x = int((event.global_position.x-offset.x)/world_size)
		y = int((event.global_position.y-offset.y)/world_size)
		overlay.position.x = x * world_size + offset.x
		overlay.position.y = y * world_size + offset.y
