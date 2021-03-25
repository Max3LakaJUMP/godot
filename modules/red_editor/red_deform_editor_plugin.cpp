#include "modules/red/red_deform.h"
#include "red_deform_editor_plugin.h"
#include "editor/plugins/canvas_item_editor_plugin.h"

void REDDeformEditor::_node_removed(Node *p_node) {
	if (p_node == node) {
		node = NULL;
		// options->hide();
	}
}

void REDDeformEditor::edit(Node *p_node) {
	if (!canvas_item_editor)
		canvas_item_editor = CanvasItemEditor::get_singleton();
	if (p_node) {
		_set_node(p_node);
		canvas_item_editor->update_viewport();
	} else {
		_set_node(NULL);
	}
}

REDDeform *REDDeformEditor::_get_node() const {
	return node;
}

void REDDeformEditor::_set_node(Node *p_node) {
	node = (REDDeform*)(p_node);
}

bool REDDeformEditor::forward_gui_input(const Ref<InputEvent> &p_event) {
	REDDeform *node = _get_node();
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
		// if (mb->get_button_index() == BUTTON_LEFT) {
		// 	if (mb->is_pressed()) {
		// 		//look for points to move
		// 		if (_wind_direction_is_near(gpoint)) {
		// 			edit_state = EditState::MOVING;
		// 			edited_point = _get_custom_position();
		// 			pre_move_edit = edited_point;
		// 			canvas_item_editor->update_viewport();
		// 			return true;
		// 		} else {
		// 			edit_state = EditState::STOPED;
		// 		}
		// 	}else {
		// 		//apply moved point
		// 		if (edit_state == EditState::MOVING) {
		// 			edit_state = EditState::STOPED;
		// 			undo_redo->create_action(TTR("Edit Custom Position"));
		// 			_action_set_custom_position(pre_move_edit, _get_custom_position());
		// 			_commit_action();
		// 			return true;
		// 		}
		// 	}
		// }else 
		if (mb->get_button_index() == BUTTON_LEFT) {
			if (mb->is_pressed()) {
				//look for points to move
				if (_wind_direction_is_near(gpoint)) {
					edit_state = EditState::ROTATING;
					edited_point = _get_wind_rotation();
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
					undo_redo->create_action(TTR("Edit Custom Rotation"));
					_action_set_wind_rotation(pre_move_edit, _get_wind_rotation());
					_commit_action();
					return true;
				}
			}
		}
	}
	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		Vector2 gpoint = mm->get_position();
		// if (edit_state == EditState::MOVING && ((mm->get_button_mask() & BUTTON_MASK_LEFT))) {
		// 	Vector2 cpoint = node->get_global_transform().affine_inverse().xform(canvas_item_editor->snap_point(canvas_item_editor->get_canvas_transform().affine_inverse().xform(gpoint)));
		// 	if(cpoint.distance_to(Vector2()) < 10.0f){
		// 		edited_point = Vector3();
		// 	}
		// 	else{
		// 		edited_point.x = cpoint.x;
		// 		edited_point.y = cpoint.y;
		// 	}
		// 	_set_custom_position(edited_point);
		// 	canvas_item_editor->update_viewport();
		// } else 
		if (edit_state == EditState::ROTATING && ((mm->get_button_mask() & BUTTON_MASK_LEFT))){
 			// Vector2 cpoint = node->get_global_transform().affine_inverse().xform(canvas_item_editor->snap_point(canvas_item_editor->get_canvas_transform().affine_inverse().xform(gpoint)));
			Vector2 cpoint = canvas_item_editor->snap_point(canvas_item_editor->get_canvas_transform().affine_inverse().xform(gpoint)) - node->get_global_transform().get_origin();
			Vector3 target(cpoint.x, cpoint.y, 0.0);
			float len = target.length();
			float target_len = 100 / canvas_item_editor->get_canvas_transform().get_scale().x;
			if(len < target_len){
				target.z = -Math::sqrt(target_len * target_len - target.x * target.x - target.y * target.y);
			}
			else if(len > target_len){
				target.normalize();
				target = target * target_len;
			}
			Vector3 euler = Transform().looking_at(target, Vector3(0, 1, 0)).basis.get_euler();
			edited_point.x = euler.x;
			edited_point.y = euler.y;
			_set_wind_rotation(edited_point);
			canvas_item_editor->update_viewport();
		}
	}
	Ref<InputEventKey> k = p_event;
	if (k.is_valid() && k->is_pressed()) {

	}
	return false;
}

