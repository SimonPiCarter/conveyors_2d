class_name RecipeTemplate

var types: Array[int] = []
var quantities: Array[int] = []
var value: float = 0

func gen_name() -> String:
	var dict = {}
	for i in range(0, types.size()):
		dict[types[i]] = quantities[i]

	return JSON.stringify(dict)

func gen_description() -> String:
	return gen_name() + "\n\tvalue = " + String.num(value, 2)

func gen_visual_description() -> String:
	var text = "[center]"+String.num(value, 1)+"$\n"
	for i in range(0, min(types.size(), quantities.size())):
		text += String.num(quantities[i], 0)+"x[img]"+SpawnerManager.get_img(types[i])+"[/img]\n"
	text += "[/center]"
	return text
