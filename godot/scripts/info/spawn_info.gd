class_name SpawnInfo

var x: int = 0
var y: int = 0
var types: Array[int] = []
var spawn_time: int = 0

func gen_visual_description():
	var text = "[center]"+String.num(spawn_time/10., 1)+"s\n"
	for type in types:
		text += "1x[img]"+SpawnerManager.get_img(type)+"[/img]\n"
	text += "[/center]"
	return text
