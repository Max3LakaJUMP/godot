/*************************************************************************/
/*  line_2d_editor_plugin.cpp                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "modules/red/red_transform.h"
#include "red_transform_editor_plugin.h"
#include "editor/plugins/canvas_item_editor_plugin.h"

void REDTransformEditor::_node_removed(Node *p_node) {
	if (p_node == node) {
		node = NULL;
		options->hide();
	}
}

void REDTransformEditor::edit(Node *p_node) {
	if (!canvas_item_editor)
		canvas_item_editor = CanvasItemEditor::get_singleton();
	if (p_node) {
		_set_node(p_node);
		canvas_item_editor->update_viewport();
	} else {
		_set_node(NULL);
	}
}

REDTransform *REDTransformEditor::_get_node() const {
	return node;
}

void REDTransformEditor::_set_node(Node *p_node) {
	node = (REDTransform*)(p_node);
}

bool REDTransformEditor::custom_pos_is_near(const Vector2 &p_pos) const {
	const real_t grab_threshold = EDITOR_GET("editors/poly_editor/point_grab_radius");
	const Transform2D xform = canvas_item_editor->get_canvas_transform() * _get_node()->get_global_transform();
	Vector3 pos3d = _get_custom_position();
	Vector2 pos2d = Vector2(pos3d.x, pos3d.y);
	Vector2 cp = xform.xform(pos2d);
	real_t d = cp.distance_to(p_pos);
	return d < grab_threshold;
}

bool REDTransformEditor::forward_gui_input(const Ref<InputEvent> &p_event) {
	REDTransform *node = _get_node();
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
				if (custom_pos_is_near(gpoint)) {
					edit_state = EditState::MOVING;
					edited_point = _get_custom_position();
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
					undo_redo->create_action(TTR("Edit Custom Position"));
					_action_set_custom_position(pre_move_edit, _get_custom_position());
					_commit_action();
					return true;
				}
			}
		}else if (mb->get_button_index() == BUTTON_RIGHT) {
			if (mb->is_pressed()) {
				//look for points to move
				if (custom_pos_is_near(gpoint)) {
					edit_state = EditState::ROTATING;
					edited_point = _get_custom_rotation();
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
					_action_set_custom_rotation(pre_move_edit, _get_custom_rotation());
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
			_set_custom_position(edited_point);
			canvas_item_editor->update_viewport();
		} else if (edit_state == EditState::ROTATING && ((mm->get_button_mask() & BUTTON_MASK_RIGHT))){
			Vector2 cpoint = node->get_global_transform().affine_inverse().xform(canvas_item_editor->snap_point(canvas_item_editor->get_canvas_transform().affine_inverse().xform(gpoint)));
			Vector3 custom_position = _get_custom_position();
			Vector2 scale = 96.0 * canvas_item_editor->get_canvas_transform().get_scale() / OS::get_singleton()->get_screen_dpi(OS::get_singleton()->get_current_screen());
			Vector3 euler = Transform().looking_at(Vector3((-cpoint.x + custom_position.x) * scale.x, (-cpoint.y + custom_position.y) * scale.y, -100.0), Vector3(0, 1, 0)).basis.get_euler();
			edited_point.x = euler.x;
			edited_point.y = euler.y;
			_set_custom_rotation(edited_point);
			canvas_item_editor->update_viewport();
		}
	}
	Ref<InputEventKey> k = p_event;
	if (k.is_valid() && k->is_pressed()) {

	}
	return false;
}

void REDTransformEditor::forward_canvas_draw_over_viewport(Control *p_overlay) {
	if (!_get_node())
		return;
	Transform2D xform = canvas_item_editor->get_canvas_transform() * _get_node()->get_global_transform();
	Ref<Texture> handle = get_icon("EditorPathSharpHandle", "EditorIcons");
	Vector3 pos3d = _get_custom_position();
	Vector2 pos2d = Vector2(pos3d.x, pos3d.y);
	const Vector2 point = xform.xform(pos2d);
	const Color modulate = EditState::STOPED ? Color(1, 1, 1) : Color(0.5, 1, 2);
	p_overlay->draw_texture(handle, point - handle->get_size() * 0.5, modulate);

	// if(edit_state == EditState::ROTATING){
	// 	p_overlay->draw_circle(point, 100.0, Color(1.0, 0.5, 0.5, 0.5));
	// 	draw_empty_circle(p_overlay, point, 100.0, Color(1.0, 1.0, 1.0, 0.5), 100.0, 1, true);
	// }
}

void REDTransformEditor::draw_empty_circle(Control *p_overlay, const Vector2 &p_center, float p_radius, const Color &p_color, float p_width, int p_resolution, bool p_antialiased){
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

Vector3 REDTransformEditor::_get_custom_position() const {
	REDTransform *node = _get_node();
	Transform c = Variant(node->get_transform());
	c.origin.z = node->get_depth_position();
	
	return c.xform_inv(_get_node()->get_rest_position());
	// return _get_node()->get_rest_position() - _get_node()->get_position3d();
}

void REDTransformEditor::_set_custom_position(const Vector3 &p_polygon) const {
	REDTransform *node = _get_node();

	Transform c = Variant(node->get_transform());
	c.origin.z = node->get_depth_position();

	node->set("rest_position", c.xform(p_polygon));
	// node->set("rest_position", p_polygon + _get_node()->get_position3d());
	node->_change_notify("rest_position");
}

void REDTransformEditor::_action_set_custom_position(const Vector3 &p_previous, const Vector3 &p_polygon) {
	REDTransform *node = _get_node();
	Transform c = Variant(node->get_transform());
	c.origin.z = node->get_depth_position();

	undo_redo->add_do_method(node, "set_rest_position", c.xform(p_polygon));// p_polygon + _get_node()->get_position3d());
	undo_redo->add_undo_method(node, "set_rest_position", c.xform(p_previous));// p_previous + _get_node()->get_position3d());
	node->_change_notify("rest_position");
}

void REDTransformEditor::_action_set_custom_position(const Vector3 &p_polygon) {
	_action_set_custom_position(_get_custom_position(), p_polygon);
}

Vector3 REDTransformEditor::_get_custom_rotation() const {
	return _get_node()->get("custom_rotation");
}

void REDTransformEditor::_set_custom_rotation(const Vector3 &p_polygon) const {
	REDTransform *node = _get_node();
	node->set("custom_rotation", p_polygon);
	node->_change_notify("custom_rotation_degrees");
}

void REDTransformEditor::_action_set_custom_rotation(const Vector3 &p_previous, const Vector3 &p_polygon) {
	Node2D *node = _get_node();
	undo_redo->add_do_method(node, "set_custom_rotation", p_polygon);
	undo_redo->add_undo_method(node, "set_custom_rotation", p_previous);
	node->_change_notify("custom_rotation_degrees");
}

void REDTransformEditor::_action_set_custom_rotation(const Vector3 &p_polygon) {
	_action_set_custom_rotation(_get_custom_rotation(), p_polygon);
}


void REDTransformEditor::_commit_action() {
	undo_redo->add_do_method(canvas_item_editor, "update_viewport");
	undo_redo->add_undo_method(canvas_item_editor, "update_viewport");
	undo_redo->commit_action();
}

void REDTransformEditor::_menu_option(int p_option) {
	REDTransform *n = nullptr;
	List<Node *> selection = EditorInterface::get_singleton()->get_selection()->get_selected_node_list();
	for (List<Node *>::Element *E = selection.front(); E; E = E->next()) {
		n = Object::cast_to<REDTransform>(E->get());
		if (!n)
			continue;
		switch (p_option) {
			case MENU_OPTION_MAKE_REST: {
				UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
				ur->create_action(TTR("Make Selected Rest Pose"));
				ur->add_do_method(n, "set_rest3d", n->get_transform3d());
				ur->add_undo_method(n, "set_rest3d", n->get_rest3d());
				ur->commit_action();
			} break;
			case MENU_OPTION_TO_REST: {
				UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
				ur->create_action(TTR("Set Selected to Rest Pose"));
				ur->add_do_method(n, "set_transform3d", n->get_rest3d());
				ur->add_undo_method(n, "set_transform3d", n->get_transform3d());
				ur->commit_action();
			} break;
		}
	}
}

void REDTransformEditor::_bind_methods() {
	ClassDB::bind_method("_menu_option", &REDTransformEditor::_menu_option);
}

REDTransformEditor::REDTransformEditor(EditorNode *p_editor) {
	node = NULL;
	canvas_item_editor = NULL;
	undo_redo = EditorNode::get_undo_redo();

	edit_state = EditState::STOPED;

	options = memnew(MenuButton);
	CanvasItemEditor::get_singleton()->add_control_to_menu_panel(options);
	options->set_text(TTR("REDTranform"));
	options->set_icon(EditorNode::get_singleton()->get_gui_base()->get_icon("Skeleton2D", "EditorIcons"));
	options->get_popup()->add_item(TTR("Make Rest Pose"), MENU_OPTION_MAKE_REST);
	options->get_popup()->add_separator();
	options->get_popup()->add_item(TTR("Set to Rest Pose"), MENU_OPTION_TO_REST);
	// options->get_popup()->add_item(TTR("Make Selected Presumed Pose"), MENU_OPTION_MAKE_PRESUMED);
	// options->get_popup()->add_item(TTR("Set Selected to Presumed Pose"), MENU_OPTION_TO_PRESUMED);
	options->set_switch_on_hover(true);
	options->get_popup()->connect("id_pressed", this, "_menu_option");
}

void REDTransformEditorPlugin::edit(Object *p_object) {

	transform_editor->edit(Object::cast_to<Node>(p_object));
}

bool REDTransformEditorPlugin::handles(Object *p_object) const {

	return p_object->is_class(klass);
}

void REDTransformEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		transform_editor->options->show();
	} else {
		transform_editor->options->hide();
		transform_editor->edit(NULL);
	}
}

REDTransformEditorPlugin::REDTransformEditorPlugin(EditorNode *p_node) :
		editor(p_node){
	transform_editor = memnew(REDTransformEditor(p_node)); 
	klass = "REDTransform";
	CanvasItemEditor::get_singleton()->add_control_to_menu_panel(transform_editor);
	make_visible(false);
}

REDTransformEditorPlugin::~REDTransformEditorPlugin() {
}
