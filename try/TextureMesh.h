#pragma once
#include <string>

#include <GL/glew.h>
#include "Magick++.h"
#include <SOIL.h>

class Texture
{
public:
	Texture(GLenum TextureTarget, const std::string& FileName);
	GLuint loadTexture(const char* path);
	void UseDefaultTexture(GLuint textureID);

	bool Load();

	void Bind(GLenum TextureUnit);

private:
	std::string m_fileName;
	GLenum m_textureTarget;
	GLuint m_textureObj;
	Magick::Image m_image;
	Magick::Blob m_blob;
};

