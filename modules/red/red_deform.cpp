/*************************************************************************/
/*  node_2d.cpp                                                          */
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

#include "red_deform.h"

#include "core/message_queue.h"
#include "scene/gui/control.h"
#include "scene/main/viewport.h"
#include "servers/visual_server.h"

void REDDeform::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		VisualServer::get_singleton()->deform_set_wind_rotation(ci, wind_rotation);
		VisualServer::get_singleton()->deform_set_wind_offset(ci, wind_offset);
		VisualServer::get_singleton()->deform_set_wind1_time(ci, wind1_time);
		VisualServer::get_singleton()->deform_set_wind1_strength(ci, wind1_strength);
		VisualServer::get_singleton()->deform_set_wind2_time(ci, wind2_time);
		VisualServer::get_singleton()->deform_set_wind2_strength(ci, wind2_strength);
		VisualServer::get_singleton()->deform_set_scale_time(ci, scale_time);
		VisualServer::get_singleton()->deform_set_scale_strength(ci, scale_strength);
		VisualServer::get_singleton()->deform_set_waves_count(ci, waves_count);
		VisualServer::get_singleton()->deform_set_elasticity(ci, elasticity);
	}
}

float REDDeform::get_wind_rotation() const{
	return wind_rotation;
}

void REDDeform::set_wind_rotation(float p_wind_rotation){
	wind_rotation = p_wind_rotation;
	VisualServer::get_singleton()->deform_set_wind_rotation(ci, p_wind_rotation);
}

float REDDeform::get_wind_offset() const{
	return wind_offset;
}

void REDDeform::set_wind_offset(float p_wind_offset){
	wind_offset = p_wind_offset;
	VisualServer::get_singleton()->deform_set_wind_offset(ci, p_wind_offset);
}

float REDDeform::get_wind1_time() const{
	return wind1_time;
}

void REDDeform::set_wind1_time(float p_wind1_time) {
	wind1_time = p_wind1_time;
	VisualServer::get_singleton()->deform_set_wind1_time(ci, p_wind1_time);
}

float REDDeform::get_wind1_strength() const{
	return wind1_strength;
}
void REDDeform::set_wind1_strength(float p_wind1_strength) {
	wind1_strength = p_wind1_strength;
	VisualServer::get_singleton()->deform_set_wind1_strength(ci, p_wind1_strength);
}

float REDDeform::get_wind2_time() const{
	return wind2_time;
}
void REDDeform::set_wind2_time(float p_wind2_time) {
	wind2_time = p_wind2_time;
	VisualServer::get_singleton()->deform_set_wind2_time(ci, p_wind2_time);
}
float REDDeform::get_wind2_strength() const{
	return wind2_strength;
}
void REDDeform::set_wind2_strength(float p_wind2_strength) {
	wind2_strength = p_wind2_strength;
	VisualServer::get_singleton()->deform_set_wind2_strength(ci, p_wind2_strength);
}

float REDDeform::get_scale_time() const{
	return scale_time;
}
void REDDeform::set_scale_time(float p_scale_time){
	scale_time = p_scale_time;
	VisualServer::get_singleton()->deform_set_scale_time(ci, p_scale_time);
}

float REDDeform::get_scale_strength() const{
	return scale_strength;
}

void REDDeform::set_scale_strength(float p_scale_strength){
	scale_strength = p_scale_strength;
	VisualServer::get_singleton()->deform_set_scale_strength(ci, p_scale_strength);
}


float REDDeform::get_waves_count() const{
	return waves_count;
}
void REDDeform::set_waves_count(float p_waves_count) {
	waves_count = p_waves_count;
	VisualServer::get_singleton()->deform_set_waves_count(ci, p_waves_count);
}

float REDDeform::get_elasticity() const{
	return elasticity;
}
void REDDeform::set_elasticity(float p_elasticity) {
	elasticity = p_elasticity;
	VisualServer::get_singleton()->deform_set_elasticity(ci, p_elasticity);
}


RID REDDeform::get_ci(){
	return ci;
}

