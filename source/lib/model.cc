#include <model.h>
#include <iostream>
#include <vector>
#include <unordered_map>
using namespace std;

#include <thirdparty/assimp/include/assimp/postprocess.h>
#include <thirdparty/assimp/code/VertexTriangleAdjacency.h>

#include <thirdparty/glad/glad.h>

bool Model::AssimpImport(const std::string& pFile)
{
    cout<<"Model::AssimpImport()\n";
    // Create an instance of the Importer class
    //Assimp::Importer importer;
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // propably to request more postprocessing than we do in this example.
    scene_ = importer_.ReadFile( pFile,
        aiProcess_CalcTangentSpace       |
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);

    // If the import failed, report it
    if(!scene_)
    {
        cout<< importer_.GetErrorString()<<endl;;
        return false;
    }
    // Now we can access the file's contents.
    //DoTheSceneProcessing( scene);
    // We're done. Everything will be cleaned up by the importer destructor
    return true;
}

class Edge
{
private:
    aiVector3D a_;
    aiVector3D b_;
public:
    Edge(const aiVector3D& a, const aiVector3D& b):a_(a),b_(b){}

    bool operator==(const Edge &other) const
    {
        return a_ == other.a_ && b_ == other.b_;
    }

    std::size_t GetHash() const
    {
        std::size_t h1 = std::hash<float>()(a_.x);
        std::size_t h2 = std::hash<float>()(a_.y);
        std::size_t h3 = std::hash<float>()(a_.z);
        std::size_t h4 = std::hash<float>()(b_.x);
        std::size_t h5 = std::hash<float>()(b_.y);
        std::size_t h6 = std::hash<float>()(b_.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5);
    }
};

class EdgeHash
{
public:
    std::size_t operator()(Edge const& e) const
    {
        return e.GetHash();
    }
};

