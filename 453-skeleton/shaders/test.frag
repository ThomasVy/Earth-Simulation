#version 330 core

in vec4 fragPos;
in vec4 n;
in vec2 tc;

uniform vec3 light;
uniform sampler2D sampler;
out vec4 color;


void main() {
	vec4 lightDir = normalize(vec4(light, 0.0) - fragPos);
    vec4 normal = normalize(n);
    float diff = max(dot(lightDir, normal), 0.0);

	vec4 d = texture(sampler, tc);
	if(d.a < 0.01)
        discard; // If the texture is transparent, don't draw the fragment
	color = vec4(diff * d);
}
