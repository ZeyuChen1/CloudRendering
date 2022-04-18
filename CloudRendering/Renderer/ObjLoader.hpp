#pragma once

#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <simd/simd.h>
#include <sstream>
#include <filesystem>

#include "SharedTypes.h"

using namespace std;

static vector<string> split(const string& s, const char delim) {
    vector<string> result;
    stringstream ss{ s };
    string item;
    while ( getline(ss, item, delim) ) {
        result.push_back(item);
    }
    return result;
}

class ObjLoader {
private:
    ifstream ifs;
    vector<vertex_t> vertices;
    vector<uint32_t> indices;
    
    void parse_file() {
        string line;
        size_t uv_index = 0;
        size_t normal_index = 0;
        
        while ( getline(ifs, line, '\n') ) {
            vector<string> line_splited = split(line, ' ');
            
            if ( line_splited.size() == 0 ) continue;
            
            if ( line_splited[0] == "v" ) {
                vertex_t v;
                v.position = simd::make_float3(
                    stof(line_splited[1]), stof(line_splited[2]), stof(line_splited[3]));
                vertices.push_back(v);
                
            } else if ( line_splited[0] == "vt" ) {
                simd::float2 uv = simd::make_float2( stof(line_splited[1]), stof(line_splited[2]) );
                vertices[uv_index].uv = uv;
                uv_index += 1;
                
            } else if ( line_splited[0] == "f" ) {
                for ( int i = 1; i <= 3; i += 1 )
                indices.push_back(stoi(line_splited[i]) - 1);
                
            } else if ( line_splited[0] == "vn" ) {
                simd::float3 normal = simd::make_float3(
                    stof(line_splited[1]), stof(line_splited[2]), stof(line_splited[3]) );
                vertices[normal_index].normal = normal;
                normal_index += 1;
            }
        }
    }
    
public:
    explicit ObjLoader( string& file_path ) : ifs( ifstream() ) {
        ifs.open(file_path);
        
        if ( ifs.fail() ) {
            cerr << "cwd: " << filesystem::current_path() << endl;
            cerr << "Cannot open file \"" << file_path << "\": "<< strerror(errno) << endl;
            exit(1);
        }
        
        parse_file();
    }
    
    void print_loaded_vertices() {
        for (vertex_t& v : vertices) {
            simd::float3 position = v.position;
            cout << "(" << position[0] << ", " << position[1] << ", " << position[2] << ") ";
        }
        cout << endl;
    }
    
    // vertices
    inline size_t get_vertices_data_size() { return vertices.size() * sizeof(vertex_t); }
    
    inline vertex_t* get_vertices_data() { return vertices.data(); }
    
    inline size_t get_vertices_count() { return vertices.size(); }
    
    // indices
    inline size_t get_indices_data_size() { return indices.size() * sizeof(uint32_t); }
    
    inline uint32_t* get_indices_data() { return indices.data(); }
    
    inline size_t get_indices_count() { return indices.size(); }
};