Model::Model(const std::string& pFile)
{
    cout<<"Model::Model()\n";
    bool result = this->AssimpImport(pFile);
    cout<<"result = "<<result<<endl;;
    assert(result);
    cout<<"Model::AssimpImport() ok\n";
    cout<<"mNumMeshes = "<<scene_->mNumMeshes<<endl;
    cout<<"mNumCameras = "<<scene_->mNumCameras<<endl;
    cout<<"mNumMaterials = "<<scene_->mNumMaterials<<endl;
    cout<<"mNumTextures = "<<scene_->mNumTextures<<endl;
    //cout<<"mPrimitiveTypes = "<<scene_->mMeshes[0]-><<endl;
    cout<<"scene_->mMeshes[0]->mName = "<<scene_->mMeshes[0]->mName.C_Str()<<endl;
    faces_ = (scene_->mMeshes[0])->mNumFaces;
    cout<<"mNumFaces = "<<faces_<<endl;

    vertices_ = scene_->mMeshes[0]->mNumVertices;//sides * (rings+1);   // One extra ring to duplicate first ring
    cout<<"mNumVertices = "<<vertices_<<endl;
    //cout<<scene_->mMeshes[0]->mNormals.

    // position
    float * v = new float[6 * vertices_];
    // normal
    float * n = new float[3 * vertices_];
    // Tex coords
    float * tex = new float[2 * vertices_];
    // Elements
    unsigned int * triangles_elements = new unsigned int[faces_ * 3];

    for(int i = 0; i < vertices_; ++ i)
    {
        v[i * 6]     = scene_->mMeshes[0]->mVertices[i].x;
        v[i * 6 + 1] = scene_->mMeshes[0]->mVertices[i].y;
        v[i * 6 + 2] = scene_->mMeshes[0]->mVertices[i].z;

        v[i * 6 + 3] = scene_->mMeshes[0]->mNormals[i].x;
        v[i * 6 + 4] = scene_->mMeshes[0]->mNormals[i].y;
        v[i * 6 + 5] = scene_->mMeshes[0]->mNormals[i].z;
    }

    int face = 0, i = 0;
    for(face = 0, i = 0; face < faces_; ++ face)
    {
        //cout<<"face #"<<face<<" has "<<scene_->mMeshes[0]->mFaces[face].mNumIndices<<" indices"<<endl;

        for(int j = 0; j < scene_->mMeshes[0]->mFaces[face].mNumIndices; ++ j)
        {
            //cout<<"Index # "<<j<<" = "<<scene_->mMeshes[0]->mFaces[face].mIndices[j]<<endl;
            triangles_elements[i] = scene_->mMeshes[0]->mFaces[face].mIndices[j];
            ++ i;
        }
    }

    cout<<endl<<"total mNumIndices = "<<i<<endl;

    // Generate index buffer with adjacency information, in a brutal way.
    // This is used in the shadow volume application.
    unordered_map<Edge, vector<unsigned int>, EdgeHash> edge_face_map;

    for(face = 0; face < faces_; ++ face)
    {
        for(int j = 0; j < scene_->mMeshes[0]->mFaces[face].mNumIndices; ++ j) // must be triangulated (3 vertices for a face)
        {
            unsigned int current = scene_->mMeshes[0]->mFaces[face].mIndices[j];

            unsigned int next = 0;
            if((j + 1) == scene_->mMeshes[0]->mFaces[face].mNumIndices)
                next = scene_->mMeshes[0]->mFaces[face].mIndices[0];
            else
                next = scene_->mMeshes[0]->mFaces[face].mIndices[j + 1];

            Edge edge(scene_->mMeshes[0]->mVertices[current], scene_->mMeshes[0]->mVertices[next]);
            /*cout<<"edge a_ = "
            <<scene_->mMeshes[0]->mVertices[current].x<<" "
            <<scene_->mMeshes[0]->mVertices[current].y<<" "
            <<scene_->mMeshes[0]->mVertices[current].z<<" b_ = "
            <<scene_->mMeshes[0]->mVertices[next].x<<" "
            <<scene_->mMeshes[0]->mVertices[next].y<<" "
            <<scene_->mMeshes[0]->mVertices[next].z<<endl;*/

            auto i = edge_face_map.find(edge);
            if(i == edge_face_map.end())
            {
                Edge edge_reverse(scene_->mMeshes[0]->mVertices[next], scene_->mMeshes[0]->mVertices[current]);

                auto k = edge_face_map.find(edge_reverse);
                if(k == edge_face_map.end())
                {
                    vector<unsigned int> faces;
                    faces.push_back(face);
                    edge_face_map[edge] = faces;
                }
                else
                {
                    edge_face_map[edge_reverse].push_back(face);
                }
            }
            else
            {
                edge_face_map[edge].push_back(face);
            }
        }
    }

#if 0
    cout<<"edge_face_map.size() = "<<edge_face_map.size()<<endl;
    for(auto i = edge_face_map.begin(); i != edge_face_map.end(); ++ i)
    {
        for(int j = 0; j < i->second.size(); ++ j)
        {
            cout<<i->second[j]<<" ";
        }
        cout<<endl;
    }
#endif

    unsigned int * triangles_adjacency_elements = new unsigned int[faces_ * 6];
    for(face = 0, i = 0; face < faces_; ++ face)
    {
        //cout<<"face #"<<face<<" : ";

        for(int j = 0; j < scene_->mMeshes[0]->mFaces[face].mNumIndices; ++ j) // assume 3 vertices for a face
        {
            //cout<<"Index # "<<j<<" = "<<scene_->mMeshes[0]->mFaces[face].mIndices[j]<<endl;
            unsigned int current = scene_->mMeshes[0]->mFaces[face].mIndices[j];
            triangles_adjacency_elements[i] = current;
            ++ i;

            //cout<<" add "<<current;

            unsigned int next = 0;
            if((j + 1) == scene_->mMeshes[0]->mFaces[face].mNumIndices)
                next = scene_->mMeshes[0]->mFaces[face].mIndices[0];
            else
                next = scene_->mMeshes[0]->mFaces[face].mIndices[j + 1];

            Edge edge(scene_->mMeshes[0]->mVertices[current], scene_->mMeshes[0]->mVertices[next]);

            auto k = edge_face_map.find(edge);
            if(k == edge_face_map.end())
            {
                Edge edge_reverse(scene_->mMeshes[0]->mVertices[next], scene_->mMeshes[0]->mVertices[current]);
                k = edge_face_map.find(edge_reverse);
            }

            auto faces = k->second;

            for(int g = 0; g < faces.size(); ++ g) // size should be 2
            {
                if(faces[g] != face)
                {
                    for(int r = 0; r < scene_->mMeshes[0]->mFaces[faces[g]].mNumIndices; ++ r)
                    {
                        unsigned int index = scene_->mMeshes[0]->mFaces[faces[g]].mIndices[r];
                        if(scene_->mMeshes[0]->mVertices[index] != scene_->mMeshes[0]->mVertices[next] &&
                            scene_->mMeshes[0]->mVertices[index] != scene_->mMeshes[0]->mVertices[current])
                        {
                            triangles_adjacency_elements[i] = index;
                            ++ i;
                            //cout<<" "<<index<<" ";
                            break;
                        }
                    }
                    break;
                }
            }
        }
        //cout<<endl;
    }

#if 0
    cout<<i<<endl;
    for(int ii = 0 ; ii < i; ++ ii)
        cout<<triangles_adjacency_elements[ii]<<" ";
    cout<<endl;
#endif

    // Create and populate the buffer objects
    unsigned int handle[3];
    glGenBuffers(3, handle);

    // vertex position + vertex normal
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, (6 * vertices_) * sizeof(float), v, GL_STATIC_DRAW);

    element_array_buffer_triangles = handle[1];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_triangles);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * faces_ * sizeof(unsigned int), triangles_elements, GL_STATIC_DRAW);

    element_array_buffer_triangles_adjacency = handle[2];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_triangles_adjacency);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * faces_ * sizeof(unsigned int), triangles_adjacency_elements, GL_STATIC_DRAW);

    // Create the VAO
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);
    cout<<"vaoHandle = "<<vaoHandle<<endl;

    glEnableVertexAttribArray(0);  // Vertex position
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), ((GLubyte *)0 + (0)));

    glEnableVertexAttribArray(1);  // Vertex normal

    // this will raise a warning: usage warning: generic vertex attribute array 1 uses a pointer with a small value .........
    // why?
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), ((GLubyte *)0 + (3 * sizeof(float))));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_triangles);

    glBindVertexArray(0);

    delete [] v;
    delete [] n;
    delete [] tex;
    delete [] triangles_elements;
    delete [] triangles_adjacency_elements;
}

void Model::Render() const
{
    glBindVertexArray(vaoHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_triangles);
    glDrawElements(GL_TRIANGLES, 3 * faces_, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
}

void Model::RenderAsTrianglesAdj() const
{
    glBindVertexArray(vaoHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_triangles_adjacency);
    glDrawElements(GL_TRIANGLES_ADJACENCY, 6 * faces_, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
}
