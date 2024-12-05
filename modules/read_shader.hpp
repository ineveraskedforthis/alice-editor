#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

inline const std::string read_shader(const std::string path) {
    std::string shader_source;
    std::ifstream shader_file;

    shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        shader_file.open(path);
        std::stringstream shader_source_stream;
        shader_source_stream << shader_file.rdbuf();
        shader_file.close();
        shader_source = shader_source_stream.str();
    } catch (std::ifstream::failure& e)
    {
        throw std::runtime_error(e);
    }

    const char * result = shader_source.c_str();

    return result;
}