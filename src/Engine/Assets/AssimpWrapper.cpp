/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015-2018 OSRE ( Open Source Render Engine ) by Kim Kulling

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-----------------------------------------------------------------------------------------------*/
#include <osre/Assets/AssimpWrapper.h>
#include <osre/Assets/Model.h>
#include <osre/IO/Uri.h>
#include <osre/Common/Logger.h>
#include <osre/Common/Ids.h>
#include <osre/RenderBackend/RenderCommon.h>
#include <osre/RenderBackend/Geometry.h>
#include <osre/Assets/AssetRegistry.h>
#include <osre/Scene/GeometryBuilder.h>
#include <osre/Scene/MaterialBuilder.h>
#include <osre/Scene/Component.h>
#include <osre/Scene/Node.h>
#include <osre/Collision/TAABB.h>
#include <osre/IO/IOService.h>
#include <osre/IO/AbstractFileSystem.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>

namespace OSRE {
namespace Assets {
    
using namespace ::Assimp;
using namespace ::OSRE::RenderBackend;
using namespace ::OSRE::Scene;
using namespace ::OSRE::Collision;

static const String Tag = "AssimpWrapper";

AssimpWrapper::AssimpWrapper( Common::Ids &ids )
: m_geoArray()
, m_matArray()
, m_model( nullptr )
, m_parent( nullptr )
, m_ids( ids )
, m_mvpParam( nullptr )
, m_root()
, m_absPathWithFile() {
    // empty
}

AssimpWrapper::~AssimpWrapper() {
    // empty
}

static void separatePathAndFilename(const String pathAndFilename, String &path, String &filename) {
    path.clear();
    filename.clear();
    if (pathAndFilename.empty()) {
        return;
    }

    String::size_type pos = pathAndFilename.rfind("/");
    if (String::npos == pos) {
        return;
    }

    path = pathAndFilename.substr(0, pos+1);
    filename = pathAndFilename.substr(pos+1, pathAndFilename.size() - pos-1);
}

bool AssimpWrapper::importAsset( const IO::Uri &file, ui32 flags ) {
    if ( !file.isValid() ) {
        osre_error( Tag, "URI " + file.getUri() + " is invalid " );
        return false;
    }

    unsigned int importFlags = aiProcess_CalcTangentSpace
        | aiProcess_GenSmoothNormals
        | aiProcess_JoinIdenticalVertices
        | aiProcess_ImproveCacheLocality
        | aiProcess_LimitBoneWeights
        | aiProcess_RemoveRedundantMaterials
        | aiProcess_SplitLargeMeshes
        | aiProcess_Triangulate
        | aiProcess_GenUVCoords
        | aiProcess_SortByPType;
    flags = importFlags;

    m_root = AssetRegistry::getPath( "media" );
    m_absPathWithFile = AssetRegistry::resolvePathFromUri( file );

    String filename;
    separatePathAndFilename(m_absPathWithFile, m_root, filename);
    filename = m_root + filename;
    Importer myImporter;
    const aiScene *scene = myImporter.ReadFile( filename, flags );
    if ( nullptr == scene ) {
        m_root = "";
        m_absPathWithFile = "";
        return false;
    }
    convertSceneToModel( scene );
    m_model->setGeoArray( m_geoArray );

    return true;
}

Model *AssimpWrapper::getModel() const {
    return m_model;
}

Model *AssimpWrapper::convertSceneToModel( const aiScene *scene ) {
    if ( nullptr == scene ) {
        return nullptr;
    }

    m_model = new Model;
    if ( scene->HasMaterials() ) {
        for ( ui32 i = 0; i < scene->mNumMaterials; i++ ) {
            aiMaterial *currentMat( scene->mMaterials[ i ] );
            if ( nullptr != currentMat ) {
                handleMaterial(currentMat);
            }
        }
    }
    
    if ( scene->HasMeshes() ) {
        for ( ui32 i = 0; i < scene->mNumMeshes; i++ ) {
            aiMesh *currentMesh( scene->mMeshes[ i ] );
            if ( nullptr == currentMesh ) {
                continue;
            }
            handleMesh( currentMesh );
        }
    }

    if ( nullptr != scene->mRootNode ) {
        handleNode( scene->mRootNode, nullptr );
    }

    return m_model;
}

void AssimpWrapper::handleMesh( aiMesh *mesh ) {
    if ( nullptr == mesh ) {
        return;
    }

    TAABB<f32> aabb = m_model->getAABB();
    Geometry *geo( Geometry::create( 1 ) );
	geo->m_vertextype = VertexType::RenderVertex;
    ui32 numVertices( mesh->mNumVertices );
    RenderVert *vertices = new RenderVert[ numVertices ];
    for ( ui32 i = 0; i < numVertices; i++ ) {
        if ( mesh->HasPositions() ) {
            aiVector3D &vec3 = mesh->mVertices[ i ];
            vertices[ i ].position.x = vec3.x;
            vertices[ i ].position.y = vec3.y;
            vertices[ i ].position.z = vec3.z;

            aabb.merge( vec3.x, vec3.y, vec3.z );
        }

        if ( mesh->HasNormals() ) {
            aiVector3D &normal = mesh->mNormals[ i ];
            vertices[ i ].normal.x = normal.x;
            vertices[ i ].normal.y = normal.y;
            vertices[ i ].normal.z = normal.z;
        }

        if ( mesh->HasVertexColors( 0 ) ) {
            aiColor4D &diffuse = mesh->mColors[ 0 ][ i ];
            vertices[ i ].color0.r = diffuse.r;
            vertices[ i ].color0.g = diffuse.g;
            vertices[ i ].color0.b = diffuse.b;
        }

        for ( ui32 texIdx =0; texIdx<AI_MAX_NUMBER_OF_TEXTURECOORDS; texIdx++ ) {
            if ( mesh->HasTextureCoords( texIdx ) ) {
                if ( 0 == texIdx ) {
                    aiVector3D &tex0 = mesh->mTextureCoords[ texIdx ][ i ];
                    vertices[ i ].tex0.x = tex0.x;
                    vertices[ i ].tex0.y = tex0.y;
                }
            }
        }
    }
    //GeometryDiagnosticUtils::dumVertices( vertices, numVertices );

    const ui32 matIdx( mesh->mMaterialIndex );
    Material *osreMat = m_matArray[ matIdx ];
    geo->m_material = osreMat;
    const ui32 vbSize( sizeof( RenderVert ) * numVertices );
    geo->m_vb = BufferData::alloc( BufferType::VertexBuffer, vbSize, BufferAccessType::ReadOnly );
    geo->m_vb->copyFrom( &vertices[ 0 ], vbSize );

    CPPCore::TArray<ui32> indexArray;
    for ( ui32 i = 0; i < mesh->mNumFaces; i++ ) {
        aiFace &currentFace = mesh->mFaces[ i ];
        for ( ui32 idx = 0; idx < currentFace.mNumIndices; idx++ ) {
            const ui32 index = currentFace.mIndices[ idx ];
            indexArray.add( index );
        }
    }
	//Scene::GeometryDiagnosticUtils::dumpIndices( indexArray );

    geo->m_ib = BufferData::alloc( BufferType::IndexBuffer, sizeof( ui32 ) * indexArray.size(), BufferAccessType::ReadOnly );
    geo->m_ib->copyFrom( &indexArray[ 0 ], geo->m_ib->m_size );

    geo->m_numPrimGroups = 1;
    geo->m_pPrimGroups = new PrimitiveGroup[ geo->m_numPrimGroups ];
    geo->m_pPrimGroups[ 0 ].m_indexType = IndexType::UnsignedInt;
    geo->m_pPrimGroups[ 0 ].m_numIndices = indexArray.size();
    geo->m_pPrimGroups[ 0 ].m_primitive = PrimitiveType::TriangleList;
    geo->m_pPrimGroups[ 0 ].m_startIndex = 0;

    geo->m_material = m_matArray[matIdx];
    
    //

    m_geoArray.add( geo );
    m_model->setAABB( aabb );
}

void AssimpWrapper::handleNode( aiNode *node, Scene::Node *parent ) {
    if ( nullptr == node) {
        return;
    }
    
    Node *newNode = new Node( node->mName.C_Str(), m_ids, 
            Node::RenderCompRequest::RenderCompRequested,
            Node::TransformCompRequest::TransformCompRequested, 
            parent );
    
    // If this is the root-node of the model, set it as the root for the model
    if ( nullptr == m_parent ) {
        m_parent = newNode;
        m_model->setRootNode( m_parent );
    }

    if ( node->mNumMeshes > 0 ) {
        for ( ui32 i = 0; i < node->mNumMeshes; i++ ) {
            const ui32 meshIdx( node->mMeshes[ i ] );
            if ( meshIdx >= m_geoArray.size() ) {
                continue;
            }

            Geometry *geo( m_geoArray[ meshIdx ] );
            if ( nullptr != geo ) {
                newNode->addGeometry( geo );
            }
        }
    }

    for ( ui32 i = 0; i < node->mNumChildren; i++ ) {
        aiNode *currentNode = node->mChildren[ i ];
        if ( nullptr == currentNode) {
            continue;
        }

        handleNode( currentNode, newNode );
    }
}

static void setColor4( const aiColor4D &aiCol, Color4 &col ) {
    col.m_r = aiCol.r;
    col.m_g = aiCol.g;
    col.m_b = aiCol.b;
    col.m_a = aiCol.a;
}

using IO::Stream;
using IO::AbstractFileSystem;

static void setTexture( const String &resolvedPath, const aiString &texPath, CPPCore::TArray<Texture*> &textures ) {
    Texture *tex = new Texture;
    textures.add( tex );
    String texname;
    texname += "file://";
    texname += resolvedPath;
    String temp( texPath.C_Str() ), temp1;
    IO::Uri::normalizePath( temp, '\\', temp1 );
    texname += temp1;

    tex->m_loc = IO::Uri( texname );
    String::size_type pos = texname.rfind( "/" );
    if ( pos != String::npos ) {
        texname = texname.substr( pos, texname.size() - pos );
    }
    if (IO::IOService::getInstance()->fileExists(tex->m_loc)) {
        IO::AbstractFileSystem *fs = IO::IOService::getInstance()->getFileSystem("file");
        IO::Stream *file = fs->open(tex->m_loc, IO::Stream::AccessMode::ReadAccess);
        tex->m_size = file->getSize();
        file->read(tex->m_data, tex->m_size);
        fs->close(&file);
    }
        
    tex->m_textureName = texname;
    tex->m_width = 0;
    tex->m_height = 0;
    tex->m_channels = 0;
    tex->m_data = nullptr;
    tex->m_size = 0;
}

static void assignTexturesToMat( Material *osreMat, CPPCore::TArray<Texture*> &textures ) {
    if ( nullptr == osreMat ) {
        return;
    }

    osreMat->m_numTextures = textures.size();
    if ( textures.isEmpty() ) {
        return;
    }

    osreMat->m_textures = new Texture *[ osreMat->m_numTextures ];
    for ( ui32 i = 0; i < osreMat->m_numTextures; ++i ) {
        osreMat->m_textures[ i ] = textures[ i ];
    }
}

void AssimpWrapper::handleMaterial( aiMaterial *material ) {
    if ( nullptr == material ) {
        return;
    }
    
    Material *osreMat( MaterialBuilder::createBuildinMaterial( RenderBackend::VertexType::RenderVertex ) );
    m_matArray.add( osreMat );

    i32 texIndex( 0 );
    aiString texPath;	// contains filename of texture
    CPPCore::TArray<Texture*> textures;
    if ( AI_SUCCESS == material->GetTexture( aiTextureType_DIFFUSE, texIndex, &texPath ) ) {
        setTexture( m_root, texPath, textures );
    }
    assignTexturesToMat( osreMat, textures );

    aiColor4D diffuse;
    if ( AI_SUCCESS == aiGetMaterialColor( material, AI_MATKEY_COLOR_DIFFUSE, &diffuse ) ) {
        setColor4( diffuse, osreMat->m_color[ (ui32) MaterialColorType::Mat_Diffuse ] );
    }

    aiColor4D specular;
    if ( AI_SUCCESS == aiGetMaterialColor( material, AI_MATKEY_COLOR_SPECULAR, &specular ) ) {
        setColor4( specular, osreMat->m_color[ ( ui32 ) MaterialColorType::Mat_Specular ] );
    }

    aiColor4D ambient;
    if ( AI_SUCCESS == aiGetMaterialColor( material, AI_MATKEY_COLOR_AMBIENT, &ambient ) ) {
        setColor4( ambient, osreMat->m_color[ ( ui32 ) MaterialColorType::Mat_Ambient ] );
    }

    aiColor4D emission;
    if ( AI_SUCCESS == aiGetMaterialColor( material, AI_MATKEY_COLOR_EMISSIVE, &emission ) ) {
        setColor4( emission, osreMat->m_color[ ( ui32 ) MaterialColorType::Mat_Emission ] );
    }

    ai_real shininess, strength;
    unsigned int max;	// changed: to unsigned
    if ( AI_SUCCESS == aiGetMaterialFloatArray( material, AI_MATKEY_SHININESS, &shininess, &max ) ) {
        // todo
    }

    if ( AI_SUCCESS == aiGetMaterialFloatArray( material, AI_MATKEY_SHININESS_STRENGTH, &strength, &max ) ) {
        // todo
    }
}

} // Namespace Assets
} // Namespace OSRE
