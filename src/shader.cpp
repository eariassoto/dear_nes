// Copyright (c) 2020 Emmanuel Arias
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "include/shader.h"
#include "include/logger.h"

namespace cpuemulator {

Shader::Shader() {}

void Shader::Init(const std::string& vertexCode,
                  const std::string& fragmentCode) {
    m_VertexCodeStr = vertexCode;
    m_FragmentCodeStr = fragmentCode;
    Compile();
    Link();
}

void Shader::Compile() {
    const char* vcode = m_VertexCodeStr.c_str();
    m_VertexId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_VertexId, 1, &vcode, NULL);
    glCompileShader(m_VertexId);

    const char* fcode = m_FragmentCodeStr.c_str();
    m_FragmentId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_FragmentId, 1, &fcode, NULL);
    glCompileShader(m_FragmentId);
    CheckCompileErr();
}

void Shader::Link() {
    m_ProgramId = glCreateProgram();
    glAttachShader(m_ProgramId, m_VertexId);
    glAttachShader(m_ProgramId, m_FragmentId);
    glLinkProgram(m_ProgramId);
    CheckLinkingErr();
    glDeleteShader(m_VertexId);
    glDeleteShader(m_FragmentId);
}

void Shader::Use() { glUseProgram(m_ProgramId); }

void Shader::CheckCompileErr() {
    int success;
    char infoLog[1024];
    glGetShaderiv(m_VertexId, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(m_VertexId, 1024, NULL, infoLog);
        Logger::Get().Log("SHADER", "Error compiling Vertex Shader:\n {}", infoLog);
    }
    glGetShaderiv(m_FragmentId, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(m_FragmentId, 1024, NULL, infoLog);
        Logger::Get().Log("SHADER", "Error compiling Fragment Shader:\n {}",
                          infoLog);
    }
}

void Shader::CheckLinkingErr() {
    int success;
    char infoLog[1024];
    glGetProgramiv(m_ProgramId, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_ProgramId, 1024, NULL, infoLog);
        Logger::Get().Log("SHADER", "Error Linking Shader Program:\n {}",
                          infoLog);
    }
}
}  // namespace cpuemulator