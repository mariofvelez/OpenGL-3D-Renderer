#version 420 core
in vec2	TexCoord;

out vec4 FragColor;

uniform sampler2D screenTexture;

void invert()
{
	FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoord)), 1.0);
}
vec4 grayscale(vec4 col)
{
	float average = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
	return vec4(average, average, average, 1.0);
}
vec4 gamma_correct(vec4 col)
{
    float gamma = 2.2;
    return vec4(pow(col.rgb, vec3(1.0 / gamma)), col.w);
}
const float offsetx = 1.0 / 450.0;
const float offsety = 1.0 / 300.0;
const float[9] kernel = float[] (
	1, 1, 1,
	1,-8, 1,
	1, 1, 1
);
const vec2 offsets[9] = vec2[](
    vec2(-offsetx, offsety), // top-left
    vec2(0.0f, offsety), // top-center
    vec2(offsetx, offsety), // top-right
    vec2(-offsetx, 0.0f),   // center-left
    vec2(0.0f, 0.0f),   // center-center
    vec2(offsetx, 0.0f),   // center-right
    vec2(-offsetx, -offsety), // bottom-left
    vec2(0.0f, -offsety), // bottom-center
    vec2(offsetx, -offsety)  // bottom-right
);
vec4 convolve()
{
    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; ++i)
        col += vec3(texture(screenTexture, TexCoord.st + offsets[i])) * kernel[i];

    return vec4(col, 1.0);
}
vec4 thresh(vec4 col)
{
    if (col.x < 0.1)
        return vec4(0.0, 0.0, 0.0, 1.0);
    return vec4(1.0);
}

void main()
{
	//vec4 col = convolve();
    //col = grayscale(col);
    //col = thresh(col);
    vec4 screen = texture(screenTexture, TexCoord);
    screen = gamma_correct(screen);
    //screen = grayscale(screen);
    FragColor = screen;
}