#version 420 core
struct Material {
	sampler2D diffuse1;
	sampler2D specular1;
	sampler2D normal1;

	vec3 specular;
	float shininess;
};

struct Light {
	vec3 position; // point light, spotlight
	vec3 direction; // direction, spotlight
	float innercutoff; // spotlight
	float outercutoff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant; // point light, spotlight
	float linear;
	float quadratic;
};
struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	sampler2D shadow_map;
	bool casts_shadow;
};
struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	samplerCube shadow_map;
	bool casts_shadow;
};
struct SpotLight {
	vec3 position;
	vec3 direction;
	float innercutoff;
	float outercutoff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	sampler2D shadow_map;
	bool casts_shadow;
};

// random
float rand(float x)
{
	return sin(x * x * 932.473);
}

// perlin noise
const int p[] = { 151,160,137,91,90,15,
			131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
			190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
			88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
			77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
			102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
			135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
			5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
			223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
			129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
			251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
			49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
			138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
			151,160,137,91,90,15,
			131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
			190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
			88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
			77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
			102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
			135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
			5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
			223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
			129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
			251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
			49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
			138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };

float fade(float t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}
float lerp(float t, float a, float b)
{
	return a + t * (b - a);
}
float grad(int hash, float x, float y, float z)
{
	int h = hash & 15;
	float u = h < 8 ? x : y;
	float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}
float perlin(float x, float y, float z)
{
	int X = int(floor(x)) & 255;
	int Y = int(floor(y)) & 255;
	int Z = int(floor(z)) & 255;

	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	float u = fade(x);
	float v = fade(y);
	float w = fade(z);

	int A = p[X] + Y;
	int AA = p[A] + Z;
	int AB = p[A + 1] + Z;
	int B = p[X + 1] + Y;
	int BA = p[B] + Z;
	int BB = p[B + 1] + Z;

	return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
								   grad(p[BA], x - 1, y, z)),
						   lerp(u, grad(p[AB], x, y - 1, z),
								   grad(p[BB], x - 1, y - 1, z))),
				   lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
								   grad(p[BA + 1], x - 1, y, z - 1)),
						   lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
								   grad(p[BB + 1], x - 1, y - 1, z - 1))));
}
class Perlin
{
	float scale_x; // how many "squares" per unit square
	float scale_y;
	float scale_z;
	float min_val;
	float max_val;

	float compute(vec3 pos)
	{
		float val = (perlin(pos.x * scale_x, pos.y * scale_y, pos.z * scale_z) + 1) / 2;
		return lerp(val, min_val, max_val);
	}
};

out vec4 FragColor;

//in vec3 vertexColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec3 LocalPos;
in vec4 FragPosLightSpace;

uniform vec3 viewPos;

uniform Material material;

uniform DirLight dirlight;
#define NUM_POINT_LIGHTS 4
uniform PointLight pointlight;//s[NUM_POINT_LIGHTS];
uniform SpotLight spotlight;

uniform vec3 fogColor;
uniform float far;

float dirLightInShadow(DirLight light, vec4 frag_light_space, vec3 lightDir, vec3 normal)
{
	vec3 proj_coords = frag_light_space.xyz / frag_light_space.w;
	proj_coords = proj_coords * 0.5 + 0.5;

	float bias = 0.0;// max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

	float current_depth = proj_coords.z;

	float shadow = 0.0;
	float closest_depth = texture(light.shadow_map, proj_coords.xy).r;
	float sample_separation = (current_depth - closest_depth) * 150.0 / textureSize(light.shadow_map, 0).x;
	//float sample_separation = 0.5 / textureSize(shadow_map, 0).x;
	for (int y = -2; y <= 2; ++y)
	{
		for (int x = -2; x <= 2; ++x)
		{
			float pcf_depth = texture(light.shadow_map, proj_coords.xy + vec2(x, y) * sample_separation).r;
			shadow += current_depth - bias > pcf_depth ? 0.0 : 1.0;
		}
	}

	shadow /= 25.0;

	if (proj_coords.z > 1.0)
		shadow = 1.0;

	return shadow;
}

