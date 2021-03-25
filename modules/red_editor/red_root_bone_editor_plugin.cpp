#include "modules/red/root_bone_2d.h"
#include "red_root_bone_editor_plugin.h"
#include "editor/plugins/canvas_item_editor_plugin.h"

void RootBone2DEditor::_node_removed(Node *p_node) {
	if (p_node == node) {
		node = NULL;
		options->hide();
	}
}

void RootBone2DEditor::edit(Node *p_node) {
	if (!canvas_item_editor)
		canvas_item_editor = CanvasItemEditor::get_singleton();
	if (p_node) {
		_set_node(p_node);
		canvas_item_editor->update_viewport();
	} else {
		_set_node(NULL);
	}
}

RootBone2D *RootBone2DEditor::_get_node() const {
	return node;
}

void RootBone2DEditor::_set_node(Node *p_node) {
	node = (RootBone2D*)(p_node);
}

bool RootBone2DEditor::spatial_pos_is_near(const Vector2 &p_pos) const {
	const real_t grab_threshold = EDITOR_GET("editors/poly_editor/point_grab_radius");
	const Transform2D xform = canvas_item_editor->get_canvas_transform() * _get_node()->get_global_transform();
	Vector3 pos3d = _get_spatial_position();
	Vector2 pos2d = Vector2(pos3d.x, pos3d.y);
	Vector2 cp = xform.xform(pos2d);
	real_t d = cp.distance_to(p_pos);
	return d < grab_threshold;
}

bool RootBone2DEditor::forward_gui_input(const Ref<InputEvent> &p_event) {
	RootBone2D *node = _get_node();
	if (!node)
		return false;
	CanvasItemEditor::Tool tool = CanvasItemEditor::get_singleton()->get_current_tool();
	if (tool != CanvasItemEditor::TOOL_SELECT)
		return false;
	
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		Vector2 gpoint = mb->get_position();
		//Transform2D xform = canvas_item_editor->get_canvas_transform() * _get_node()->get_global_transform();
		Vector2 cpoint = node->get_global_transform().affine_inverse().xform(canvas_item_editor->snap_point(canvas_item_editor->get_canvas_transform().affine_inverse().xform(mb->get_position())));
		if (mb->get_button_index() == BUTTON_LEFT) {
			if (mb->is_pressed()) {
				//look for points to move
				if (spatial_pos_is_near(gpoint)) {
					edit_state = EditState::MOVING;
					edited_point = _get_spatial_position();
					pre_move_edit = edited_point;
					canvas_item_editor->update_viewport();
					return true;
				} else {
					edit_state = EditState::STOPED;
				}
			}else {
				//apply moved point
				if (edit_state == EditState::MOVING) {
					edit_state = EditState::STOPED;
					undo_redo->create_action(TTR("Edit spatial Position"));
					_action_set_spatial_position(pre_move_edit, _get_spatial_position());
					_commit_action();
					return true;
				}
			}
		}else if (mb->get_button_index() == BUTTON_RIGHT) {
			if (mb->is_pressed()) {
				//look for points to move
				if (spatial_pos_is_near(gpoint)) {
					edit_state = EditState::ROTATING;
					edited_point = _get_spatial_rotation();
					pre_move_edit = edited_point;
					canvas_item_editor->update_viewport();
					return true;
				} else {
					edit_state = EditState::STOPED;
				}
			}else {
				//apply moved point
				if (edit_state == EditState::ROTATING){
					edit_state = EditState::STOPED;
					undo_redo->create_action(TTR("Edit spatial Rotation"));
					_action_set_spatial_rotation(pre_move_edit, _get_spatial_rotation());
					_commit_action();
					return true;
				}
			}
		}
	}
	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		Vector2 gpoint = mm->get_position();
		if (edit_state == EditState::MOVING && ((mm->get_button_mask() & BUTTON_MASK_LEFT))) {
			Vector2 cpoint = node->get_global_transform().affine_inverse().xform(canvas_item_editor->snap_point(canvas_item_editor->get_canvas_transform().affine_inverse().xform(gpoint)));
			if(cpoint.distance_to(Vector2()) < 10.0f){
				edited_point = Vector3();
			}
			else{
				edited_point.x = cpoint.x;
				edited_point.y = cpoint.y;
			}
			_set_spatial_position(edited_point);
			canvas_item_editor->update_viewport();
		} else if (edit_state == EditState::ROTATING && ((mm->get_button_mask() & BUTTON_MASK_RIGHT))){
			Vector2 cpoint = node->get_global_transform().affine_inverse().xform(canvas_item_editor->snap_point(canvas_item_editor->get_canvas_transform().affine_inverse().xform(gpoint)));
			Vector3 spatial_position = _get_spatial_position();
			Vector2 scale = 96.0 * canvas_item_editor->get_canvas_transform().get_scale() / OS::get_singleton()->get_screen_dpi(OS::get_singleton()->get_current_screen());
			Vector3 euler = Transform().looking_at(Vector3((-cpoint.x + spatial_position.x) * scale.x, (-cpoint.y + spatial_position.y) * scale.y, -100.0), Vector3(0, 1, 0)).basis.get_euler();
			edited_point.x = euler.x;
			edited_point.y = euler.y;
			_set_spatial_rotation(edited_point);
			canvas_item_editor->update_viewport();
		}
	}
	Ref<InputEventKey> k = p_event;
	if (k.is_valid() && k->is_pressed()) {

	}
	return false;
}

