#ifndef RED_LINE_2D_EDITOR_H
#define RED_LINE_2D_EDITOR_H

#include "editor/plugins/abstract_polygon_2d_editor.h"
#include "modules/red/red_line.h"

class REDLineEditor : public AbstractPolygon2DEditor {

	GDCLASS(REDLineEditor, AbstractPolygon2DEditor);

	REDLine *node;

protected:
	virtual Node2D *_get_node() const;
	virtual void _set_node(Node *p_line);

	virtual bool _is_line() const;
	virtual Variant _get_polygon(int p_idx) const;
	virtual void _set_polygon(int p_idx, const Variant &p_polygon) const;
	virtual void _action_set_polygon(int p_idx, const Variant &p_previous, const Variant &p_polygon);

public:
	REDLineEditor(EditorNode *p_editor);
};

class REDLineEditorPlugin : public AbstractPolygon2DEditorPlugin {

	GDCLASS(REDLineEditorPlugin, AbstractPolygon2DEditorPlugin);

public:
	REDLineEditorPlugin(EditorNode *p_node);

};

#endif // RED_LINE_2D_EDITOR_H
