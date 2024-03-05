class_name ScoreStage extends Control

@onready var label = $Panel/VBoxContainer/Label
@onready var button = $Panel/VBoxContainer/Button

signal end_score_stage

func _ready():
	button.pressed.connect(emit_signal.bind("end_score_stage"))

func set_score(score):
	label.text = "Score "+String.num(score)