void REDDeformEditor::forward_canvas_draw_over_viewport(Control *p_overlay) {
	if (!_get_node())
		return;
	Transform2D xform = canvas_item_editor->get_canvas_transform() * _get_node()->get_global_transform();
	Ref<Texture> handle = get_icon("EditorPathSharpHandle", "EditorIcons");

	Vector3 pos3d = _get_wind_direction();
	const Vector2 point = xform.get_origin() + Vector2(pos3d.x, pos3d.y);
	// const Vector2 point = xform.xform(Vector2(pos3d.x, pos3d.y));
	const Color modulate = edit_state == EditState::STOPED ? Color(0.5, 1, 2) : Color(2, 0.5, 0.25);

	// if(edit_state == EditState::ROTATING){
		p_overlay->draw_circle(xform.get_origin(), 100.0, modulate * Color(1, 1, 1, 0.25));
		draw_empty_circle(p_overlay, xform.get_origin(), 100.0, modulate, 2.f, 10.f, true);
	// }
	p_overlay->draw_line(xform.get_origin(), point, modulate, 4.0f, true);
	p_overlay->draw_texture(handle, point - handle->get_size() * 0.5, modulate);
}

void REDDeformEditor::draw_empty_circle(Control *p_overlay, const Vector2 &p_center, float p_radius, const Color &p_color, float p_width, int p_resolution, bool p_antialiased){
	float draw_counter = p_resolution;
	Vector2 clock = Vector2(p_radius, 0.0f);
	Vector2 line_origin = clock + p_center;
	Vector2 line_end;
	while (draw_counter <= 360){
		line_end = clock.rotated(Math::deg2rad(draw_counter)) + p_center;
		p_overlay->draw_line(line_origin, line_end, p_color, p_width, p_antialiased);
		draw_counter += p_resolution;
		line_origin = line_end;
	}
	line_end = clock.rotated(Math::deg2rad(360.0f)) + p_center;
	p_overlay->draw_line(line_origin, line_end, p_color, p_width, p_antialiased);
}

Vector3 REDDeformEditor::_get_custom_position() const {
	// REDDeform *node = _get_node();
	// Transform c = Variant(node->get_transform());
	// c.origin.z = node->get_depth_position();
	
	// return c.xform_inv(_get_node()->get_rest_position());
	return Vector3();
}

Vector2 REDDeformEditor::_get_wind_rotation() const {
	return _get_node()->get("wind_rotation");
}

Vector3 REDDeformEditor::_get_wind_direction() const {
	Vector2 wind_euler = _get_wind_rotation();
	Transform transform(Basis(Vector3(wind_euler.x, wind_euler.y, 0.f)));
	return transform.xform(Vector3(0, 0, -100));
}

bool REDDeformEditor::_wind_direction_is_near(const Vector2 &p_pos) const {
	const real_t grab_threshold = EDITOR_GET("editors/poly_editor/point_grab_radius");
	const Transform2D xform = canvas_item_editor->get_canvas_transform() * _get_node()->get_global_transform();
	Vector2 object_point = xform.get_origin();

	Vector3 pos3d = _get_wind_direction(); // canvas_item_editor->get_canvas_transform().get_scale().x;
	const Vector2 point = object_point + Vector2(pos3d.x, pos3d.y); // xform.xform(Vector2(pos3d.x, pos3d.y));
	real_t d = (point).distance_to(p_pos);
	return d < grab_threshold;
}

void REDDeformEditor::_set_wind_rotation(const Vector2 &p_rotation) const {
	REDDeform *node = _get_node();
	node->set("wind_rotation", p_rotation);
	node->_change_notify("wind_rotation_degrees");
}

