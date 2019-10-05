/* register_types.cpp */

#ifdef TOOLS_ENABLED
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


static Ref<ResourceImporterJSCN> import_jscn;
static Ref<ResourceImporterPSD> import_psd;
#endif
	
void register_red_editor_types() {
#ifdef TOOLS_ENABLED

	ClassDB::register_class<PSD>();
	ClassDB::register_class<JSCN>();
    	EditorPlugins::add_by_type<REDFrameEditorPlugin>();
	EditorPlugins::add_by_type<REDBubbleEditorPlugin>();
	EditorPlugins::add_by_type<REDLineEditorPlugin>();
	EditorPlugins::add_by_type<REDPolygonEditorPlugin>();
	EditorPlugins::add_by_type<SceneExportEditorPlugin>();
/*
	Ref<ResourceImporterJSCN> import_jscn;
	import_jscn.instance();
	ResourceFormatImporter::get_singleton()->add_importer(import_jscn);
	
	Ref<ResourceImporterPSD> import_psd;
	import_psd.instance();
	ResourceFormatImporter::get_singleton()->add_importer(import_psd);
*/
	import_jscn.instance();
	ResourceFormatImporter::get_singleton()->add_importer(import_jscn);
	import_psd.instance();
	ResourceFormatImporter::get_singleton()->add_importer(import_psd);
#endif
}

void unregister_red_editor_types() {
#ifdef TOOLS_ENABLED

	ResourceFormatImporter::get_singleton()->remove_importer(import_jscn);
	import_jscn.unref();
	ResourceFormatImporter::get_singleton()->remove_importer(import_psd);
	import_psd.unref();
#endif
}
