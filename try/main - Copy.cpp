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
#include <glm/gtc/matrix_transform.hpp>
#define SCENEMODEL "sponza.obj"
#define OBJMODEL "Wooden Box.obj"
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#include <math.h>


Mesh* mainMesh;
Mesh* objMesh[5];
oldCamera* mainCamera;
GLuint transparencyID;
GLuint vblurFactorID;
GLuint hblurFactorID;
float blurFactor = 0.1;
//double eyeXPos_old = WINDOW_WIDTH/2;
//double eyeYPos_old = WINDOW_HEIGHT/2;
float contrast = 1;
float brightness = 1;
float saturation = 1;

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


GLuint createTexture(int w,int h, bool depth)
{
	unsigned int textureId;
	glGenTextures(1,&textureId);
	glBindTexture(GL_TEXTURE_2D,textureId);
	if(depth){
		glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT24, 1024, 768, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	}else{
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
	}
	
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

glm::vec2 calculateWindowSpacePos(glm::mat4 mP, glm::mat4 mV, glm::vec3 pointPos){
	glm::vec4 clipSpacePos = mP * mV * glm::vec4(pointPos, 1.0);
		//printf("clip Space value is:  %f,  %f, %f, %f \n", clipSpacePos.x, clipSpacePos.y, clipSpacePos.z, clipSpacePos.w);
	if(clipSpacePos.w != 0){
		glm::vec3 NDCSpacePos = glm::vec3( clipSpacePos.x / clipSpacePos.w, clipSpacePos.y / clipSpacePos.w, clipSpacePos.z / clipSpacePos.w);
		//printf("NDC Space value is:  %f, %f, %f \n", NDCSpacePos.x, NDCSpacePos.y, NDCSpacePos.z);
		glm::vec2 viewSize = glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT);
		glm::vec2 viewOffSet = glm::vec2(0.0);
		//glm::vec2 windowSpacePos = glm::vec2((NDCSpacePos.x+1.0)/2.0, (NDCSpacePos.y+1.0)/2.0) * glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT)+glm::vec2(0.0);
		glm::vec2 windowSpacePos = glm::vec2 ( ((NDCSpacePos.x + 1.0) / 2.0) * viewSize.x + viewOffSet.x, ((NDCSpacePos.y + 1.0) / 2.0) * viewSize.y + viewOffSet.y);
		return windowSpacePos;
	}
	return glm::vec2(1.0);
}


glm::vec3 convertToWorldCoord(glm::mat4 mP, glm::mat4 mV, glm::mat4 mM){
		double eyeXPos_current, eyeYPos_current;//mouse position on the screen space
		glfwGetCursorPos(window, &eyeXPos_current, &eyeYPos_current);
		float eye_current_z;
		glReadPixels(eyeXPos_current, eyeYPos_current, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &eye_current_z);
		glm::vec3 windowCoordinates = glm::vec3(eyeXPos_current, eyeYPos_current, eye_current_z);
		glm::vec4 viewport = glm::vec4(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT);
		glm::vec3 worldCoordinates = glm::unProject(windowCoordinates, mV*mM, mP, viewport);
		//printf("(%f, %f, %f)\n", worldCoordinates.x, worldCoordinates.y, worldCoordinates.z);
		
		return worldCoordinates;

}

