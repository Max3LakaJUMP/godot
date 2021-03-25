#include "red_shape_editor_plugin.h"
#include "modules/red/red_shape.h"

Node2D *REDShapeEditor::_get_node() const {

	return node;
}

void REDShapeEditor::_set_node(Node *p_line) {
	node = (REDShape*)(p_line);
}

bool REDShapeEditor::_is_line() const {

	return false;
}

Vector2 REDShapeEditor::_get_offset(int p_idx) const {

	return node->get_offset();
}

Variant REDShapeEditor::_get_polygon(int p_idx) const {
	return _get_node()->get("polygon");
}

void REDShapeEditor::_set_polygon(int p_idx, const Variant &p_polygon) const {
	_get_node()->set("polygon", p_polygon);
}
void REDShapeEditor::_action_set_polygon(int p_idx, const Variant &p_previous, const Variant &p_polygon) {
	Node2D *node = _get_node();
	undo_redo->add_do_method(node, "set_polygon", p_polygon);
	undo_redo->add_undo_method(node, "set_polygon", p_previous);
}
void REDShapeEditor::_action_set_polygon(int p_idx, const Variant &p_polygon) {

	_action_set_polygon(p_idx, _get_polygon(p_idx), p_polygon);
}

void REDShapeEditor::_action_add_polygon(const Variant &p_polygon) {
	_action_set_polygon(0, p_polygon);
}

REDShapeEditor::REDShapeEditor(EditorNode *p_editor) :
		AbstractPolygon2DEditor(p_editor) {
	node = NULL;
}

REDShapeEditorPlugin::REDShapeEditorPlugin(EditorNode *p_node) :
		AbstractPolygon2DEditorPlugin(p_node, memnew(REDShapeEditor(p_node)), "REDShape") {
}
