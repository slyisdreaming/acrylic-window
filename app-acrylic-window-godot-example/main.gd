extends Control

# set Project > Project Settings > Display > Window:
#	Transparent = true
# can also set these settings to scale ui:
# 	Stretch Mode = canvas_items
# 	Aspect = expand

enum Frame {
	Auto,
	Custom,
	Default
}

enum Backdrop {
	Auto,
	None,
	Mica,
	Acryllic,
	Tabbed
}

@export var color: Color = Color("#ea121235") # dcdcdcd9
@export var frame: Frame = Frame.Auto
@export var backdrop: Backdrop = Backdrop.Acryllic

@onready var background: ColorRect = $Background
@onready var color_button: ColorPickerButton = $SettingsGrid/ColorButton
@onready var frame_button: CheckButton = $SettingsGrid/FrameButton
@onready var backdrop_button: OptionButton = $SettingsGrid/BackdropButton


func _ready() -> void:
	color_button.color = color
	frame_button.button_pressed = frame == Frame.Custom
	
	for key in Backdrop.keys():
		backdrop_button.add_item(key)
	backdrop_button.select(backdrop)
	
	get_window().set_transparent_background(true)
	#get_window().size = Vector2i(1280, 720)
	apply_acrylic_style()
	
	
func apply_acrylic_style() -> void:
	background.color = color
	RenderingServer.set_default_clear_color(color)
	
	var tool_path: = "../app-acrylic-window/out/build/x64-release/AcrylicWindow.exe" \
		if OS.has_feature("editor") \
		else "./AcrylicWindow.exe"
	var window_title: = get_window().title + (" (DEBUG)" if OS.has_feature("debug") else "")
	var luminance: = color.get_luminance()
	var border_tint: = Color(luminance, luminance, luminance) * 0.75
	var border_color: = color.lerp(border_tint, 1 - color.a)
	var caption_color := border_color
	var text_color: = Color.WHITE if color.get_luminance() < 0.65 else Color.BLACK
	var args: PackedStringArray = [
		"--title", window_title,
		"--frame", Frame.keys()[frame],
		"--backdrop", Backdrop.keys()[backdrop],
		"--border-color", border_color.to_html(),
		"--caption-color", caption_color.to_html(),
		"--text-color", text_color.to_html()
	]
	
	var is_debug: = false
	if is_debug:
		args.push_back("--verbose")
		args.push_back("--wait")
		
	OS.create_process(tool_path, args, is_debug)


func _on_color_button_color_changed(color: Color) -> void:
	self.color = color
	apply_acrylic_style()


func _on_frame_button_toggled(toggled_on: bool) -> void:
	frame = Frame.Custom if toggled_on else Frame.Default
	apply_acrylic_style()


func _on_backdrop_button_item_selected(index: int) -> void:
	backdrop = index
	apply_acrylic_style()
