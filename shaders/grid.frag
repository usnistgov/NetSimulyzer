#version 330

in vec3 fragment_position;

out vec4 final_color;

uniform float intensity;
uniform vec3 eye_position;
uniform float discard_distance;

void main() {
    float eye_fragment_distance = abs(distance(eye_position, fragment_position));
    if (eye_fragment_distance > discard_distance) {
        discard;
    }

    // Should never exceed 1, as those fragments are discarded
    float attenuation = eye_fragment_distance/discard_distance;
    float color = clamp(intensity - attenuation, 0, 1);
    final_color = vec4(color, color, color, 1.0f);
}
