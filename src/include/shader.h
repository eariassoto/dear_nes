// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include <vector>
#include <type_traits>
#include <glm/glm.hpp>

namespace cpuemulator {
class Shader {
   public:
    Shader();
    void Init(const std::string& vertexCode, const std::string& fragmentCode);
    void Use();

    unsigned int GetProgramId() const { return m_ProgramId; }

    template <typename... T>
    void SetUniform(const std::string& name, T... val) {
        constexpr std::size_t argumentLength = sizeof...(val);
        static_assert(argumentLength < 4);  // todo log error

        unsigned int uniformVarLocation =
            glGetUniformLocation(m_ProgramId, name.c_str());
        if constexpr (argumentLength == 1) {
            SetUniform1d(uniformVarLocation, std::forward<T>(val)...);
        } else if constexpr (argumentLength == 2) {
            SetUniform2d(uniformVarLocation, std::forward<T>(val)...);
        } else {
            SetUniform3d(uniformVarLocation, std::forward<T>(val)...);
        }
    }

   private:
    unsigned int m_VertexId;
    unsigned int m_FragmentId;
    unsigned int m_ProgramId;
    std::string m_VertexCodeStr;
    std::string m_FragmentCodeStr;

    void CheckCompileErr();
    void CheckLinkingErr();
    void Compile();
    void Link();

    template <typename T>
    void SetUniform1d(unsigned int uniformVarLocation, T val) {
        if constexpr (std::is_same_v<T, int> || std::is_same_v<T, bool>) {
            glUniform1i(uniformVarLocation, val);
        } else if constexpr (std::is_same_v<T, float>) {
            glUniform1f(uniformVarLocation, val);
        } else if constexpr (std::is_same_v<T, float*> ||
                             std::is_same_v<T, glm::mat4*>) {
            glUniformMatrix4fv(uniformVarLocation, 1, GL_FALSE, val);
        } else {
            static_assert(false);  // todo log error
        }
    }

    template <typename T>
    void SetUniform2d(unsigned int uniformVarLocation, T val1, T val2) {
        if constexpr (std::is_same_v<T, float>) {
            glUniform2f(uniformVarLocation, val1, val2);
        } else {
            static_assert(false);  // todo log error
        }
    }

    template <typename T>
    void SetUniform3d(unsigned int uniformVarLocation, T val1, T val2, T val3) {
        if constexpr (std::is_same_v<T, float>) {
            glUniform3f(uniformVarLocation, val1, val2, val3);
        } else {
            static_assert(false);  // todo log error
        }
    }
};
}  // namespace cpuemulator