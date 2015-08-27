// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

#include "shader.hpp"
#include "MeshLoader.h"
#include "oldCamera.h"
#include <iostream>

#define SCENEMODEL "suzanne.obj"
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768



Mesh* mainMesh;
oldCamera* mainCamera;
int iteration;
float transparency = 0.3;

void quadDraw(GLuint quad_vertexbuffer)
{
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

		glDisableVertexAttribArray(0);
}

void createFramebuffer(GLuint& fbo, GLuint &fbtex) {
	glGenTextures(1, &fbtex);
	glBindTexture(GL_TEXTURE_2D, fbtex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbtex, 0);
	{
		GLenum status;
		status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		switch (status) {
			case GL_FRAMEBUFFER_COMPLETE:
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				cerr << "Error: unsupported framebuffer format" << endl;
				exit(0);
			default:
				cerr << "Error: invalid framebuffer config" << endl;
				exit(0);
		}
	}
}

GLuint createTexture(int w,int h)
{
	unsigned int textureId;
	glGenTextures(1,&textureId);
	glBindTexture(GL_TEXTURE_2D,textureId);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	
	int i;
	i=glGetError();
	if(i!=0)
	{
		std::cout << "Error happened while loading the texture: " << i << std::endl;
	}
	glBindTexture(GL_TEXTURE_2D,0);
	return textureId;
}


