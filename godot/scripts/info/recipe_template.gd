class_name RecipeTemplate

var types: Array[int] = []
var quantities: Array[int] = []
var value: float = 0

func get_name() -> String:
	var dict = {}
	for i in range(0, types.size()):
		dict[types[i]] = quantities[i]

	return JSON.stringify(dict)