void REDDeform::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_wind_rotation", "wind_rotation"), &REDDeform::set_wind_rotation);
	ClassDB::bind_method(D_METHOD("get_wind_rotation"), &REDDeform::get_wind_rotation);
	ClassDB::bind_method(D_METHOD("set_wind_offset", "wind_offset"), &REDDeform::set_wind_offset);
	ClassDB::bind_method(D_METHOD("get_wind_offset"), &REDDeform::get_wind_offset);

	ClassDB::bind_method(D_METHOD("set_wind1_time", "wind1_time"), &REDDeform::set_wind1_time);
	ClassDB::bind_method(D_METHOD("get_wind1_time"), &REDDeform::get_wind1_time);
	ClassDB::bind_method(D_METHOD("set_wind1_strength", "wind1_strength"), &REDDeform::set_wind1_strength);
	ClassDB::bind_method(D_METHOD("get_wind1_strength"), &REDDeform::get_wind1_strength);

	ClassDB::bind_method(D_METHOD("set_wind2_time", "wind2_time"), &REDDeform::set_wind2_time);
	ClassDB::bind_method(D_METHOD("get_wind2_time"), &REDDeform::get_wind2_time);
	ClassDB::bind_method(D_METHOD("set_wind2_strength", "wind2_strength"), &REDDeform::set_wind2_strength);
	ClassDB::bind_method(D_METHOD("get_wind2_strength"), &REDDeform::get_wind2_strength);

	ClassDB::bind_method(D_METHOD("set_scale_time", "scale_time"), &REDDeform::set_scale_time);
	ClassDB::bind_method(D_METHOD("get_scale_time"), &REDDeform::get_scale_time);
	ClassDB::bind_method(D_METHOD("set_scale_strength", "scale_strength"), &REDDeform::set_scale_strength);
	ClassDB::bind_method(D_METHOD("get_scale_strength"), &REDDeform::get_scale_strength);
	
	ClassDB::bind_method(D_METHOD("set_waves_count", "waves_count"), &REDDeform::set_waves_count);
	ClassDB::bind_method(D_METHOD("get_waves_count"), &REDDeform::get_waves_count);
	ClassDB::bind_method(D_METHOD("set_elasticity", "elasticity"), &REDDeform::set_elasticity);
	ClassDB::bind_method(D_METHOD("get_elasticity"), &REDDeform::get_elasticity);

	ADD_GROUP("Wind base", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "waves_count", PROPERTY_HINT_RANGE, "0,10,0.1,or_lesser,or_greater"), "set_waves_count", "get_waves_count");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "elasticity", PROPERTY_HINT_RANGE, "0,1,0.1,or_lesser,or_greater"), "set_elasticity", "get_elasticity");

	ADD_GROUP("Wind1", "wind1_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "wind_rotation", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater"), "set_wind_rotation", "get_wind_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "wind_offset", PROPERTY_HINT_RANGE, "0,1,0.1,or_lesser,or_greater"), "set_wind_offset", "get_wind_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "wind1_time", PROPERTY_HINT_RANGE, "0,10,0.1, or_lesser,or_greater"), "set_wind1_time", "get_wind1_time");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "wind1_strength"), "set_wind1_strength", "get_wind1_strength");
	ADD_GROUP("Wind2", "wind2_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "wind2_time", PROPERTY_HINT_RANGE, "0,10,0.1, or_lesser,or_greater"), "set_wind2_time", "get_wind2_time");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "wind2_strength"), "set_wind2_strength", "get_wind2_strength");
	ADD_GROUP("Scale", "scale_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "scale_time", PROPERTY_HINT_RANGE, "0,10,0.1, or_lesser,or_greater"), "set_scale_time", "get_scale_time");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "scale_strength"), "set_scale_strength", "get_scale_strength");
}

REDDeform::REDDeform() {
	ci = VS::get_singleton()->deform_create();
	wind_rotation = 90.0f;
	wind_offset = 1.0f;
	waves_count = 0.5;
	elasticity = 0.5;
	wind1_time = 1.0f;
	wind1_strength = 100.0f;
	wind2_time = 1.0f;
	wind2_strength = 0.0;
	scale_time = 1.0f;
	scale_strength = 0.0f;
}

REDDeform::~REDDeform() {

	VS::get_singleton()->free(ci);
}