void RootBone2DEditor::forward_canvas_draw_over_viewport(Control *p_overlay) {
	if (!_get_node())
		return;
	Transform2D xform = canvas_item_editor->get_canvas_transform() * _get_node()->get_global_transform();
	Ref<Texture> handle = get_icon("EditorPathSharpHandle", "EditorIcons");
	Vector3 pos3d = _get_spatial_position();
	Vector2 pos2d = Vector2(pos3d.x, pos3d.y);
	const Vector2 point = xform.xform(pos2d);
	const Color modulate = EditState::STOPED ? Color(1, 1, 1) : Color(0.5, 1, 2);
	p_overlay->draw_texture(handle, point - handle->get_size() * 0.5, modulate);

	// if(edit_state == EditState::ROTATING){
	// 	p_overlay->draw_circle(point, 100.0, Color(1.0, 0.5, 0.5, 0.5));
	// 	draw_empty_circle(p_overlay, point, 100.0, Color(1.0, 1.0, 1.0, 0.5), 100.0, 1, true);
	// }
}

void RootBone2DEditor::draw_empty_circle(Control *p_overlay, const Vector2 &p_center, float p_radius, const Color &p_color, float p_width, int p_resolution, bool p_antialiased){
	float draw_counter = 1.0f;
	Vector2 clock = Vector2(p_radius, 0.0f);
	Vector2 line_origin = clock + p_center;
	Vector2 line_end;
	while (draw_counter <= 360){
		line_end = clock.rotated(Math::deg2rad(draw_counter)) + p_center;
		p_overlay->draw_line(line_origin, line_end, p_color);
		draw_counter += 1 / p_resolution;
		line_origin = line_end;
	}
	line_end = clock.rotated(Math::deg2rad(360.0f)) + p_center;
	p_overlay->draw_line(line_origin, line_end, p_color, p_width, p_antialiased);
}

Vector3 RootBone2DEditor::_get_spatial_position() const {
	RootBone2D *node = _get_node();
	// Transform c = Variant(node->get_transform());
	// c.origin.z = node->get_depth_position();
	Transform c = node->get_spatial_transform();

	return c.xform_inv(_get_node()->get_rest_position());
	/////////// return _get_node()->get_rest_position() - _get_node()->get_position3d();
}

void RootBone2DEditor::_set_spatial_position(const Vector3 &p_polygon) const {
	RootBone2D *node = _get_node();

	// Transform c = Variant(node->get_transform());
	// c.origin.z = node->get_depth_position();
	Transform c = node->get_spatial_transform();

	node->set("rest_position", c.xform(p_polygon));
	// node->set("rest_position", p_polygon + _get_node()->get_position3d());
	node->_change_notify("rest_position");
}

void RootBone2DEditor::_action_set_spatial_position(const Vector3 &p_previous, const Vector3 &p_polygon) {
	RootBone2D *node = _get_node();
	// Transform c = Variant(node->get_transform());
	// c.origin.z = node->get_depth_position();
	Transform c = node->get_spatial_transform();

	undo_redo->add_do_method(node, "set_rest_position", c.xform(p_polygon));// p_polygon + _get_node()->get_position3d());
	undo_redo->add_undo_method(node, "set_rest_position", c.xform(p_previous));// p_previous + _get_node()->get_position3d());
	node->_change_notify("rest_position");
}

void RootBone2DEditor::_action_set_spatial_position(const Vector3 &p_polygon) {
	_action_set_spatial_position(_get_spatial_position(), p_polygon);
}

Vector3 RootBone2DEditor::_get_spatial_rotation() const {
	return _get_node()->get("spatial_rotation");
}

