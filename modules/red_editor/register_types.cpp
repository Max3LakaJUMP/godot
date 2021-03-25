/* register_types.cpp */

#ifdef TOOLS_ENABLED
#include "register_types.h"

#include "core/class_db.h"
#include "editor/editor_node.h"

#include "psd.h"
#include "mediapipe.h"
#include "red_render_data.h"
#include "maya.h"

#include "resource_importer_psd.h"

#include "red_shape_editor_plugin.h"
#include "red_root_bone_editor_plugin.h"
#include "red_deform_editor_plugin.h"
#include "mediapipe_editor_plugin.h"
#include "red_tools_editor_plugin.h"

static Ref<ResourceImporterPSD> import_psd;
#endif
	
void register_red_editor_types() {
#ifdef TOOLS_ENABLED
	ClassDB::register_class<REDRenderData>();
	ClassDB::register_class<PSD>();
	ClassDB::register_class<Mediapipe>();
	ClassDB::register_class<Maya>();

    EditorPlugins::add_by_type<MediapipeEditorPlugin>();
	EditorPlugins::add_by_type<REDShapeEditorPlugin>();
	EditorPlugins::add_by_type<REDToolsEditorPlugin>();
	EditorPlugins::add_by_type<RootBone2DEditorPlugin>();
	EditorPlugins::add_by_type<REDDeformEditorPlugin>();

	import_psd.instance();
	ResourceFormatImporter::get_singleton()->add_importer(import_psd);
	Mediapipe::load_dll();
#endif
}

void unregister_red_editor_types() {
#ifdef TOOLS_ENABLED
	ResourceFormatImporter::get_singleton()->remove_importer(import_psd);
	import_psd.unref();
	Mediapipe::free_dll();
#endif
}
