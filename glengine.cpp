#include "glengine.h"



void GLEngine::InitializeProgram(std::vector<shaderName> & shaderNames)
{
//	std::string strVertexShader = getFile("v.perspective.shader");	
//	std::string strFragmentShader = getFile("fragment.shader");

	std::vector<GLuint> shaderList;
	std::vector<shaderName>::iterator it;
	for( it = shaderNames.begin(); it != shaderNames.end(); it++) {
		std::string strShader(getFile(it->second.c_str()));
		shaderList.push_back(CreateShader(it->first,strShader));
	}
	
//	shaderList.push_back(CreateShader(GL_VERTEX_SHADER, strVertexShader));
//	shaderList.push_back(CreateShader(GL_FRAGMENT_SHADER, strFragmentShader));
	
	theProgram = CreateProgram(shaderList);
	
	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

//	glUseProgram(0);
	
//Perspective Uniform Matrix.
//	perspectiveMatrixUnif = glGetUniformLocation(theProgram, "perspectiveMatrix");
//	SetPerspective();
}

GLuint GLEngine::CreateShader(GLenum eShaderType, const std::string &strShaderFile) {
	GLuint shader = glCreateShader(eShaderType);
	const char *strFileData = strShaderFile.c_str();
	glShaderSource(shader, 1, &strFileData, NULL);
	
	glCompileShader(shader);
	
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		
		const char *strShaderType = NULL;
		switch(eShaderType) {
			case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
			case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
			case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}
		
		fprintf(stderr, "Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
		delete[] strInfoLog;
	 }
	 return shader;
}

GLuint GLEngine::CreateProgram(const std::vector<GLuint> &shaderList) {
	GLuint program = glCreateProgram();

	for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glAttachShader(program, shaderList[iLoop]);
	
	glLinkProgram(program);
	CheckProgram(program);

	for( size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glDetachShader(program, shaderList[iLoop]);

	return program;
}

void GLEngine::CheckProgram(GLuint & program) {
	GLint status;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}
}
