#version 330

// TODO: get vertex attributes from each location
layout(location = 0) in vec3 v_position;
layout(location = 3) in vec3 v_normal;
layout(location = 2) in vec2 v_coord;
layout(location = 1) in vec3 v_color;
layout(location = 4) in vec3 v_colour_vec;
layout(location = 5) in vec3 v_colour_vec2;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 ColourVec;
uniform vec3 ColourVec2;

// TODO: output values to fragment shader
out vec2 frag_coord;
out vec3 frag_normal;
out vec3 frag_color;
out vec3 frag_position;

void main()
{
	// TODO: send output to fragment shader
	frag_position = v_position;
	frag_normal = v_normal;
	frag_color = ColourVec + (ColourVec2 - ColourVec);
	frag_coord = v_coord;

	// TODO: compute gl_Position
	gl_Position = Projection * View * Model * vec4(v_position, 1.0);
}