void keyEventHandle(){
	if (glfwGetKey(window, GLFW_KEY_R ) == GLFW_PRESS)
	{
		transparency +=0.01;
		if (transparency >= 1)
		{
			transparency = 1;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_E ) == GLFW_PRESS)
	{
		transparency -=0.01;
		if (transparency <= 0)
		{
			transparency = 0;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_1 ) == GLFW_PRESS)
	{
		iteration = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_2 ) == GLFW_PRESS)
	{
		iteration = 4;
	}if (glfwGetKey(window, GLFW_KEY_3 ) == GLFW_PRESS)
	{
		iteration = 8;
	}
	/*mainCamera->OnKeyboard();	
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	mainCamera->OnMouse(xpos, ypos);*/

}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "SPONZA", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(window, WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	iteration = 8;//default iteration time

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShadingRTT.vertexshader", "StandardShadingRTT.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	//// Load the texture
	//GLuint Texture = loadDDS("uvmap.DDS");
	//
	//// Get a handle for our "myTextureSampler" uniform
	//GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	mainMesh = new Mesh();
	bool mainMeshLoadSuccess = mainMesh->LoadMesh(SCENEMODEL);

	mainCamera = new oldCamera();


	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");


	//// ---------------------------------------------
	//// Render to Texture - specific code begins here
	//// ---------------------------------------------

	//// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	//GLuint FramebufferName = 0;
	//glGenFramebuffers(1, &FramebufferName);
	//glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	//// The texture we're going to render to
	//GLuint renderedTexture;
	//glGenTextures(1, &renderedTexture);
	//
	//// "Bind" the newly created texture : all future texture functions will modify this texture
	//glBindTexture(GL_TEXTURE_2D, renderedTexture);

	//// Give an empty image to OpenGL ( the last "0" means "empty" )
	//glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

	//// Poor filtering
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//GLuint renderTexture2 = createTexture(WINDOW_WIDTH, WINDOW_HEIGHT);


	//// The depth buffer
	//GLuint depthrenderbuffer;
	//glGenRenderbuffers(1, &depthrenderbuffer);
	//glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	////// Alternative : Depth texture. Slower, but you can sample it later in your shader
	////GLuint depthTexture;
	////glGenTextures(1, &depthTexture);
	////glBindTexture(GL_TEXTURE_2D, depthTexture);
	////glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT24, 1024, 768, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//// Set "renderedTexture" as our colour attachement #0
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

	////// Depth texture alternative : 
	////glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);


	//// Set the list of draw buffers.
	//GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	//glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	//// Always check that our framebuffer is ok
	//if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	//	return false;
	//**********************************************************************************************************************
	GLuint vert, frag, fragDiscreteV, fragDiscreteH, fragLinearV, fragLinearH, prog[2];
	prog[0] = LoadShaders( "passthrough.vs", "linear_vert.fs" );
	prog[1] = LoadShaders( "passthrough.vs", "linear_horiz.fs" );
	GLuint fbo[2], fbtex[2];
	createFramebuffer(fbo[0], fbtex[0]);
	createFramebuffer(fbo[1], fbtex[1]);
	GLuint blurV_texID = glGetUniformLocation(prog[0], "renderedTexture");
	GLuint blurH_texID = glGetUniformLocation(prog[1], "renderedTexture");

	// The fullscreen quad's FBO
	static const GLfloat g_quad_vertex_buffer_data[] = { 
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

	GLuint quad_vertexbuffer;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	//// Create and compile our GLSL program from the shaders
	//GLuint quad_programID = LoadShaders( "Passthrough.vertexshader", "WobblyTexture.fragmentshader" );//render the quad with a simple texture
	//GLuint texID = glGetUniformLocation(quad_programID, "renderedTexture");
	//GLuint timeID = glGetUniformLocation(quad_programID, "time");
	////glUniform2f(glGetUniformLocation(quad_programID,"pixelSize"),1.0/WINDOW_WIDTH,1.0/WINDOW_HEIGHT);

	//GLuint blur_programID = LoadShaders( "Passthrough.vertexshader", "blur.fragmentshader" );
	//GLuint blur_texID = glGetUniformLocation(blur_programID, "renderedTexture");
	//GLuint isVertical_ID = glGetUniformLocation(blur_programID,"isVertical");
	//GLuint pixelSize_ID = glGetUniformLocation(blur_programID,"pixelSize");
	//GLuint directionID = glGetUniformLocation(quad_programID, "direction");

	do{

		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);//enable the framebuffer
		glViewport(0,0,1024,768); // Render on the whole framebuffer, complete from the lower left corner to the upper right

		// Clear the frame buffer, the render the scene (the scene is on the frame buffer, basically render texture if it's attached in the screen buffer)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		mainCamera->computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = mainCamera->getProjectionMatrix();
		glm::mat4 ViewMatrix = mainCamera->getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		//glm::mat4 ModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(0.1,0.1,0.1));
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(10,10,10);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		//// Bind our texture in Texture Unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, Texture);
		//// Set our "myTextureSampler" sampler to user Texture Unit 0
		//glUniform1i(TextureID, 0);

		mainMesh->Render();
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		glUseProgram(0);

		//
		//bool b=true;
		//for(int i=0;i<20;i++)
		//{
		//	glBindFramebuffer(GL_FRAMEBUFFER,FramebufferName);
		//	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,b ? renderTexture2 : renderedTexture,0);
		//	glActiveTexture(GL_TEXTURE0);
		//	glBindTexture(GL_TEXTURE_2D,b ? renderedTexture : renderTexture2);
		//	glUseProgram(blur_programID);
		//	glUniform1i(blur_texID,0);
		//	glUniform1i(isVertical_ID,(int)b);
		//	glUniform2f(pixelSize_ID, 1.0/WINDOW_WIDTH, 1.0/WINDOW_HEIGHT);
		//	//glUniformMatrix4fv(blur_MatrixID, 1, GL_FALSE, &MVP[0][0]);
		//	//glUniformMatrix4fv(blur_ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		//	//glUniformMatrix4fv(blur_ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		//	quadDraw(quad_vertexbuffer);
		//	b=!b;
		//}	
	


		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);//使用窗口默认帧缓冲
		glViewport(0,0,1024,768); // Render on the whole framebuffer, complete from the lower left corner to the upper right

		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//// Use our shader rendering the quad
		//glUseProgram(quad_programID);

		//// Bind our texture in Texture Unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, renderedTexture);
		////glBindTexture(GL_TEXTURE_2D,b ? renderedTexture : renderTexture2);
		//// Set our "renderedTexture" sampler to user Texture Unit 0
		//glUniform1i(texID, 0);

		//glUniform1f(timeID, (float)(glfwGetTime()*10.0f) );
		glm::vec2 direction;

		for (int i = 0;i<iteration; i++)
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[1]);
			glBindTexture(GL_TEXTURE_2D, fbtex[0]);

			glUseProgram(prog[0]);
			glUniform1i(blurV_texID, 0);
			quadDraw(quad_vertexbuffer);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[0]);
			glBindTexture(GL_TEXTURE_2D, fbtex[1]);

			glUseProgram(prog[1]);
			glUniform1i(blurH_texID, 0);
			quadDraw(quad_vertexbuffer);
			
		}
		//glUniform2f(directionID, direction.x, direction.y);
		//quadDraw(quad_vertexbuffer);
		glUseProgram(0);


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader

	glDeleteProgram(programID);
//	glDeleteTextures(1, &TextureID);

	//glDeleteFramebuffers(1, &FramebufferName);
	//glDeleteTextures(1, &renderedTexture);
	//glDeleteRenderbuffers(1, &depthrenderbuffer);
		glDeleteTextures(2, fbtex);
	glDeleteFramebuffers(2, fbo);

	glDeleteBuffers(1, &quad_vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);


	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

