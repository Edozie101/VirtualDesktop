//
//  Model.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 11/28/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "Model.h"

#include "SimpleWorldRendererPlugin.h"

#ifdef __APPLE__
#include <boost/filesystem.hpp>
#else
#include "Shlwapi.h"
#endif

Model::Model() :
	vertexLoc(0),
	normalLoc(1),
	texCoordLoc(2),
	matricesUniLoc(1),
	materialUniLoc(2),
	texUnit(0),
	modelShaderProgram(),
    ModelUniformLocations(),
    ModelAttribLocations()
{
	memset(ModelUniformLocations,0,sizeof(ModelUniformLocations));
	memset(ModelAttribLocations,0,sizeof(ModelAttribLocations));
}
GLuint Model::setupShaders() {
    modelShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/standard.v.glsl", "/resources/shaders/standard.f.glsl");
    
    GLuint p = modelShaderProgram.shader.program;
    
	glBindFragDataLocation(p, 0, "color");
    
	glBindAttribLocation(p,vertexLoc,"vertexPosition_modelspace");
	glBindAttribLocation(p,normalLoc,"vertexNormal_modelspace");
	glBindAttribLocation(p,texCoordLoc,"vertexUV");
    
	glLinkProgram(p);
	glValidateProgram(p);
    
	GLuint k = glGetUniformBlockIndex(p,"Matrices");
	glUniformBlockBinding(p, k, matricesUniLoc);
	glUniformBlockBinding(p, glGetUniformBlockIndex(p,"Material"), materialUniLoc);
    
	texUnit = glGetUniformLocation(p,"textureIn");
    
    
    ModelUniformLocations[0] = glGetUniformLocation(modelShaderProgram.shader.program, "MVP");
    ModelUniformLocations[1] = glGetUniformLocation(modelShaderProgram.shader.program, "V");
    ModelUniformLocations[2] = glGetUniformLocation(modelShaderProgram.shader.program, "M");
    ModelUniformLocations[3] = glGetUniformLocation(modelShaderProgram.shader.program, "textureIn");
    ModelUniformLocations[4] = glGetUniformLocation(modelShaderProgram.shader.program, "MV");
    ModelUniformLocations[5] = glGetUniformLocation(groundShaderProgram.shader.program, "LightPosition_worldspace");
    
    ModelAttribLocations[0] = glGetAttribLocation(modelShaderProgram.shader.program, "vertexPosition_modelspace");
    ModelAttribLocations[1] = glGetAttribLocation(modelShaderProgram.shader.program, "vertexNormal_modelspace");
    ModelAttribLocations[2] = glGetAttribLocation(modelShaderProgram.shader.program, "vertexUV");
    
    
    
    //
	// Uniform Block
	//
	glGenBuffers(1,&matricesUniBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUniBuffer);
	glBufferData(GL_UNIFORM_BUFFER, MatricesUniBufferSize,NULL,GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, matricesUniLoc, matricesUniBuffer, 0, MatricesUniBufferSize);	//setUniforms();
	glBindBuffer(GL_UNIFORM_BUFFER,0);
    
//	glEnable(GL_MULTISAMPLE);

    
	return(p);
}

bool Model::Import3DFromFile( const std::string& pFile)
{
    static bool first = true;
    if(first) {
        first = false;
        
        setupShaders();
    }
    
    std::ifstream fin(pFile.c_str());
    if(!fin.fail()) {
        fin.close();
    }
    else{
        printf("Couldn't open file: %s\n", pFile.c_str());
        printf("%s\n", importer.GetErrorString());
        return false;
    }
    
    scene = importer.ReadFile( pFile, aiProcessPreset_TargetRealtime_Quality);
    
    if( !scene)
    {
        printf("%s\n", importer.GetErrorString());
        return false;
    }
    
    // Now we can access the file's contents.
    printf("Import of scene %s succeeded.\n",pFile.c_str());
    
#ifdef __APPLE__
    boost::filesystem::path p(pFile);
    std::string basePath = p.parent_path().string()+"/";
#else
	char path[2000];
	wchar_t wpath[2000];
	strcpy(path, pFile.c_str());
	mbstowcs(wpath, path, strlen(path)+1);//Plus null
	LPWSTR ptr = wpath;
	PathRemoveFileSpec(ptr);
	std::wstring backToString(ptr);
	std::string basePath(std::string(backToString.begin(),backToString.end())+"\\");
#endif

    printf("Import textures of scene %s...\n",pFile.c_str());
    LoadGLTextures(scene, basePath);
    printf("Import textures of scene %s done.\n",pFile.c_str());
    
    printf("Import generating VAOs for scene %s...\n",pFile.c_str());
    genVAOsAndUniformBuffer(scene);
    printf("Import generating VAOs for scene %s done.\n",pFile.c_str());

    if(!checkForErrors()) {
        std::cerr << "Problem loading model..." << std::endl;
        exit(1);
    }
    
    return true;
}