void keyEventHandle(){
	
	if (glfwGetKey(window, GLFW_KEY_V ) == GLFW_PRESS)
	{
		blurFactor -=0.001;
		if (blurFactor <= 0.05)
		{
			blurFactor = 0.05;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_B ) == GLFW_PRESS)
	{
		blurFactor +=0.001;
		if (blurFactor >= 1)
		{
			blurFactor = 1;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_Q ) == GLFW_PRESS)
	{
		brightness +=0.01;
		if (brightness >= 2)
		{
			brightness = 2;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_W ) == GLFW_PRESS)
	{
		brightness -=0.005;
		if (brightness <= 0.5)
		{
			brightness = 0.5;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_Z ) == GLFW_PRESS)
	{
		contrast +=0.005;
		if (contrast >= 1)
		{
			contrast = 1;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_X ) == GLFW_PRESS)
	{
		contrast -= 0.005;
		if (contrast <= 0.3)
		{
			contrast = 0.3;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_A ) == GLFW_PRESS)
	{
		saturation += 0.005;
		if (saturation >= 2)
		{
			saturation =2;
		}
		//printf("satuation is: %f \n", saturation);
	}
	if (glfwGetKey(window, GLFW_KEY_S ) == GLFW_PRESS)
	{
		saturation -= 0.005;
		if (saturation <= 0)
		{
			saturation = 0;
		}
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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	mainCamera = new oldCamera();

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

#pragma region //set up shaders
	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShadingRTT.vertexshader", "StandardShadingRTT.fragmentshader" );
	GLuint ProjectMatrixID = glGetUniformLocation(programID, "P");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	GLuint transparencyID = glGetUniformLocation(programID, "transparency");
	GLuint fadeInCue = glGetUniformLocation(programID, "fadeIn");
	//GLuint AttractorCue = glGetUniformLocation(programID, "attractor");

	GLuint falseColor_programID = LoadShaders( "falseColor.vertexshader", "falseColor.fragmentshader" );
	GLuint falseMatrixID = glGetUniformLocation(falseColor_programID, "MVP");

	GLuint H_quad_programID = LoadShaders( "HBlur.vs", "WobblyTexture.fragmentshader" );
	GLuint h_texID = glGetUniformLocation(H_quad_programID, "renderedTexture");
	GLuint false_h_texID = glGetUniformLocation(H_quad_programID, "falseColorTexture");
	hblurFactorID = glGetUniformLocation(H_quad_programID, "blurFactor");
	GLuint h_contrastID = glGetUniformLocation(H_quad_programID, "contrast");
	GLuint h_brightnessID = glGetUniformLocation(H_quad_programID, "brightness");
	GLuint h_saturationID = glGetUniformLocation(H_quad_programID, "saturation");

	GLuint V_quad_programID = LoadShaders( "VBlur.vs", "WobblyTexture.fragmentshader" );
	GLuint v_texID = glGetUniformLocation(V_quad_programID, "renderedTexture");
	GLuint false_v_texID = glGetUniformLocation(V_quad_programID, "falseColorTexture");
	vblurFactorID = glGetUniformLocation(V_quad_programID, "blurFactor");
	GLuint edgeCueID = glGetUniformLocation(V_quad_programID, "edgeCue");
	GLuint v_contrastID = glGetUniformLocation(V_quad_programID, "contrast");
	GLuint v_brightnessID = glGetUniformLocation(V_quad_programID, "brightness");
	GLuint v_saturationID = glGetUniformLocation(V_quad_programID, "saturation");

#pragma endregion
	
	// Read our .obj file
	mainMesh = new Mesh();
	//bool mainMeshLoadSuccess = mainMesh->LoadMesh(SCENEMODEL);
	
	glm::vec3 objPos[5];
	objPos[0] = glm::vec3(-30,30,10);
	objPos[1] = glm::vec3(30,30,10);
	objPos[2] = glm::vec3(-30,60,10);
	objPos[3] = glm::vec3(30,60,10);
	objPos[4] = glm::vec3(0,45,10);
	for(int i = 0; i <5; i++)
	{
		objMesh[i] = new Mesh();
		objMesh[i]->centerPos = objPos[i];
		objMesh[i]->LoadMesh(OBJMODEL);
	}
	int defaultCue = rand() % 5; 
	objMesh[defaultCue]->attractor = true;

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");


	// ---------------------------------------------
	// Render to Texture
	// ---------------------------------------------
#pragma region //放置经过光照后整个画面的FBO
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	GLuint renderedTexture = createTexture(WINDOW_WIDTH, WINDOW_HEIGHT,false);
	// The depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
	GLuint depthTexture = createTexture(WINDOW_WIDTH, WINDOW_HEIGHT,true);;
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
#pragma endregion

#pragma region //放置经过水平+竖直高斯模糊后的texture
	GLuint FramebufferName2 = 0;
	glGenFramebuffers(1, &FramebufferName2);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName2);
	GLuint renderedTexture2 = createTexture(WINDOW_WIDTH, WINDOW_HEIGHT,false);//second pass rendering-vertical
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture2, 0);
	//GLenum DrawBuffers2[1] = {GL_COLOR_ATTACHMENT0};
	//glDrawBuffers(1, DrawBuffers2); // "1" is the size of DrawBuffers
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
#pragma endregion

#pragma region //放置对cue obj的false color rendering texture
	GLuint FramebufferName3 = 0;
	glGenFramebuffers(1, &FramebufferName3);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName3);
	GLuint renderedTexture3 = createTexture(WINDOW_WIDTH, WINDOW_HEIGHT,false);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture3, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
#pragma endregion

