#include "red_deform.h"

#include "core/message_queue.h"
#include "scene/gui/control.h"
#include "scene/main/viewport.h"
#include "servers/visual_server.h"

void REDDeform::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		VisualServer::get_singleton()->deform_set_wind_rotation(ci, wind_rotation);
		VisualServer::get_singleton()->deform_set_wind_offset(ci, wind_offset);
		VisualServer::get_singleton()->deform_set_wind_time(ci, wind_time);
		VisualServer::get_singleton()->deform_set_wind_strength(ci, wind_strength);
		VisualServer::get_singleton()->deform_set_wind2_time(ci, wind2_time);
		VisualServer::get_singleton()->deform_set_wind2_strength(ci, wind2_strength);
		VisualServer::get_singleton()->deform_set_scale_time(ci, scale_time);
		VisualServer::get_singleton()->deform_set_scale_strength(ci, scale_strength);
	}
}

void REDDeform::set_wind_rotation(const Vector2 &p_wind_rotation){
	wind_rotation = p_wind_rotation;
	VisualServer::get_singleton()->deform_set_wind_rotation(ci, p_wind_rotation);
	_change_notify("wind_rotation");
	_change_notify("wind_rotation_degrees");
}

Vector2 REDDeform::get_wind_rotation() const{
	return wind_rotation;
}

void REDDeform::set_wind_rotation_degrees(const Vector2 &p_wind_rotation){
	set_wind_rotation(Vector2(Math::deg2rad(p_wind_rotation.x), Math::deg2rad(p_wind_rotation.y)));
}

Vector2 REDDeform::get_wind_rotation_degrees() const{
	return Vector2(Math::rad2deg(wind_rotation.x), Math::rad2deg(wind_rotation.y));
}

float REDDeform::get_wind_offset() const{
	return wind_offset;
}

void REDDeform::set_wind_offset(float p_wind_offset){
	wind_offset = p_wind_offset;
	VisualServer::get_singleton()->deform_set_wind_offset(ci, p_wind_offset);
}

float REDDeform::get_wind_time() const{
	return wind_time;
}

void REDDeform::set_wind_time(float p_wind_time) {
	wind_time = p_wind_time;
	VisualServer::get_singleton()->deform_set_wind_time(ci, p_wind_time);
}

float REDDeform::get_wind_strength() const{
	return wind_strength;
}
void REDDeform::set_wind_strength(float p_wind_strength) {
	wind_strength = p_wind_strength;
	VisualServer::get_singleton()->deform_set_wind_strength(ci, p_wind_strength);
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
}


RID REDDeform::get_ci(){
	return ci;
}

void REDDeform::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_wind_rotation", "wind_rotation"), &REDDeform::set_wind_rotation);
	ClassDB::bind_method(D_METHOD("get_wind_rotation"), &REDDeform::get_wind_rotation);
	ClassDB::bind_method(D_METHOD("set_wind_rotation_degrees", "wind_rotation_degrees"), &REDDeform::set_wind_rotation_degrees);
	ClassDB::bind_method(D_METHOD("get_wind_rotation_degrees"), &REDDeform::get_wind_rotation_degrees);
	ClassDB::bind_method(D_METHOD("set_wind_offset", "wind_offset"), &REDDeform::set_wind_offset);
	ClassDB::bind_method(D_METHOD("get_wind_offset"), &REDDeform::get_wind_offset);

	ClassDB::bind_method(D_METHOD("set_wind_time", "wind_time"), &REDDeform::set_wind_time);
	ClassDB::bind_method(D_METHOD("get_wind_time"), &REDDeform::get_wind_time);
	ClassDB::bind_method(D_METHOD("set_wind_strength", "wind_strength"), &REDDeform::set_wind_strength);
	ClassDB::bind_method(D_METHOD("get_wind_strength"), &REDDeform::get_wind_strength);

	ClassDB::bind_method(D_METHOD("set_wind2_time", "wind2_time"), &REDDeform::set_wind2_time);
	ClassDB::bind_method(D_METHOD("get_wind2_time"), &REDDeform::get_wind2_time);
	ClassDB::bind_method(D_METHOD("set_wind2_strength", "wind2_strength"), &REDDeform::set_wind2_strength);
	ClassDB::bind_method(D_METHOD("get_wind2_strength"), &REDDeform::get_wind2_strength);

	ClassDB::bind_method(D_METHOD("set_scale_time", "scale_time"), &REDDeform::set_scale_time);
	ClassDB::bind_method(D_METHOD("get_scale_time"), &REDDeform::get_scale_time);
	ClassDB::bind_method(D_METHOD("set_scale_strength", "scale_strength"), &REDDeform::set_scale_strength);
	ClassDB::bind_method(D_METHOD("get_scale_strength"), &REDDeform::get_scale_strength);
	
	ADD_GROUP("wind", "wind_");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "wind_rotation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_wind_rotation", "get_wind_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "wind_rotation_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater"), "set_wind_rotation_degrees", "get_wind_rotation_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "wind_offset", PROPERTY_HINT_RANGE, "0,1,0.1,or_lesser,or_greater"), "set_wind_offset", "get_wind_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "wind_time", PROPERTY_HINT_RANGE, "0,10,0.1, or_lesser,or_greater"), "set_wind_time", "get_wind_time");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "wind_strength", PROPERTY_HINT_RANGE, "0,1000"), "set_wind_strength", "get_wind_strength");
	//ADD_GROUP("Wind2", "wind2_");
	//ADD_PROPERTY(PropertyInfo(Variant::REAL, "wind2_time", PROPERTY_HINT_RANGE, "0,10,0.1, or_lesser,or_greater"), "set_wind2_time", "get_wind2_time");
	//ADD_PROPERTY(PropertyInfo(Variant::REAL, "wind2_strength"), "set_wind2_strength", "get_wind2_strength");
	ADD_GROUP("Scale", "scale_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "scale_time", PROPERTY_HINT_RANGE, "0,10,0.1, or_lesser,or_greater"), "set_scale_time", "get_scale_time");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "scale_strength"), "set_scale_strength", "get_scale_strength");
}

REDDeform::REDDeform() {
	ci = VS::get_singleton()->deform_create();
	wind_rotation = Vector2(0, Math_PI * 0.5);

	// wind_rotation = 90.0f;
	wind_offset = 0.0f;

	wind_time = 1.0f;
	wind_strength = 10.0f;
	wind2_time = 1.0f;
	wind2_strength = 0.0;
	scale_time = 1.0f;
	scale_strength = 0.0f;
}

REDDeform::~REDDeform() {

	VS::get_singleton()->free(ci);
}