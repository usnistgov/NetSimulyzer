#version 330

uniform uint object_type;
uniform uint object_id;

out uvec3 picking_fragment;

void main() {
    // The leading 1.0 is to indicate the presence of an object
    // in the texture, since by default, OpenGL will, clear to 0
    // so, we need some way to tell an object was rendered at this
    // fragment
    picking_fragment = uvec3(1.0, object_type, object_id);
}
