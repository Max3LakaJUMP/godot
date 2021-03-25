/*************************************************************************/
/*  resource_importer_texture_atlas.h                                    */
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

#ifndef RED_RENDER_DATA
#define RED_RENDER_DATA

#include "core/image.h"
#include "core/io/resource_importer.h"

#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"
#include "modules/red/red_clipper.h"
#include "scene/gui/viewport_container.h"
#include "core/array.h"
#include "scene/main/viewport.h"

class ShaderMaterial;
class ViewportContainer;
class Viewport;
class Camera2D;

class REDRenderData : public ViewportContainer {
	GDCLASS(REDRenderData, ViewportContainer);
private:
 	Ref<Material> back_material;
	Array materials;
	Viewport *viewport;
	Camera2D *camera;

	String render_path;	

	Size2 resolution;

	bool apply_normal;

	bool use_background;
	bool _need_render;
	bool _delete_after_render;
	bool _clean_viewport_before_render;
protected:
	void _notification(int p_what);
	static void _bind_methods();
public:
	void set_render_target(const Ref<ViewportTexture>);
	Ref<ViewportTexture> get_render_target() const;

	void set_back_material(const Ref<ShaderMaterial> p_materials);
	Ref<ShaderMaterial> get_back_material() const;
	void set_materials(const Array &p_materials);
	Array get_materials() const;
	void set_render_path(const String &p_render_path);
	String get_render_path() const;
	void set_resolution(const Size2 &p_resolution);
	Size2 get_resolution() const;
	void set_apply_normal(bool p_apply_normal);
	bool get_apply_normal() const;

	void set_use_background(bool p_use_background);
	bool get_use_background() const;
	void set_delete_after_render(bool p_delete_after_render);
	bool get_delete_after_render() const;
	void set_clean_viewport_before_render(bool p_clean_viewport_before_render);
	bool get_clean_viewport_before_render() const;
	
	Ref<ViewportTexture> render_target;	
	void render();
	void render(const Size2 &p_resolution_override);
	void _render0();
	void _render();
	void _normal_to_polygon();
	REDRenderData();
};

#endif // RED_RENDER_DATA
