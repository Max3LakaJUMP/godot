/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"

#include "psd.h"
#include "jscn.h"

#include "red_frame_editor_plugin.h"
#include "red_bubble_editor_plugin.h"
#include "red_line_editor.h"
#include "red_polygon_editor_plugin.h"
#include "editor/editor_node.h"
#include "scene_export_editor_plugin.h"

#include "resource_importer_psd.h"
#include "resource_importer_jscn.h"
void register_red_editor_types() {


	/*
	file_popup->add_shortcut(ED_SHORTCUT("editor/new_scene", TTR("New Scene")), FILE_NEW_SCENE);
	file_popup->add_separator();*/

#ifdef TOOLS_ENABLED
	ClassDB::register_class<PSD>();
	ClassDB::register_class<JSCN>();
    EditorPlugins::add_by_type<REDFrameEditorPlugin>();
	EditorPlugins::add_by_type<REDBubbleEditorPlugin>();
	EditorPlugins::add_by_type<REDLineEditorPlugin>();
	EditorPlugins::add_by_type<REDPolygonEditorPlugin>();
	EditorPlugins::add_by_type<SceneExportEditorPlugin>();

	Ref<ResourceImporterJSCN> import_jscn;
	import_jscn.instance();
	ResourceFormatImporter::get_singleton()->add_importer(import_jscn);
	
	Ref<ResourceImporterPSD> import_psd;
	import_psd.instance();
	ResourceFormatImporter::get_singleton()->add_importer(import_psd);
#endif
}

void unregister_red_editor_types() {
	// Nothing to do here in this example.
}