int Model::LoadGLTextures(const aiScene* scene, const std::string &basePath)
{
	for (unsigned int m=0; m<scene->mNumMaterials; ++m)
	{
		int texIndex = 0;
		aiString path;
		aiReturn texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
		while (texFound == AI_SUCCESS) {
			textureIdMap[path.data] = 0;
			texIndex++;
			texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
		}
	}
    
	unsigned long numTextures = textureIdMap.size();
	GLuint* textureIds = new GLuint[numTextures];
    
	std::map<std::string, GLuint>::iterator itr = textureIdMap.begin();
	for (int i=0; itr != textureIdMap.end(); ++i, ++itr)
	{
		std::string filename = basePath+(*itr).first;  // get filename
        GLuint loadedTexId = loadTexture(filename.c_str(), false, true);
        textureIds[i] = loadedTexId;
        (*itr).second = textureIds[i];
	}
    
	delete [] textureIds;
    
    return true;
}

void Model::genVAOsAndUniformBuffer(const aiScene *sc) {
    checkForErrors();
    
    struct MyMesh aMesh;
    struct MyMaterial aMat;
    GLuint buffer;
    
    // For each mesh
    for (unsigned int n = 0; n < sc->mNumMeshes; ++n)
    {
        const aiMesh* mesh = sc->mMeshes[n];
        
        // create array with faces
        // have to convert from Assimp format to array
        unsigned int *faceArray;
        faceArray = (unsigned int *)malloc(sizeof(unsigned int) * mesh->mNumFaces * 3);
        unsigned int faceIndex = 0;
        
        for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
            const aiFace* face = &mesh->mFaces[t];
            
            memcpy(&faceArray[faceIndex], face->mIndices,3 * sizeof(unsigned int));
            faceIndex += 3;
        }
        aMesh.numFaces = sc->mMeshes[n]->mNumFaces;
        
        // generate Vertex Array for mesh
        glGenVertexArrays(1,&(aMesh.vao));
        glBindVertexArray(aMesh.vao);
        
        // buffer for faces
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->mNumFaces * 3, faceArray, GL_STATIC_DRAW);
        free(faceArray);
        
        // buffer for vertex positions
        if (mesh->HasPositions()) {
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(vertexLoc);
            glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);
        }
        
        // buffer for vertex normals
        if (mesh->HasNormals()) {
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);
            glEnableVertexAttribArray(normalLoc);
            glVertexAttribPointer(normalLoc, 3, GL_FLOAT, 0, 0, 0);
        }
        
        // buffer for vertex texture coordinates
        if (mesh->HasTextureCoords(0)) {
            float *texCoords = (float *)malloc(sizeof(float)*2*mesh->mNumVertices);
            for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
                
                texCoords[k*2]   = mesh->mTextureCoords[0][k].x;
                texCoords[k*2+1] = mesh->mTextureCoords[0][k].y;
                
            }
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mesh->mNumVertices, texCoords, GL_STATIC_DRAW);
            glEnableVertexAttribArray(texCoordLoc);
            glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, 0, 0, 0);
        }
        
        // unbind buffers
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        
        // create material uniform buffer
        aiMaterial *mtl = sc->mMaterials[mesh->mMaterialIndex];
        
        aiString texPath;   //contains filename of texture
        if(AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath)){
            //bind texture
            unsigned int texId = textureIdMap[texPath.data];
            aMesh.texIndex = texId;
            aMat.texCount = 1;
        }
        else
            aMat.texCount = 0;
        
        float c[4];
        set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
        aiColor4D diffuse;
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
            color4_to_float4(&diffuse, c);
        memcpy(aMat.diffuse, c, sizeof(c));
        
        set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
        aiColor4D ambient;
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
            color4_to_float4(&ambient, c);
        memcpy(aMat.ambient, c, sizeof(c));
        
        set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
        aiColor4D specular;
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
            color4_to_float4(&specular, c);
        memcpy(aMat.specular, c, sizeof(c));
        
        set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
        aiColor4D emission;
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
            color4_to_float4(&emission, c);
        memcpy(aMat.emissive, c, sizeof(c));
        
        float shininess = 0.0;
        unsigned int max;
        aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
        aMat.shininess = shininess;
        
        glGenBuffers(1,&(aMesh.uniformBlockIndex));
        glBindBuffer(GL_UNIFORM_BUFFER,aMesh.uniformBlockIndex);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(aMat), (void *)(&aMat), GL_STATIC_DRAW);
        
        myMeshes.push_back(aMesh);
    }
}