float pointLightInShadow(PointLight light, vec3 frag_pos)
{
	vec3 frag_to_light = frag_pos - light.position;
	float closest_depth = texture(light.shadow_map, frag_to_light).r;
	closest_depth *= 25; // far_plane

	float current_depth = length(frag_to_light);

	float bias = 0.0;
	float shadow = current_depth - bias > closest_depth ? 0.0 : 1.0;

	return shadow;
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction.xyz);

	float shadow = 1.0;
	if(light.casts_shadow)
		shadow = dirLightInShadow(light, FragPosLightSpace, lightDir, normal);

	// ambient
	vec3 ambient = light.ambient;

	// diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * shadow;

	// specular
	//vec3 reflectDir = reflect(-lightDir, normal); // Phong lighting
	vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong
	float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);
	vec3 specular = material.specular * spec * light.specular * shadow;

	return (ambient + diffuse + specular);
}
vec3 calcPointLight(PointLight light, vec3 normal, vec3 FragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position.xyz - FragPos);

	float shadow = 1.0;
	if (light.casts_shadow)
		shadow = pointLightInShadow(light, FragPos);

	float distance = length(light.position.xyz - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// ambient
	vec3 ambient = light.ambient;

	// diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * shadow;

	// specular
	//vec3 reflectDir = reflect(-lightDir, normal); // Phong Lighting
	vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong
	float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);
	vec3 specular = material.specular * spec * light.specular * shadow;

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 FragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position.xyz - FragPos);

	float distance = length(light.position.xyz - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// ambient
	vec3 ambient = light.ambient;

	// diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse;

	// specular
	//vec3 reflectDir = reflect(-lightDir, normal); // Phong lighting
	vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong
	float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);
	vec3 specular = material.specular * spec * light.specular;

	diffuse *= attenuation;
	specular *= attenuation;

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.innercutoff - light.outercutoff;
	float intensity = clamp((theta - light.outercutoff) / epsilon, 0.0, 1.0);

	diffuse *= intensity;
	specular *= intensity;

	return (ambient + diffuse + specular);
}

float near = 0.1;

float linearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
	//FragColor = vec4(vertexColor, 1.0); // colors based on vertex
	//FragColor = texture(uTexture, TexCoord) * vec4(vertexColor, 1.0); // colors texture mixed with vertex colors

	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	//texture
	//float scale1 = 1;
	//float scale2 = 3;
	//float scale3 = 5;

	//vec2 offs1 = vec2(0, 50);
	//vec2 offs2 = vec2(100, 150);
	//vec2 offs3 = vec2(200, 250);

	//perlin
	//float offsetx3 = perlin(LocalPos.x * scale3 + offs3.x, LocalPos.y * scale3 + offs3.x, LocalPos.z * scale3 + offs1.x) / 16;
	//float offsety3 = perlin(LocalPos.x * scale3 + offs3.y, LocalPos.y * scale3 + offs3.y, LocalPos.z * scale3 + offs1.y) / 16;
	//float offsetx1 = perlin(LocalPos.x * scale1 * offsetx3 * 64 + offs1.x, LocalPos.y * scale1 * offsety3 * 64 + offs1.x, LocalPos.z * scale1 + offs1.x) / 16; // offset
	//float offsety1 = perlin(LocalPos.x * scale1 * offsetx3 * 64 + offs1.y, LocalPos.y * scale1 * offsety3 * 64 + offs1.y, LocalPos.z * scale1 + offs1.y) / 16;
	//float offsetx2 = perlin(LocalPos.x * scale2 * offsetx3 * 64 + offs2.x, LocalPos.y * scale2 * offsety3 * 64 + offs2.x, LocalPos.z * scale2 + offs1.x) / 16;
	//float offsety2 = perlin(LocalPos.x * scale2 * offsetx3 * 64 + offs2.y, LocalPos.y * scale2 * offsety3 * 64 + offs2.y, LocalPos.z * scale2 + offs1.y) / 16;

	

	/*vec3 textureColor = texture(material.diffuse1, TexCoord + vec2(offsetx1, offsety1)
														   + vec2(offsetx2, offsety2)
														   + vec2(offsetx3, offsety3)).xyz;*/
	vec4 textureColor = texture(material.diffuse1, TexCoord);
	if (textureColor.w < 0.1)
		discard;

	//if (LocalPos.y < -0.9f)
	//	textureColor = vec3(1, 1, 1);
	vec3 result = vec3(0.0);
	
	result += calcDirLight(dirlight, norm, viewDir);

	//for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
		result += calcPointLight(pointlight, norm, FragPos, viewDir);

	result += calcSpotLight(spotlight, norm, FragPos, viewDir);

	result *= textureColor.xyz;

	//result = mix(result, fogColor, linearizeDepth(gl_FragCoord.z) / far);

	FragColor = vec4(result, 1.0);
}