extends Control

@onready var grid_container = $ScrollContainer/GridContainer

@onready var shop_items = [
	$ScrollContainer/GridContainer/shop_item,
	$ScrollContainer/GridContainer/shop_item2,
	$ScrollContainer/GridContainer/shop_item3,
	$ScrollContainer/GridContainer/shop_item4,
	$ScrollContainer/GridContainer/shop_item5,
	$ScrollContainer/GridContainer/shop_item6
]

signal selected_item(bonus)

func _ready():
	for item in shop_items:
		item.selected.connect(item_selected)

func gen_shop_items(run_info : RunInfo):
	for i in range(0,6):
		var type = randi_range(0, 2)
		var item = null
		match type:
			0:
				item = NewRecipeBonus.gen_bonus(run_info)
			1:
				item = ImproveRecipeBonus.gen_bonus(run_info)
			_:
				item = NewLineBonus.gen_bonus(run_info)
		shop_items[i].load_from_bonus(item)

func item_selected(bonus):
	selected_item.emit(bonus)