void Model::recursive_render(const aiScene *sc, const aiNode* nd,  const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP)
{
    aiMatrix4x4 m = nd->mTransformation;
    m.Transpose(); // OpenGL matrices are column major
    
    glm::mat4 transformation(m.a1, m.a2, m.a3, m.a4,
                             m.b1, m.b2, m.b3, m.b4,
                             m.c1, m.c2, m.c3, m.c4,
                             m.d1, m.d2, m.d3, m.d4);
    glm::mat4 MVP2 = MVP*transformation;
    glm::mat4 M2 = M*transformation;
    
    if(shadowPass) {
        glUniformMatrix4fv(ShadowUniformLocations[0], 1, GL_FALSE, &MVP2[0][0]);
    } else {
        glUniformMatrix4fv(ModelUniformLocations[0], 1, GL_FALSE, &MVP2[0][0]);
        glUniformMatrix4fv(ModelUniformLocations[1], 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(ModelUniformLocations[2], 1, GL_FALSE, &M2[0][0]);
        glUniformMatrix4fv(ModelUniformLocations[4], 1, GL_FALSE, &(V*M2)[0][0]);
        if(ModelUniformLocations[5] > -1) glUniform3f(ModelUniformLocations[5], lightInvDir.x, lightInvDir.y, lightInvDir.z);
    }
    
    // draw node
    for (unsigned int n=0; n < nd->mNumMeshes; ++n){
        glBindBufferRange(GL_UNIFORM_BUFFER, materialUniLoc, myMeshes[nd->mMeshes[n]].uniformBlockIndex, 0, sizeof(struct MyMaterial));
        
        if(!shadowPass) {
            glBindTexture(GL_TEXTURE_2D, myMeshes[nd->mMeshes[n]].texIndex);
        }
        glBindVertexArray(myMeshes[nd->mMeshes[n]].vao);
        glDrawElements(GL_TRIANGLES,myMeshes[nd->mMeshes[n]].numFaces*3,GL_UNSIGNED_INT,0);
        
    }
    
    // draw children
    for (unsigned int n=0; n < nd->mNumChildren; ++n){
        recursive_render(sc, nd->mChildren[n], MVP2, V, M2, shadowPass, depthMVP);
    }
}

void Model::renderScene(GLuint program, const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP) {
//	glUseProgram(program);
    
    if(shadowPass) {
        glUseProgram(shadowProgram.shader.program);
    } else {
        glUseProgram(modelShaderProgram.shader.program);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(texUnit,0);
    }
    
//    glActiveTexture(GL_TEXTURE0);
//    glUniform1i(ModelUniformLocations[3], 0);
    
	recursive_render(scene, scene->mRootNode, MVP, V, M, shadowPass, depthMVP);
}
