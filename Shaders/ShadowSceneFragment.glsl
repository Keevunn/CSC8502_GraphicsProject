#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D shadowTex;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform float lightRadius;

in Vertex {
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj;
} IN;

out vec4 fragColour;

void main(void) {
	vec3 incident = normalize(lightPos - IN.worldPos);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	vec3 normal = texture(bumpTex, IN.texCoord).rgb;
	normal = normalize(TBN * normal * 2 - 1);
	float lambert = max(dot(incident, normal), 0);
	float dist = length(lightPos - IN.worldPos);
	float attenuation = 1 - clamp(dist / lightRadius, 0, 1);

	float specFactor = clamp(dot(halfDir, normal), 0, 1);
	specFactor = pow(specFactor, 60);

	float shadow = 1;

	vec3 shadowNDC = IN.shadowProj.xyz / IN.shadowProj.w;
	if (abs(shadowNDC.x) < 1 && abs(shadowNDC.y) < 1 && abs(shadowNDC.z) < 1) {
		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
		float shadowZ = texture(shadowTex, biasCoord.xy).x;
		if (shadowZ < biasCoord.z) {
			shadow = 0;
		}
	}

	vec3 surface = diffuse.rgb * lightColour.rgb; //Base colour
	fragColour.rgb = surface * attenuation * lambert; //Diffuse
	fragColour.rgb += lightColour.rgb * attenuation * specFactor * 0.33;
	fragColour.rgb *= shadow; //shadowing factor
	fragColour.rgb += surface * 0.1f; //ambient
	fragColour.a = diffuse.a; // alpha
}