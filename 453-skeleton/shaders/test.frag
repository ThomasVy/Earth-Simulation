#version 330 core

in vec4 objectFragPos;
in vec4 n;
in vec2 tc;

uniform vec3 lightPos;
uniform sampler2D sampler;
uniform vec3 cameraPos;
uniform bool applyShading;
out vec4 color;


void main() {
	vec4 lightDir = vec4(lightPos, 1.0) - objectFragPos;
	float lightDistance = length(lightDir);
	vec4 normalizedLightDir = normalize(lightDir);
    vec4 normal = normalize(n);
    float diff = max(dot(normalizedLightDir, normal), 0.0);

	vec4 viewDir = normalize(vec4(cameraPos, 1.0) - objectFragPos);
	vec4 reflectedLightDir = normalize(2 * dot(normalizedLightDir, normal) * normal - normalizedLightDir);
	float alpha = 16;
	float specular = pow(max(dot(reflectedLightDir, viewDir), 0.0), alpha);

	float ambient = 0.02;
	vec4 d = texture(sampler, tc);
	if(d.a < 0.01)
        discard; // If the texture is transparent, don't draw the fragment

	if(applyShading)
		color = ((diff+specular)/pow(lightDistance,2)+ambient) * d;
	else
		color = d;
}
