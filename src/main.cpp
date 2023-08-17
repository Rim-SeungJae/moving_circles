#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility
#include "circle.h"		// circle class definition

//*************************************
// global constants
static const char*	window_name = "cgbase - circle";
static const char*	vert_shader_path = "../bin/shaders/circ.vert";
static const char*	frag_shader_path = "../bin/shaders/circ.frag";
static const uint	MIN_TESS = 3;		// minimum tessellation factor (down to a triangle)
static const uint	MAX_TESS = 256;		// maximum tessellation factor (up to 256 triangles)
uint				NUM_TESS = 36;		// initial tessellation factor of the circle as a polygon
static const uint	MIN_CIRCLES = 2;		// minimum tessellation factor (down to a triangle)
static const uint	MAX_CIRCLES = 256;		// maximum tessellation factor (up to 256 triangles)
uint				NUM_CIRCLES = 20;

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2(1280,720); // initial window size

//*************************************
// OpenGL objects
GLuint	program = 0;		// ID holder for GPU program
GLuint	vertex_array = 0;	// ID holder for vertex array object

//*************************************
// global variables
int		frame = 0;						// index of rendering frames
float	t = 0.0f;						// current simulation parameter
bool	b_solid_color = true;			// use circle's color?
bool	b_index_buffer = true;			// use index buffering?
#ifndef GL_ES_VERSION_2_0
bool	b_wireframe = false;
#endif
auto	circles = std::move(create_circles(NUM_CIRCLES));
struct { bool add=false, sub=false; operator bool() const { return add||sub; } } b; // flags of keys for smooth changes

//*************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>	unit_circle_vertices;	// host-side vertices

std::vector<std::vector<float>> collision_graph(NUM_CIRCLES,std::vector<float>(NUM_CIRCLES,0));

//*************************************
void update()
{

	// tricky aspect correction matrix for non-square window
	float aspect = window_size.x/float(window_size.y);
	mat4 aspect_matrix = 
	{
		std::min(1/aspect,1.0f), 0, 0, 0,
		0, std::min(aspect,1.0f), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	// update common uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation( program, "b_solid_color" );	if(uloc>-1) glUniform1i( uloc, b_solid_color );
	uloc = glGetUniformLocation( program, "aspect_matrix" );	if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, aspect_matrix );

	// update vertex buffer by the pressed keys
	void update_cnum(); // forward declaration
	if(b) update_cnum(); 
}

