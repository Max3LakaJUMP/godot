#include "red_line_editor.h"
#include "modules/red/red_line.h"

Node2D *REDLineEditor::_get_node() const {

	return node;
}

void REDLineEditor::_set_node(Node *p_line) {

	node = Object::cast_to<REDLine>(p_line);
}

bool REDLineEditor::_is_line() const {

	return true;
}

Variant REDLineEditor::_get_polygon(int p_idx) const {

	return _get_node()->get("points");
}

void REDLineEditor::_set_polygon(int p_idx, const Variant &p_polygon) const {

	_get_node()->set("points", p_polygon);
}

void REDLineEditor::_action_set_polygon(int p_idx, const Variant &p_previous, const Variant &p_polygon) {

	Node2D *node = _get_node();
	undo_redo->add_do_method(node, "set_points", p_polygon);
	undo_redo->add_undo_method(node, "set_points", p_previous);
}

REDLineEditor::REDLineEditor(EditorNode *p_editor) :
        REDAbstractPolygonEditor(p_editor) {
	node = NULL;
}

REDLineEditorPlugin::REDLineEditorPlugin(EditorNode *p_node) :
		REDAbstractPolygonEditorPlugin(p_node, memnew(REDLineEditor(p_node)), "REDLine") {
}
