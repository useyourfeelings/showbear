#include <thirdparty/assimp/include/assimp/scene.h>
#include <thirdparty/assimp/include/assimp/Importer.hpp>

#include <string>

class Model
{
private:
    unsigned int vaoHandle;
    unsigned int element_array_buffer_triangles;
    unsigned int element_array_buffer_triangles_adjacency;
    int faces_;
    int vertices_;

    const aiScene* scene_;
    Assimp::Importer importer_;

public:
    Model(const std::string& pFile);
    ~Model(){}

    bool AssimpImport(const std::string& pFile);

    void Render() const;
    void RenderAsTrianglesAdj() const;
};