void render()
{
	float dt;
	if (t != 0.0f) dt = float(glfwGetTime()) - t;
	else dt = 0;

	// update global simulation parameter
	t = float(glfwGetTime());

	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// notify GL that we use our own program
	glUseProgram( program );

	// bind vertex array object
	glBindVertexArray( vertex_array );

	// render two circles: trigger shader program to process vertex data
	int i = 0;
	for( auto& c1 : circles )
	{
		if (collide_up(c1))
		{
			if (c1.previous_up > up_dist(c1)) c1.velocity.x = -c1.velocity.x;
			c1.previous_up = up_dist(c1);
		}
		if (collide_down(c1))
		{
			if (c1.previous_down > down_dist(c1)) c1.velocity.x = -c1.velocity.x;
			c1.previous_down = down_dist(c1);
		}
		if (collide_left(c1))
		{
			if (c1.previous_left > left_dist(c1)) c1.velocity.y = -c1.velocity.y;
			c1.previous_left = left_dist(c1);
		}
		if (collide_right(c1))
		{
			if (c1.previous_right > right_dist(c1)) c1.velocity.y = -c1.velocity.y;
			c1.previous_right = right_dist(c1);
		}
		
		int j = 0;
		for (auto& c2 : circles)
		{
			if (&c1 == &c2)break;
			if (is_collide(c1, c2))
			{
				if (collision_graph[i][j] != 0)
				{
					if (overlap_dist(c1, c2) > collision_graph[i][j])
					{
						float dx = c1.center.x - c2.center.x;
						float dy = c1.center.y - c2.center.y;

						float ex = (c1.center.x - c2.center.x) / (sqrt(dx * dx + dy * dy));
						float ey = (c1.center.y - c2.center.y) / (sqrt(dx * dx + dy * dy));

						float vi1_x = (c1.velocity.x * dx + c1.velocity.y * dy) / (sqrt(dx * dx + dy * dy));
						float vf1_y = sqrt((c1.velocity.x * c1.velocity.x + c1.velocity.y * c1.velocity.y) - vi1_x * vi1_x);

						float vi2_x = (c2.velocity.x * dx + c2.velocity.y * dy) / (sqrt(dx * dx + dy * dy));
						float vf2_y = sqrt((c2.velocity.x * c2.velocity.x + c2.velocity.y * c2.velocity.y) - vi2_x * vi2_x);

						float vf1_x = (c1.radius * c1.radius - c2.radius * c2.radius) * vi1_x / (c1.radius * c1.radius + c2.radius + c2.radius) + 2 * c2.radius * c2.radius * vi2_x / (c1.radius * c1.radius + c2.radius * c2.radius);
						float vf2_x = (c2.radius * c2.radius - c1.radius * c1.radius) * vi2_x / (c2.radius * c2.radius + c1.radius + c1.radius) + 2 * c1.radius * c1.radius * vi1_x / (c2.radius * c2.radius + c1.radius * c1.radius);

						c1.velocity.x = vf1_x * ex + vf1_y * -ey;
						c1.velocity.y = vf1_x * ey + vf1_y * ex;
						c2.velocity.x = vf2_x * ex + vf2_y * -ey;
						c2.velocity.y = vf2_x * ey + vf2_y * ex;

					}
				}
				collision_graph[i][j] = overlap_dist(c1, c2);
			}
			else collision_graph[i][j] = 0.0f;
			j++;
		}
		
		// per-circle update
		c1.update(dt);

		// update per-circle uniforms
		GLint uloc;
		uloc = glGetUniformLocation( program, "solid_color" );		if(uloc>-1) glUniform4fv( uloc, 1, c1.color );	// pointer version
		uloc = glGetUniformLocation( program, "model_matrix" );		if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, c1.model_matrix );

		// per-circle draw calls
		if(b_index_buffer)	glDrawElements( GL_TRIANGLES, NUM_TESS*3, GL_UNSIGNED_INT, nullptr );
		else				glDrawArrays( GL_TRIANGLES, 0, NUM_TESS*3 ); // NUM_TESS = N
		i++;
	}

	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "- press 'd' to toggle between solid color and texture coordinates\n" );
	printf( "- press '+/-' to increase/decrease number of circles (min=%d, max=%d)\n", MIN_CIRCLES, MAX_CIRCLES );
	printf( "- press 'i' to toggle between index buffering and simple vertex buffering\n" );
#ifndef GL_ES_VERSION_2_0
	printf( "- press 'w' to toggle wireframe\n" );
#endif
	printf( "\n" );
}

std::vector<vertex> create_circle_vertices( uint N )
{
	std::vector<vertex> v = {{ vec3(0), vec3(0,0,-1.0f), vec2(0.5f) }}; // origin
	for( uint k=0; k <= N; k++ )
	{
		float t=PI*2.0f*k/float(N), c=cos(t), s=sin(t);
		v.push_back( { vec3(c,s,0), vec3(0,0,-1.0f), vec2(c,s)*0.5f+0.5f } );
	}
	return v;
}

void update_vertex_buffer( const std::vector<vertex>& vertices, uint N )
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if(vertex_buffer)	glDeleteBuffers( 1, &vertex_buffer );	vertex_buffer = 0;
	if(index_buffer)	glDeleteBuffers( 1, &index_buffer );	index_buffer = 0;

	// check exceptions
	if(vertices.empty()){ printf("[error] vertices is empty.\n"); return; }

	// create buffers
	if(b_index_buffer)
	{
		std::vector<uint> indices;
		for( uint k=0; k < N; k++ )
		{
			indices.push_back(0);	// the origin
			indices.push_back(k+1);
			indices.push_back(k+2);
		}

		// generation of vertex buffer: use vertices as it is
		glGenBuffers( 1, &vertex_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

		// geneation of index buffer
		glGenBuffers( 1, &index_buffer );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*indices.size(), &indices[0], GL_STATIC_DRAW );
	}
	else
	{
		std::vector<vertex> v; // triangle vertices
		for( uint k=0; k < N; k++ )
		{
			v.push_back(vertices.front());	// the origin
			v.push_back(vertices[k+1]);
			v.push_back(vertices[k+2]);
		}

		// generation of vertex buffer: use triangle_vertices instead of vertices
		glGenBuffers( 1, &vertex_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*v.size(), &v[0], GL_STATIC_DRAW );
	}

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if(vertex_array) glDeleteVertexArrays(1,&vertex_array);
	vertex_array = cg_create_vertex_array( vertex_buffer, index_buffer );
	if(!vertex_array){ printf("%s(): failed to create vertex aray\n",__func__); return; }
}