#pragma region //设置将texture粘贴到屏幕的画板
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
#pragma endregion

	float transparency = 0;
	bool fadeOut = false;
	float countUnknownTime = 0;
	do{

		keyEventHandle();

#pragma region //设置闪烁效果
		if(transparency >= 1 ){
			fadeOut = true;
		}
		
		if(fadeOut)
		{
			transparency -= 0.007;
			if(transparency <= 0.5 )
			{
				fadeOut = false;
			}
		}else{
			transparency += 0.007;
		}
#pragma endregion

#pragma region		//1-pass:将整个场景的光照渲染结果存储到fbo1的texture
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);//enable the framebuffer
		glViewport(0,0,1024,768); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		// Clear the frame buffer, the render the scene (the scene is on the frame buffer, basically render texture if it's attached in the screen buffer)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram(programID);

		glm::vec3 lightPos = glm::vec3(10,10,10);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform1f(transparencyID, transparency);

		// Compute the MVP matrix from keyboard and mouse input
		mainCamera->computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = mainCamera->getProjectionMatrix();
		glm::mat4 ViewMatrix = mainCamera->getViewMatrix();
		glUniformMatrix4fv(ProjectMatrixID, 1, GL_FALSE, &ProjectionMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniform1i(fadeInCue, false);
		//glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 SceneModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(0.2,0.2,0.2));
		SceneModelMatrix = SceneModelMatrix * glm::rotate(90.0f, glm::vec3(0,1,0));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &SceneModelMatrix[0][0]);
		mainMesh->Render();

		glm::mat4 objModelMatrix[5];
		for(int i = 0; i<5; i++){
			objModelMatrix[i] = glm::scale(glm::translate(glm::mat4(1.0), objMesh[i]->centerPos), glm::vec3(2.5,2.5,2.5));
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &objModelMatrix[i][0][0]);
			glUniform1i(fadeInCue, objMesh[i]->fadeIn);
			objMesh[i]->Render();
		}
		double eyeXPos_current, eyeYPos_current;//mouse position on the screen space
		glfwGetCursorPos(window, &eyeXPos_current, &eyeYPos_current);
		eyeYPos_current =  WINDOW_HEIGHT-eyeYPos_current;
		objMesh[defaultCue]->screenPos = calculateWindowSpacePos(ProjectionMatrix, ViewMatrix, objMesh[defaultCue]->centerPos);
		//printf("cue object position is: %f, %f \n", objMesh[defaultCue]->screenPos.x, objMesh[defaultCue]->screenPos.y);
		//printf("Mouse position is: %f, %f \n", eyeXPos_current, eyeYPos_current);
		glUseProgram(0);
		glDisable(GL_BLEND);
#pragma endregion

#pragma region //cue的false color rendering
		glUseProgram(falseColor_programID);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName3);
		glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		

		for(int i = 0; i<5; i++){
			if(objMesh[i]->attractor){
				glm::vec2 distToObj = glm::vec2(eyeXPos_current-objMesh[i]->screenPos.x, eyeYPos_current-objMesh[i]->screenPos.y);
				//判断眼睛是否触碰到物体
				float dist_squared = glm::dot(distToObj, distToObj);
				if(dist_squared < 40000){
					objMesh[i]->attractor = false;
					objMesh[i]->fadeIn = false;
					objMesh[i]->edge = false;
					defaultCue = (defaultCue + (rand() % 5 + 1)) % 5; 
					objMesh[defaultCue]->attractor = true;
					countUnknownTime = 0;
				}else{
					countUnknownTime += 1;
				}
				if(countUnknownTime >= 150){
					objMesh[i]->fadeIn = true;
				}
				if(countUnknownTime >= 300){
					objMesh[i]->edge = true;
				}
				glm::mat4 MVP = ProjectionMatrix * ViewMatrix * objModelMatrix[i];
				glUniformMatrix4fv(falseMatrixID, 1, GL_FALSE, &MVP[0][0]);
				objMesh[i]->Render();
			}
		}
		//printf("frame count is: %d \n", countUnknownTime);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		glUseProgram(0);
#pragma endregion

#pragma region			//水平方向的高斯模糊处理
		//输入为整个场景的光照渲染结果,输出为经过水平方向高斯模糊后的texture
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName2);
		glViewport(0,0,1024,768); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(H_quad_programID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderedTexture);
		glUniform1i(h_texID, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderedTexture3);//false color texture
		glUniform1i(false_h_texID, 1);

		glUniform1f(hblurFactorID, blurFactor);
		glUniform1f(h_contrastID, contrast);
		glUniform1f(h_brightnessID, brightness);
		glUniform1f(h_saturationID, saturation);
		quadDraw(quad_vertexbuffer);
		glUseProgram(0);
#pragma endregion

#pragma region	//竖直方向的高斯模糊处理,并将最后处理过的texture显示到屏幕上
		glBindFramebuffer(GL_FRAMEBUFFER, 0);//使用窗口默认帧缓冲
		glViewport(0,0,1024,768); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(V_quad_programID);
		// Bind our scene texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderedTexture2);
		glUniform1i(v_texID, 0);
		// Bind our false color texture in Texture Unit 1
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderedTexture3);//false color texture
		glUniform1i(false_h_texID, 1);
		glUniform1f(vblurFactorID, blurFactor);
		glUniform1f(v_contrastID, contrast);
		glUniform1f(v_brightnessID, brightness);
		glUniform1f(v_saturationID, saturation);
		for(int i = 0; i<5; i++){
			if(objMesh[i]->attractor){
				glUniform1i(edgeCueID, objMesh[i]->edge);
			}
		}
		quadDraw(quad_vertexbuffer);
		glUseProgram(0);
#pragma endregion

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader

	glDeleteProgram(programID);

	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteFramebuffers(1, &FramebufferName2);
	glDeleteFramebuffers(1, &FramebufferName3);
	glDeleteTextures(1, &renderedTexture);
	glDeleteTextures(1, &renderedTexture2);
	glDeleteTextures(1, &renderedTexture3);
	glDeleteRenderbuffers(1, &depthrenderbuffer);
	glDeleteBuffers(1, &quad_vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);


	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