void RootBone2DEditor::_set_spatial_rotation(const Vector3 &p_polygon) const {
	RootBone2D *node = _get_node();
	node->set("spatial_rotation", p_polygon);
	node->_change_notify("spatial_rotation_degrees");
}

void RootBone2DEditor::_action_set_spatial_rotation(const Vector3 &p_previous, const Vector3 &p_polygon) {
	Node2D *node = _get_node();
	undo_redo->add_do_method(node, "set_spatial_rotation", p_polygon);
	undo_redo->add_undo_method(node, "set_spatial_rotation", p_previous);
	node->_change_notify("spatial_rotation_degrees");
}

void RootBone2DEditor::_action_set_spatial_rotation(const Vector3 &p_polygon) {
	_action_set_spatial_rotation(_get_spatial_rotation(), p_polygon);
}


void RootBone2DEditor::_commit_action() {
	undo_redo->add_do_method(canvas_item_editor, "update_viewport");
	undo_redo->add_undo_method(canvas_item_editor, "update_viewport");
	undo_redo->commit_action();
}

void RootBone2DEditor::_menu_option(int p_option) {
	RootBone2D *n = nullptr;
	List<Node *> selection = EditorInterface::get_singleton()->get_selection()->get_selected_node_list();
	UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	bool commit = false;
	switch (p_option) {
		case MENU_OPTION_MAKE_REST: {
			ur->create_action(TTR("Make Selected Rest Pose"));
			for (List<Node *>::Element *E = selection.front(); E; E = E->next()) {
				n = Object::cast_to<RootBone2D>(E->get());
				if (!n)
					continue;
				ur->add_do_method(n, "set_rest", n->get_spatial_transform());
				ur->add_undo_method(n, "set_rest", n->get_rest());
				commit = true;
			}
		} break;
		case MENU_OPTION_TO_REST: {
			ur->create_action(TTR("Set Selected to Rest Pose"));
			for (List<Node *>::Element *E = selection.front(); E; E = E->next()) {
				n = Object::cast_to<RootBone2D>(E->get());
				if (!n)
					continue;
				ur->add_do_method(n, "set_spatial_transform", n->get_rest());
				ur->add_undo_method(n, "set_spatial_transform", n->get_spatial_transform());
				commit = true;
			}
		} break;
	}
	if(commit)
		ur->commit_action();
}

void RootBone2DEditor::_bind_methods() {
	ClassDB::bind_method("_menu_option", &RootBone2DEditor::_menu_option);
}

RootBone2DEditor::RootBone2DEditor(EditorNode *p_editor) {
	node = NULL;
	canvas_item_editor = NULL;
	undo_redo = EditorNode::get_undo_redo();

	edit_state = EditState::STOPED;

	options = memnew(MenuButton);
	CanvasItemEditor::get_singleton()->add_control_to_menu_panel(options);
	options->set_text(TTR("RootBone2D"));
	options->set_icon(EditorNode::get_singleton()->get_gui_base()->get_icon("Skeleton2D", "EditorIcons"));
	options->get_popup()->add_item(TTR("Make Rest Pose"), MENU_OPTION_MAKE_REST);
	options->get_popup()->add_separator();
	options->get_popup()->add_item(TTR("Set to Rest Pose"), MENU_OPTION_TO_REST);
	// options->get_popup()->add_item(TTR("Make Selected Presumed Pose"), MENU_OPTION_MAKE_PRESUMED);
	// options->get_popup()->add_item(TTR("Set Selected to Presumed Pose"), MENU_OPTION_TO_PRESUMED);
	options->set_switch_on_hover(true);
	options->get_popup()->connect("id_pressed", this, "_menu_option");
}

void RootBone2DEditorPlugin::edit(Object *p_object) {

	transform_editor->edit(Object::cast_to<Node>(p_object));
}

bool RootBone2DEditorPlugin::handles(Object *p_object) const {

	return p_object->is_class(klass);
}

void RootBone2DEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		transform_editor->options->show();
	} else {
		transform_editor->options->hide();
		transform_editor->edit(NULL);
	}
}

RootBone2DEditorPlugin::RootBone2DEditorPlugin(EditorNode *p_node) :
		editor(p_node){
	transform_editor = memnew(RootBone2DEditor(p_node)); 
	klass = "RootBone2D";
	CanvasItemEditor::get_singleton()->add_control_to_menu_panel(transform_editor);
	make_visible(false);
}

RootBone2DEditorPlugin::~RootBone2DEditorPlugin() {
}