void REDDeformEditor::_action_set_wind_rotation(const Vector2 &p_previous, const Vector2 &p_rotation) {
	Node2D *node = _get_node();
	undo_redo->add_do_method(node, "set_wind_rotation", p_rotation);
	undo_redo->add_undo_method(node, "set_wind_rotation", p_previous);
	node->_change_notify("wind_rotation_degrees");
}

void REDDeformEditor::_action_set_wind_rotation(const Vector2 &p_rotation) {
	_action_set_wind_rotation(_get_wind_rotation(), p_rotation);
}

void REDDeformEditor::_commit_action() {
	undo_redo->add_do_method(canvas_item_editor, "update_viewport");
	undo_redo->add_undo_method(canvas_item_editor, "update_viewport");
	undo_redo->commit_action();
}

void REDDeformEditor::_menu_option(int p_option) {
	// REDDeform *n = nullptr;
	// List<Node *> selection = EditorInterface::get_singleton()->get_selection()->get_selected_node_list();
	// for (List<Node *>::Element *E = selection.front(); E; E = E->next()) {
	// 	n = Object::cast_to<REDDeform>(E->get());
	// 	if (!n)
	// 		continue;
	// 	switch (p_option) {
	// 		case MENU_OPTION_MAKE_REST: {
	// 			UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	// 			ur->create_action(TTR("Make Selected Rest Pose"));
	// 			ur->add_do_method(n, "set_rest3d", n->get_Deform3d());
	// 			ur->add_undo_method(n, "set_rest3d", n->get_rest3d());
	// 			ur->commit_action();
	// 		} break;
	// 		case MENU_OPTION_TO_REST: {
	// 			UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	// 			ur->create_action(TTR("Set Selected to Rest Pose"));
	// 			ur->add_do_method(n, "set_Deform3d", n->get_rest3d());
	// 			ur->add_undo_method(n, "set_Deform3d", n->get_Deform3d());
	// 			ur->commit_action();
	// 		} break;
	// 	}
	// }
}

void REDDeformEditor::_bind_methods() {
	ClassDB::bind_method("_menu_option", &REDDeformEditor::_menu_option);
}

REDDeformEditor::REDDeformEditor(EditorNode *p_editor) {
	node = NULL;
	canvas_item_editor = NULL;
	undo_redo = EditorNode::get_undo_redo();

	edit_state = EditState::STOPED;

	// options = memnew(MenuButton);
	// CanvasItemEditor::get_singleton()->add_control_to_menu_panel(options);
	// options->set_text(TTR("REDTranform"));
	// options->set_icon(EditorNode::get_singleton()->get_gui_base()->get_icon("Skeleton2D", "EditorIcons"));
	// options->get_popup()->add_item(TTR("Make Rest Pose"), MENU_OPTION_MAKE_REST);
	// options->get_popup()->add_separator();
	// options->get_popup()->add_item(TTR("Set to Rest Pose"), MENU_OPTION_TO_REST);
	// // options->get_popup()->add_item(TTR("Make Selected Presumed Pose"), MENU_OPTION_MAKE_PRESUMED);
	// // options->get_popup()->add_item(TTR("Set Selected to Presumed Pose"), MENU_OPTION_TO_PRESUMED);
	// options->set_switch_on_hover(true);
	// options->get_popup()->connect("id_pressed", this, "_menu_option");
}

void REDDeformEditorPlugin::edit(Object *p_object) {

	deform_editor->edit(Object::cast_to<Node>(p_object));
}

bool REDDeformEditorPlugin::handles(Object *p_object) const {

	return p_object->is_class(klass);
}

void REDDeformEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		// deform_editor->options->show();
	} else {
		// deform_editor->options->hide();
		deform_editor->edit(NULL);
	}
}

REDDeformEditorPlugin::REDDeformEditorPlugin(EditorNode *p_node) :
		editor(p_node){
	deform_editor = memnew(REDDeformEditor(p_node)); 
	klass = "REDDeform";
	CanvasItemEditor::get_singleton()->add_control_to_menu_panel(deform_editor);
	make_visible(false);
}

REDDeformEditorPlugin::~REDDeformEditorPlugin() {
}
