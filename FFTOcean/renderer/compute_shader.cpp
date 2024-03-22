#include "compute_shader.h"
#include <glad/gl.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace FFTOcean;
using namespace std;

ComputeShader::ComputeShader(const char* compShaderPath) {
    string compShaderSource;
    ifstream compShaderFile;

    compShaderFile.exceptions(ifstream::failbit | ifstream::badbit);

    try
    {
        compShaderFile.open(compShaderPath);
        stringstream compShaderStream, fragmentShaderStream;
        compShaderStream << compShaderFile.rdbuf();
        compShaderFile.close();

        compShaderSource = compShaderStream.str();
    }
    catch (ifstream::failure e)
    {
        cout << "[Error] : File is not succesfully read." << endl;
    }
    const char *compShaderRaw = compShaderSource.c_str();

    unsigned int compute;
    // compute shader
    compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &compShaderRaw, NULL);
    glCompileShader(compute);
    checkCompileErrors(compute, "COMPUTE");

    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, compute);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
}

ComputeShader::~ComputeShader()
{
    glDeleteProgram(this->ID);
}

void ComputeShader::use()
{
    glUseProgram(this->ID);
}

void ComputeShader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(this->ID, name.c_str()), (int)value);
}

void ComputeShader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(this->ID, name.c_str()), value);
}

void ComputeShader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(this->ID, name.c_str()), value);
}

void ComputeShader::setVec3(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(this->ID, name.c_str()), 1, &value[0]);
}

void ComputeShader::setVec3(const std::string &name, float x, float y, float z) const
{
    glUniform3fv(glGetUniformLocation(this->ID, name.c_str()), 1, &glm::vec3(x, y, z)[0]);
}

unsigned int ComputeShader::getID() const
{
    return this->ID;
}

void ComputeShader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            cout << "[ERROR] : Compute Shader Compilation Failed, type: " << type << "\n" << infoLog << "\n----\n";
        }
        return;
    }

    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        cout << "[ERROR] : Compute Shader Compilation Failed, type: " << type << "\n" << infoLog << "\n----\n";
    }
}