void update_cnum()
{
	uint n = NUM_CIRCLES;
	if (b.add)
	{
		n++;
		b.add = false;
	}
	if (b.sub)
	{
		n--;
		b.sub = false;
	}
	if(n==NUM_CIRCLES||n<MIN_CIRCLES||n>MAX_CIRCLES) return;
	
	circles = std::move(create_circles(NUM_CIRCLES=n));
	collision_graph.resize(NUM_CIRCLES, std::vector<float>(NUM_CIRCLES, 0));
	printf( "> number of circles = % -4d\r", NUM_CIRCLES );
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if(key==GLFW_KEY_KP_ADD||(key==GLFW_KEY_EQUAL&&(mods&GLFW_MOD_SHIFT)))	b.add = true;
		else if(key==GLFW_KEY_KP_SUBTRACT||key==GLFW_KEY_MINUS) b.sub = true;
		else if(key==GLFW_KEY_I)
		{
			b_index_buffer = !b_index_buffer;
			update_vertex_buffer( unit_circle_vertices,NUM_TESS );
			printf( "> using %s buffering\n", b_index_buffer?"index":"vertex" );
		}
		else if(key==GLFW_KEY_D)
		{
			b_solid_color = !b_solid_color;
			printf( "> using %s\n", b_solid_color ? "solid color" : "texture coordinates as color" );
		}
#ifndef GL_ES_VERSION_2_0
		else if(key==GLFW_KEY_W)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode( GL_FRONT_AND_BACK, b_wireframe ? GL_LINE:GL_FILL );
			printf( "> using %s mode\n", b_wireframe ? "wireframe" : "solid" );
		}
#endif
	}
}

void mouse( GLFWwindow* window, int button, int action, int mods )
{
	if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS )
	{
		dvec2 pos; glfwGetCursorPos(window,&pos.x,&pos.y);
		printf( "> Left mouse button pressed at (%d, %d)\n", int(pos.x), int(pos.y) );
	}
}

void motion( GLFWwindow* window, double x, double y )
{
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glLineWidth( 1.0f );
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	
	// define the position of four corner vertices
	unit_circle_vertices = std::move(create_circle_vertices( NUM_TESS ));

	// create vertex buffer; called again when index buffering mode is toggled
	update_vertex_buffer( unit_circle_vertices, NUM_TESS );

	return true;
}

void user_finalize()
{
}

int main( int argc, char* argv[] )
{
	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if(!(program=cg_create_program_from_string( "layout(location = 0) in vec3 position;\
	layout(location = 1) in vec3 normal;\
	layout(location = 2) in vec2 texcoord;\
\
	out vec3 norm;\
	out vec2 tc;\
\
	uniform mat4	model_matrix;\
	uniform mat4	aspect_matrix;\
\
\nvoid main()\
\n{\
\n	gl_Position = aspect_matrix * model_matrix * vec4(position, 1);\
\n\
	\n// other outputs to rasterizer/fragment shader\
	\nnorm = normal;\
	\ntc = texcoord;\
\n}", 
"#ifdef GL_ES\
\n\t#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined\
\n\t\t#define highp mediump\
\n\t#endif\
\n\tprecision highp float; // default precision needs to be defined\
\n#endif\
\n\
\n// inputs from vertex shader\
\nin vec2 tc;	// used for texture coordinate visualization\
\n\
\n// output of the fragment shader\
\nout vec4 fragColor;\
\n\
\n// shader's global variables, called the uniform variables\
\nuniform bool b_solid_color;\
\nuniform vec4 solid_color;\
\n\
\nvoid main()\
\n{\
\n	fragColor = b_solid_color ? solid_color : vec4(tc.xy, 0, 1);\
\n}" ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movements

	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}
	
	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}
