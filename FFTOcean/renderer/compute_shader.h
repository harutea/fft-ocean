#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H

#include <string>
#include <glm/glm.hpp>

namespace FFTOcean
{
    class ComputeShader
    {
    public:
        ComputeShader(const char *compShaderPath);
        ~ComputeShader();
        void use();
        void setBool(const std::string &name, bool value) const;
        void setInt(const std::string &name, int value) const;
        void setFloat(const std::string &name, float value) const;
        void setVec3(const std::string &name, const glm::vec3 &value) const;
        void setVec3(const std::string &name, float x, float y, float z) const;
        unsigned int getID() const;

    private:
        unsigned int ID;
        void checkCompileErrors(unsigned int shader, std::string type);
    };
}

#endif COMPUTE_SHADER_H