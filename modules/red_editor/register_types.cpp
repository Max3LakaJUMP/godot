/* register_types.cpp */

#ifdef TOOLS_ENABLED
#include "register_types.h"

#include "core/class_db.h"

#include "psd.h"
#include "jscn.h"

#include "mediapipe_editor_plugin.h"
#include "red_shape_editor_plugin.h"
//#include "red_frame_editor_plugin.h"
//#include "red_bubble_editor_plugin.h"
#include "red_transform_editor_plugin.h"
#include "editor/editor_node.h"
#include "red_tools_editor_plugin.h"

#include "resource_importer_psd.h"
#include "resource_importer_jscn.h"
#include "mediapipe.h"
#include "red_render_data.h"
#include "maya.h"

static Ref<ResourceImporterJSCN> import_jscn;
static Ref<ResourceImporterPSD> import_psd;
#endif
	
void register_red_editor_types() {
#ifdef TOOLS_ENABLED
	ClassDB::register_class<REDRenderData>();
	ClassDB::register_class<PSD>();
	ClassDB::register_class<JSCN>();
	ClassDB::register_class<Mediapipe>();

	Mediapipe::load_dll();
    EditorPlugins::add_by_type<MediapipeEditorPlugin>();
	EditorPlugins::add_by_type<REDShapeEditorPlugin>();
    //EditorPlugins::add_by_type<REDFrameEditorPlugin>();
	//EditorPlugins::add_by_type<REDBubbleEditorPlugin>();
	EditorPlugins::add_by_type<REDToolsEditorPlugin>();
	EditorPlugins::add_by_type<REDTransformEditorPlugin>();
	import_jscn.instance();
	ResourceFormatImporter::get_singleton()->add_importer(import_jscn);
	import_psd.instance();
	ResourceFormatImporter::get_singleton()->add_importer(import_psd);
	
	ClassDB::register_class<Maya>();
#endif
}

void unregister_red_editor_types() {
#ifdef TOOLS_ENABLED
	ResourceFormatImporter::get_singleton()->remove_importer(import_jscn);
	import_jscn.unref();
	ResourceFormatImporter::get_singleton()->remove_importer(import_psd);
	import_psd.unref();
	Mediapipe::free_dll();
#endif
}
