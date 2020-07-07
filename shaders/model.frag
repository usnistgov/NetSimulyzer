#version 330

in vec4 color;
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

uniform vec3 material_color;

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
    vec4 lights = calculateDirectionalLight() + calculatePointLights() + calculateSpotLights();
    if (useTexture) {
        final_color = texture(texture_sampler, texture_coordinates) * lights;
    } else {
        final_color = vec4(material_color, 1) * lights;
    }
}
