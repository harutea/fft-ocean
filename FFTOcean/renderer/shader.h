#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glm/glm.hpp>

namespace FFTOcean
{
    class Shader
    {
    public:
        Shader(const char *vertexShaderPath, const char *fragmentShaderPath);
        ~Shader();
        void use();
        void setBool(const std::string &name, bool value) const;
        void setInt(const std::string &name, int value) const;
        void setFloat(const std::string &name, float value) const;
        void setVec3(const std::string &name, const glm::vec3 &value) const;
        void setVec3(const std::string &name, float x, float y, float z) const;
        void setMat4(const std::string &name, const glm::mat4 &value) const;
        void setMat3(const std::string &name, const glm::mat3 &value) const;
        unsigned int getID() const;

    private:
        unsigned int ID;
    };
}

#endif SHADER_H