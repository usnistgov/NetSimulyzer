#version 330

in vec2 texture_coordinates;
in vec3 normal;
in vec3 fragment_position;

out vec4 final_color;

const uint maxPointLights = 5u;
const uint maxSpotLights = 5u;

struct Light {
    vec3 color;
    float ambient_intensity;
    float diffuse_intensity;
};

struct DirectionalLight {
    Light base;
    vec3 direction;
};

struct PointLight {
    Light base;
    vec3 position;
    float constant;
    float linear;
    float exponent;
};

struct SpotLight {
    PointLight pointLight;
    vec3 direction;
    float edge;
};

struct Material {
    float specular_intensity;
    float shininess;
};

uniform uint pointLightCount = 0u;
uniform uint spotLightCount = 0u;

uniform DirectionalLight directional_light;
uniform PointLight pointLights[maxPointLights];
uniform SpotLight spotLights[maxSpotLights];

uniform bool useTexture;
uniform sampler2D texture_sampler;
uniform Material material;
uniform vec3 eye_position;
uniform float saturation_factor = 1.0f;

uniform vec3 material_color;

// `hsv` & `rgb` Significantly optimized from the article http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
// Although somewhat esoteric...

vec3 hsv(vec3 rgb_color) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(rgb_color.bg, K.wz), vec4(rgb_color.gb, K.xy), step(rgb_color.b, rgb_color.g));
    vec4 q = mix(vec4(p.xyw, rgb_color.r), vec4(rgb_color.r, p.yzx), step(p.x, rgb_color.r));

    float d = q.x - min(q.w, q.y);
    const float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 rgb(vec3 hsv_color) {
    const vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(hsv_color.xxx + K.xyz) * 6.0 - K.www);
    return hsv_color.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), hsv_color.y);
}

vec4 lightByDirection(Light base, vec3 direction) {
    vec4 ambient_color = vec4(base.color, 1.0) * base.ambient_intensity;

    // Between 0 and 1 (the `max()`)
    // cosine of the angle betweem the normal & the light source
    float diffuse_factor = max(dot(normalize(normal), normalize(direction)), 0.0f);
    vec4 diffuse_color = vec4(base.color * base.diffuse_intensity * diffuse_factor, 1.0f);

    vec4 specular_color = vec4(0, 0, 0, 0);
    if (diffuse_factor > 0.0f) {
        vec3 fragment_to_eye = normalize(eye_position - fragment_position);
        vec3 reflected_vertex = normalize(reflect(direction, normalize(normal)));

        float specular_factor = dot(fragment_to_eye, reflected_vertex);
        if (specular_factor > 0.0f) {
            specular_factor = pow(specular_factor, material.shininess);
            specular_color = vec4(base.color * material.specular_intensity * specular_factor, 1.0f);
        }
    }

    return ambient_color + diffuse_color + specular_color;
}

vec4 calculateDirectionalLight() {
    return lightByDirection(directional_light.base, directional_light.direction);
}

vec4 calculatePointLight(PointLight light) {
    vec3 direction = fragment_position - light.position;
    float distance = length(direction);
    direction = normalize(direction);

    vec4 color = lightByDirection(light.base, direction);

    // ax^2 + bx + c
    float attenuation = light.exponent * distance * distance + light.linear * distance + light.constant;

    return color / attenuation;
}

vec4 calculateSpotLight(SpotLight light) {
    vec3 rayDirection = normalize(fragment_position - light.pointLight.position);

    // Angle between the fragment and the light's direction
    float factor = dot(rayDirection, light.direction);
    if (factor > light.edge) {
        vec4 color = calculatePointLight(light.pointLight);

        // Scale the output color 0 to 1.0
        // softens the edges of the spotlight
        float scale =  1.0f - (1.0f - factor) * (1.0f/(1.0f - light.edge));

        return color * scale;
    }

    return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

vec4 calculatePointLights() {
    vec4 total = vec4(0, 0, 0, 0);

    for (uint i = 0u; i < pointLightCount; i++) {
        total += calculatePointLight(pointLights[i]);
    }

    return total;
}

vec4 calculateSpotLights() {
    vec4 total = vec4(0, 0, 0, 0);

    for (uint i = 0u; i < spotLightCount; i++) {
        total += calculateSpotLight(spotLights[i]);
    }

    return total;
}

void main()
{
    // Choose Material color or Texture for the base
    final_color = mix(vec4(material_color, 1.0), texture(texture_sampler, texture_coordinates), useTexture);

    // Apply lighting
    final_color *= calculateDirectionalLight() + calculatePointLights() + calculateSpotLights();

    // Adjust final saturation, based on `saturation_factor`
    vec3 hsv = hsv(final_color.rgb);
    hsv[1] = clamp(hsv[1] * saturation_factor, 0.0, 1.0);
    final_color.rgb = rgb(hsv);
}
