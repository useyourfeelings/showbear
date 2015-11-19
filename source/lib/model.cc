#include <model.h>
#include <iostream>
using namespace std;

#include <thirdparty/assimp/include/assimp/postprocess.h>
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

Model::Model(const std::string& pFile)
{
    cout<<"Model::Model()\n";
    bool result = this->AssimpImport(pFile);
    cout<<"result = "<<result<<endl;;
    assert(result);
    cout<<"Model::AssimpImport() ok\n";
    //faces_ = sides * rings;
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
    // Verts
    float * v = new float[6 * vertices_];
    // Normals
    float * n = new float[3 * vertices_];
    // Tex coords
    float * tex = new float[2 * vertices_];
    // Elements
    unsigned int * el = new unsigned int[faces_ * 3];

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
            el[i] = scene_->mMeshes[0]->mFaces[face].mIndices[j];
            ++ i;
        }
    }

    cout<<endl<<"total mNumIndices = "<<i<<endl;
/*
    for(int i = 0; i < vertices_; ++ i)
    {
        cout<<"#"<<i<<" "<<v[i * 3]<<" "<<v[i * 3 +1]<<" "<<v[i * 3 + 2]<<endl;
    }
    cout<<endl;
    for(int i = 0; i < vertices_; ++ i)
    {
        cout<<"#"<<i<<" "<<n[i * 3]<<" "<<n[i * 3 +1]<<" "<<n[i * 3 + 2]<<endl;
    }
    cout<<endl;
    for(int i = 0; i < faces_ * 3; ++ i)
    {
        cout<<el[i]<<" ";
    }
    cout<<endl;
*/
    // Create and populate the buffer objects
    unsigned int handle[2];
    glGenBuffers(2, handle);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, (6 * vertices_) * sizeof(float), v, GL_STATIC_DRAW);

    //glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    //glBufferData(GL_ARRAY_BUFFER, (3 * vertices_) * sizeof(float), n, GL_STATIC_DRAW);

    //glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
    //glBufferData(GL_ARRAY_BUFFER, (2 * vertices_) * sizeof(float), tex, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * faces_ * sizeof(unsigned int), el, GL_STATIC_DRAW);

    // Create the VAO
    glGenVertexArrays( 1, &vaoHandle );
    glBindVertexArray(vaoHandle);

    glEnableVertexAttribArray(0);  // Vertex position
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), ((GLubyte *)0 + (0)) );

    glEnableVertexAttribArray(1);  // Vertex normal
    glVertexAttribPointer( (GLuint)1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), ((GLubyte *)0 + (3*sizeof(float))) );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[1]);

    glBindVertexArray(0);

    delete [] v;
    delete [] n;
    delete [] el;
    delete [] tex;
}

void Model::Render() const {
    glBindVertexArray(vaoHandle);
    glDrawElements(GL_TRIANGLES, 3 * faces_, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
}
