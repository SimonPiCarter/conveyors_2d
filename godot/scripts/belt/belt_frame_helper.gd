class_name BeltFrameHelper

static func getBeltFrame(mode : BuildMode.Mode, horizontal, negative, flipped):
	if mode == BuildMode.Mode.LINE:
		if horizontal:
			if negative:
				return preload("res://godot/frames/belt/v3/straight/belt_left_v3.tres")
			else:
				return preload("res://godot/frames/belt/v3/straight/belt_right_v3.tres")
		else:
			if negative:
				return preload("res://godot/frames/belt/v3/straight/belt_up_v3.tres")
			else:
				return preload("res://godot/frames/belt/v3/straight/belt_down_v3.tres")
	elif mode == BuildMode.Mode.SPLITTER:
		if horizontal:
			var flipped_str = "up" if flipped else "down"
			if negative:
				return load("res://godot/frames/belt/v3/splitter/splitter_left_"+flipped_str+".tres")
			else:
				return load("res://godot/frames/belt/v3/splitter/splitter_right_"+flipped_str+".tres")
		else:
			var flipped_str = "left" if flipped else "right"
			if negative:
				return load("res://godot/frames/belt/v3/splitter/splitter_up_"+flipped_str+".tres")
			else:
				return load("res://godot/frames/belt/v3/splitter/splitter_down_"+flipped_str+".tres")
	elif mode == BuildMode.Mode.MERGER:
		if horizontal:
			var flipped_str =  "up" if not flipped else "down"
			if negative:
				return load("res://godot/frames/belt/v3/merger/merger_left_"+flipped_str+".tres")
			else:
				return load("res://godot/frames/belt/v3/merger/merger_right_"+flipped_str+".tres")
		else:
			var flipped_str = "left" if not flipped else "right"
			if negative:
				return load("res://godot/frames/belt/v3/merger/merger_up_"+flipped_str+".tres")
			else:
				return load("res://godot/frames/belt/v3/merger/merger_down_"+flipped_str+".tres")
	return null
