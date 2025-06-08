#pragma once

#include <InteractiveToolkit/InteractiveToolkit.h>
#include <InteractiveToolkit-Extension/InteractiveToolkit-Extension.h>

bool starts_with(std::string const &value, std::string const &starting)
{
    return (value.rfind(starting, 0) == 0);
}

// MacOS fix importing
#ifdef __APPLE__
#define noexcept
#endif

// assimp imports
#include <assimp/config.h>
#include <assimp/cimport.h>     // Plain-C interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

unsigned int floatToColor(float c)
{
    c = MathCore::OP<float>::clamp(c, 0.0f, 1.0f);

    unsigned int result = (unsigned int)(c * 255.0f + 0.5f);
    if (result > 255)
        result = 255;

    return result;
}

// aiTextureType_DIFFUSE
void processTextureType(ITKExtension::Model::Material &material, aiMaterial *aimaterial, aiTextureType textureType, ITKExtension::Model::TextureType ourTextureType)
{

    for (uint32_t j = 0; j < aimaterial->GetTextureCount(textureType); j++)
    {
        aiString path;
        aiTextureMapping mapping = aiTextureMapping_UV;
        unsigned int uvIndex;
        float blend;
        aiTextureOp texOP = (aiTextureOp)aiTextureOp_Multiply; // TextureOp_Default;
        aiTextureMapMode mapMode = aiTextureMapMode_Wrap;

        fprintf(stdout, "         tex(%i) => \n", j);
        if (aimaterial->GetTexture(textureType, j, &path, &mapping, &uvIndex, &blend, &texOP, &mapMode) == aiReturn_SUCCESS)
        {
            std::string folder, filename, file_wo_ext, ext;
            ITKCommon::Path::splitPathString(path.data, &folder, &filename, &file_wo_ext, &ext);

            ITKExtension::Model::TextureOp mTexOP = ITKExtension::Model::TextureOp_Default;
            switch (texOP)
            {
            case aiTextureOp_Multiply:
                mTexOP = ITKExtension::Model::TextureOp_Multiply;
                break;
            case aiTextureOp_Add:
                mTexOP = ITKExtension::Model::TextureOp_Add;
                break;
            case aiTextureOp_Subtract:
                mTexOP = ITKExtension::Model::TextureOp_Subtract;
                break;
            case aiTextureOp_Divide:
                mTexOP = ITKExtension::Model::TextureOp_Divide;
                break;
            case aiTextureOp_SmoothAdd:
                mTexOP = ITKExtension::Model::TextureOp_SmoothAdd;
                break;
            case aiTextureOp_SignedAdd:
                mTexOP = ITKExtension::Model::TextureOp_SignedAdd;
                break;
            }

            ITKExtension::Model::TextureMapMode mMapMode = ITKExtension::Model::TextureMapMode_Wrap;
            switch (mapMode)
            {
            case aiTextureMapMode_Wrap:
                mMapMode = ITKExtension::Model::TextureMapMode_Wrap;
                break;
            case aiTextureMapMode_Clamp:
                mMapMode = ITKExtension::Model::TextureMapMode_Clamp;
                break;
            case aiTextureMapMode_Decal:
                mMapMode = ITKExtension::Model::TextureMapMode_Decal;
                break;
            case aiTextureMapMode_Mirror:
                mMapMode = ITKExtension::Model::TextureMapMode_Mirror;
                break;
            }

            if (mapping == aiTextureMapping_UV)
            {

                fprintf(stdout, "                    file: %s\n", file_wo_ext.c_str());
                fprintf(stdout, "                    ext: %s\n", ext.c_str());
                fprintf(stdout, "                    type: %s\n", ITKExtension::Model::TextureTypeToStr(ourTextureType));
                fprintf(stdout, "                    op: %s\n", ITKExtension::Model::TextureOpToStr(mTexOP));
                fprintf(stdout, "                    mapMode: %s\n", ITKExtension::Model::TextureMapModeToStr(mMapMode));
                fprintf(stdout, "                    uvIndex: %i\n", uvIndex);

                ITKExtension::Model::Texture texture;
                texture.filename = file_wo_ext;
                texture.fileext = ext;
                texture.type = ourTextureType; // TextureType_DIFFUSE;
                texture.op = mTexOP;
                texture.uvIndex = uvIndex;
                texture.mapMode = mMapMode;

                material.textures.push_back(texture);
            }
            else
            {
                fprintf(stdout, "                    NOT IMPORTED TEXTURE INFO (%s) -- mapping type only support UV mapping...\n", filename.c_str());
            }
        }
        else
        {
            fprintf(stdout, "                    Error to process...\n");
        }
    }
}

void recursiveInsertNodes(ITKExtension::Model::ModelContainer *result, aiNode *ainode, std::string output, int parentIndex, bool leftHanded)
{

    ITKExtension::Model::Node node;

    node.name = ainode->mName.data;
    node.transform = MathCore::OP<MathCore::mat4f>::transpose(MathCore::mat4f(
        ainode->mTransformation.a1, ainode->mTransformation.b1, ainode->mTransformation.c1, ainode->mTransformation.d1,
        ainode->mTransformation.a2, ainode->mTransformation.b2, ainode->mTransformation.c2, ainode->mTransformation.d2,
        ainode->mTransformation.a3, ainode->mTransformation.b3, ainode->mTransformation.c3, ainode->mTransformation.d3,
        ainode->mTransformation.a4, ainode->mTransformation.b4, ainode->mTransformation.c4, ainode->mTransformation.d4));

    // fix transform
    {
        MathCore::mat4f m = node.transform;
        // vec3 translatevec =toVec3(m * vec4(0,0,0,1)) * vec3(1,1,-1);
        MathCore::vec3f translatevec = MathCore::CVT<MathCore::vec4f>::toVec3(m * MathCore::vec4f(0, 0, 0, 1));
        MathCore::vec3f scalevec = MathCore::vec3f(MathCore::OP<MathCore::vec4f>::length(m[0]),
                                                   MathCore::OP<MathCore::vec4f>::length(m[1]),
                                                   MathCore::OP<MathCore::vec4f>::length(m[2]));

        m = MathCore::OP<MathCore::mat4f>::extractRotation(m);
        m[0] = MathCore::OP<MathCore::vec4f>::normalize(m[0]);
        m[1] = MathCore::OP<MathCore::vec4f>::normalize(m[1]);
        m[2] = MathCore::OP<MathCore::vec4f>::normalize(m[2]);

        MathCore::quatf rotationquat = MathCore::GEN<MathCore::quatf>::fromMat4(m); // * aRibeiro::quatFromEuler(0, 0, DEG2RAD(180.0));
        if (parentIndex == -1)
            rotationquat = MathCore::GEN<MathCore::quatf>::fromEuler(0, MathCore::OP<float>::deg_2_rad(180.0f), 0) * rotationquat;
        node.transform =
            MathCore::GEN<MathCore::mat4f>::translateHomogeneous(translatevec) *
            MathCore::GEN<MathCore::mat4f>::fromQuat(rotationquat) *
            MathCore::GEN<MathCore::mat4f>::scaleHomogeneous(scalevec);
    }

    for (uint32_t i = 0; i < ainode->mNumMeshes; i++)
    {
        node.geometries.push_back(ainode->mMeshes[i]);
    }

    result->nodes.push_back(node);
    int nodeIndex = (int)result->nodes.size() - 1;

    if (parentIndex != -1)
    {
        result->nodes[parentIndex].children.push_back(nodeIndex);
    }

    if (ainode->mNumMeshes > 0)
    {
        fprintf(stdout, "%s+%s ( meshCount: %i ids: ", output.c_str(), ainode->mName.data, ainode->mNumMeshes);
        for (uint32_t i = 0; i < ainode->mNumMeshes; i++)
        {
            fprintf(stdout, "%i ", ainode->mMeshes[i]);
        }
        fprintf(stdout, ")\n");
    }
    else
    {
        fprintf(stdout, "%s+%s\n", output.c_str(), ainode->mName.data);
    }

    {
        MathCore::vec3f LocalPosition = MathCore::CVT<MathCore::vec4f>::toVec3(node.transform * MathCore::vec4f(0, 0, 0, 1));
        MathCore::vec3f LocalScale = MathCore::vec3f(
            MathCore::OP<MathCore::vec4f>::length(node.transform[0]),
            MathCore::OP<MathCore::vec4f>::length(node.transform[1]),
            MathCore::OP<MathCore::vec4f>::length(node.transform[2]));

        MathCore::mat4f m = MathCore::OP<MathCore::mat4f>::extractRotation(node.transform);

        m[0] = MathCore::OP<MathCore::vec4f>::normalize(m[0]);
        m[1] = MathCore::OP<MathCore::vec4f>::normalize(m[1]);
        m[2] = MathCore::OP<MathCore::vec4f>::normalize(m[2]);

        MathCore::quatf LocalRotation = MathCore::GEN<MathCore::quatf>::fromMat4(m);

        printf("%s LocalPosition %f, %f, %f\n",
               output.c_str(),
               LocalPosition.x,
               LocalPosition.y,
               LocalPosition.z);

        printf("%s LocalScale %f, %f, %f\n",
               output.c_str(),
               LocalScale.x,
               LocalScale.y,
               LocalScale.z);

        printf("%s LocalRotation %f, %f, %f, %f\n",
               output.c_str(),
               LocalRotation.x,
               LocalRotation.y,
               LocalRotation.z,
               LocalRotation.w);
    }

    for (uint32_t i = 0; i < ainode->mNumChildren; i++)
        recursiveInsertNodes(result, ainode->mChildren[i], output + std::string(" "), nodeIndex, leftHanded);
}

ITKExtension::Model::ModelContainer *ImportFromAssimp(const char *filename, bool leftHanded = true)
{
    const aiScene *scene;
    aiPropertyStore *store = aiCreatePropertyStore();

    // disable the extra FBX nodes generation...
    // aiSetImportPropertyInteger(store, AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, AI_FALSE);

    // aiSetImportPropertyInteger(store, AI_CONFIG_PP_SLM_VERTEX_LIMIT, AI_SLM_DEFAULT_MAX_VERTICES);

    // aiSetImportPropertyInteger(store, AI_CONFIG_PP_SLM_VERTEX_LIMIT, 65536);
    aiSetImportPropertyInteger(store, AI_CONFIG_PP_SLM_VERTEX_LIMIT, AI_MAX_VERTICES);
    // aiSetImportPropertyInteger(store, AI_SLM_DEFAULT_MAX_VERTICES, AI_SLM_DEFAULT_MAX_TRIANGLES);

    // assimp unit is cm... convert it to meters
    //  aiSetImportPropertyFloat(store, AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 0.01f);

    unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_CalcTangentSpace | aiProcess_GlobalScale
        //| aiProcess_GenNormals
        ;
    // &(~aiProcess_SplitLargeMeshes);
    if (leftHanded)
        flags |= aiProcess_ConvertToLeftHanded;

    scene = aiImportFileExWithProperties(filename, flags, NULL, store);
    aiReleasePropertyStore(store);

    if (!scene)
    {
        fprintf(stderr, "[Loader] ERROR: %s\n", aiGetErrorString());
        exit(-1);
    }

    ITKExtension::Model::ModelContainer *result = new ITKExtension::Model::ModelContainer();
    //
    // MESH EXPORTING
    //
    for (uint32_t i = 0; i < scene->mNumMeshes; i++)
    {
        fprintf(stdout, "[Mesh] %i / %i\n", i + 1, scene->mNumMeshes);
        ITKExtension::Model::Geometry geometry;
        // geometry.format = 0;

        aiMesh *mesh = scene->mMeshes[i];

        fprintf(stdout, "         Name: %s\n", mesh->mName.data);
        geometry.name = mesh->mName.data;
        geometry.materialIndex = mesh->mMaterialIndex;

        int primitiveCount = 0;
        // check primitive count
        if (mesh->mPrimitiveTypes & aiPrimitiveType::aiPrimitiveType_POINT)
        {
            fprintf(stdout, "         Contains points\n");
            primitiveCount++;
            geometry.indiceCountPerFace = 1;
        }
        if (mesh->mPrimitiveTypes & aiPrimitiveType::aiPrimitiveType_LINE)
        {
            fprintf(stdout, "         Contains lines\n");
            primitiveCount++;
            geometry.indiceCountPerFace = 2;
        }
        if (mesh->mPrimitiveTypes & aiPrimitiveType::aiPrimitiveType_TRIANGLE)
        {
            fprintf(stdout, "         Contains triangles\n");
            primitiveCount++;
            geometry.indiceCountPerFace = 3;
        }
        if (mesh->mPrimitiveTypes & aiPrimitiveType::aiPrimitiveType_POLYGON)
        {
            fprintf(stderr, "[Loader] ERROR: Polygon import not supported. Try to split into triangles before importe...\n");
            exit(-1);
        }

        if (primitiveCount != 1)
        {
            fprintf(stderr, "[Loader] ERROR: Geometry with mixed faces or no face not supported (points with lines or lines with triangles)...\n");
            exit(-1);
        }

        if (mesh->HasPositions())
        {
            fprintf(stdout, "         + positions (%i)\n", mesh->mNumVertices);
            geometry.format = geometry.format | ITKExtension::Model::CONTAINS_POS;
        }
        else
        {
            fprintf(stderr, "[Loader] ERROR: Geometry without vertices...\n");
            exit(-1);
        }

        if (mesh->HasNormals())
        {
            fprintf(stdout, "         + normals\n");
            geometry.format = geometry.format | ITKExtension::Model::CONTAINS_NORMAL;
        }
        if (mesh->HasTangentsAndBitangents())
        {
            fprintf(stdout, "         + tangents (binormals need to be derivated)\n");
            // fprintf(stdout, "         + binormals\n");
            geometry.format = geometry.format | ITKExtension::Model::CONTAINS_TANGENT; // | VertexFormat::CONTAINS_BINORMAL;
        }

        for (int i = 0; i < 8; i++)
        {
            if (mesh->HasTextureCoords(i))
            {
                fprintf(stdout, "         + uv %i\n", i);
                geometry.format = geometry.format | (ITKExtension::Model::CONTAINS_UV0 << i);
            }
        }

        for (int i = 0; i < 8; i++)
        {
            if (mesh->HasVertexColors(i))
            {
                fprintf(stdout, "         + vertex color %i\n", i);
                geometry.format = geometry.format | (ITKExtension::Model::CONTAINS_COLOR0 << i);
            }
        }

        //
        // Vertex attrib import
        //
        geometry.vertexCount = mesh->mNumVertices;
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            geometry.pos.push_back(MathCore::vec3f(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
            if (mesh->HasNormals())
                geometry.normals.push_back(MathCore::vec3f(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
            if (mesh->HasTangentsAndBitangents())
            {
                geometry.tangent.push_back(MathCore::vec3f(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z));
                // geometry.binormal.push_back(vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z));
            }
            for (int j = 0; j < 8; j++)
            {
                if (mesh->HasTextureCoords(j))
                    geometry.uv[j].push_back(MathCore::vec3f(mesh->mTextureCoords[j][i].x, mesh->mTextureCoords[j][i].y, mesh->mTextureCoords[j][i].z));
                if (mesh->HasVertexColors(j))
                    geometry.color[j].push_back(
                        MathCore::vec4f(mesh->mColors[j][i].r,
                                        mesh->mColors[j][i].g,
                                        mesh->mColors[j][i].b,
                                        mesh->mColors[j][i].a)
                        /*
                        (floatToColor(mesh->mColors[j][i].r) << 24) |
                        (floatToColor(mesh->mColors[j][i].g) << 16) |
                        (floatToColor(mesh->mColors[j][i].b) << 8) |
                        floatToColor(mesh->mColors[j][i].a)
                        */
                    );
            }
        }

        //
        // Indices import
        //
        if (mesh->HasFaces() && geometry.indiceCountPerFace != 1)
        {
            for (uint32_t i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                if (face.mNumIndices != geometry.indiceCountPerFace)
                {
                    fprintf(stderr, "[Loader] ERROR: Face indice count different from the geometry primitive type...\n");
                    exit(-1);
                }
                // Flip the winding order to be CCW with a left-handed coordinate system
                if (face.mNumIndices == 3 && leftHanded)
                {
                    geometry.indice.push_back(face.mIndices[0]);
                    geometry.indice.push_back(face.mIndices[2]);
                    geometry.indice.push_back(face.mIndices[1]);
                }
                else
                for (uint32_t i = 0; i < face.mNumIndices; i++)
                {
                    // ARIBEIRO_ABORT(face.mIndices[i] >= 65536, "Max index value error (%u).\n", face.mIndices[i]);
                    geometry.indice.push_back(face.mIndices[i]);
                }
            }
        }

        //
        // Bones import
        //
        if (mesh->HasBones())
        {
            fprintf(stdout, "         Contains bones\n");

            for (uint32_t i = 0; i < mesh->mNumBones; i++)
            {
                aiBone *aibone = mesh->mBones[i];

                ITKExtension::Model::Bone bone;
                bone.name = aibone->mName.data;

                /*
                bone.offset = transpose( mat4(
                                   aibone->mOffsetMatrix.a1, aibone->mOffsetMatrix.b1, aibone->mOffsetMatrix.c1, aibone->mOffsetMatrix.d1,
                                   aibone->mOffsetMatrix.a2, aibone->mOffsetMatrix.b2, aibone->mOffsetMatrix.c2, aibone->mOffsetMatrix.d2,
                                   aibone->mOffsetMatrix.a3, aibone->mOffsetMatrix.b3, aibone->mOffsetMatrix.c3, aibone->mOffsetMatrix.d3,
                                   aibone->mOffsetMatrix.a4, aibone->mOffsetMatrix.b4, aibone->mOffsetMatrix.c4, aibone->mOffsetMatrix.d4
                                   ) );

                //fix transform
                {
                    mat4 m = bone.offset;
                    vec3 translatevec =toVec3(m * vec4(0,0,0,1));
                    vec3 scalevec = MathCore::vec3f(aRibeiro::length(m[0]),
                                                aRibeiro::length(m[1]),
                                                aRibeiro::length(m[2]));
                    m = extractRotation(m);
                    m[0] = normalize(m[0]);
                    m[1] = normalize(m[1]);
                    m[2] = normalize(m[2]);
                    quat rotationquat = extractQuat( m );
                    bone.offset = translate(translatevec) * toMat4(rotationquat) * scale(scalevec);
                }
                */

                for (uint32_t j = 0; j < aibone->mNumWeights; j++)
                {
                    ITKExtension::Model::VertexWeight bw;
                    bw.vertexID = aibone->mWeights[j].mVertexId;
                    bw.weight = aibone->mWeights[j].mWeight;
                    bone.weights.push_back(bw);
                }

                geometry.bones.push_back(bone);
            }
        }

        if (mesh->mNumAnimMeshes > 0)
        {
            fprintf(stdout, "         ERROR: AnimMesh not implemented...\n");
        }

        result->geometries.push_back(geometry);
    }

    //
    // ANIMATION EXPORTING
    //

    for (uint32_t i = 0; i < scene->mNumAnimations; i++)
    {
        fprintf(stdout, "[Animation] %i / %i\n", i + 1, scene->mNumAnimations);
        aiAnimation *aianimation = scene->mAnimations[i];
        ITKExtension::Model::Animation animation;

        animation.name = aianimation->mName.data;
        animation.durationTicks = (float)aianimation->mDuration;
        animation.ticksPerSecond = (float)aianimation->mTicksPerSecond;
        if (aianimation->mTicksPerSecond == 0)
            animation.ticksPerSecond = 1.0f;

        fprintf(stdout, "            name => %s\n", aianimation->mName.data);
        fprintf(stdout, "            durationTicks => %f\n", animation.durationTicks);
        fprintf(stdout, "            ticksPerSecond => %f\n", animation.ticksPerSecond);
        fprintf(stdout, "            bone channels => %i\n", aianimation->mNumChannels);

        //
        // export channels
        //

        for (uint32_t j = 0; j < aianimation->mNumChannels; j++)
        {
            aiNodeAnim *ainodeanim = aianimation->mChannels[j];

            ITKExtension::Model::NodeAnimation na;
            na.nodeName = ainodeanim->mNodeName.data;

            na.preState = ITKExtension::Model::AnimBehaviour_DEFAULT;
            na.postState = ITKExtension::Model::AnimBehaviour_DEFAULT;

            switch (ainodeanim->mPreState)
            {
            case aiAnimBehaviour_DEFAULT:
                na.preState = ITKExtension::Model::AnimBehaviour_DEFAULT;
                break;
            case aiAnimBehaviour_CONSTANT:
                na.preState = ITKExtension::Model::AnimBehaviour_CONSTANT;
                break;
            case aiAnimBehaviour_LINEAR:
                na.preState = ITKExtension::Model::AnimBehaviour_LINEAR;
                break;
            case aiAnimBehaviour_REPEAT:
                na.preState = ITKExtension::Model::AnimBehaviour_REPEAT;
                break;
            }

            switch (ainodeanim->mPostState)
            {
            case aiAnimBehaviour_DEFAULT:
                na.postState = ITKExtension::Model::AnimBehaviour_DEFAULT;
                break;
            case aiAnimBehaviour_CONSTANT:
                na.postState = ITKExtension::Model::AnimBehaviour_CONSTANT;
                break;
            case aiAnimBehaviour_LINEAR:
                na.postState = ITKExtension::Model::AnimBehaviour_LINEAR;
                break;
            case aiAnimBehaviour_REPEAT:
                na.postState = ITKExtension::Model::AnimBehaviour_REPEAT;
                break;
            }

            for (uint32_t k = 0; k < ainodeanim->mNumPositionKeys; k++)
            {
                aiVectorKey aivectorkey = ainodeanim->mPositionKeys[k];

                ITKExtension::Model::Vec3Key vec3key;
                vec3key.time = (float)aivectorkey.mTime;
                vec3key.value = MathCore::vec3f(aivectorkey.mValue.x, aivectorkey.mValue.y, aivectorkey.mValue.z);

                na.positionKeys.push_back(vec3key);
            }

            for (uint32_t k = 0; k < ainodeanim->mNumScalingKeys; k++)
            {
                aiVectorKey aivectorkey = ainodeanim->mScalingKeys[k];

                ITKExtension::Model::Vec3Key vec3key;
                vec3key.time = (float)aivectorkey.mTime;
                vec3key.value = MathCore::vec3f(aivectorkey.mValue.x, aivectorkey.mValue.y, aivectorkey.mValue.z);

                na.scalingKeys.push_back(vec3key);
            }

            for (uint32_t k = 0; k < ainodeanim->mNumRotationKeys; k++)
            {
                aiQuatKey aiquatkey = ainodeanim->mRotationKeys[k];

                ITKExtension::Model::QuatKey quatkey;
                quatkey.time = (float)aiquatkey.mTime;
                quatkey.value = MathCore::quatf(aiquatkey.mValue.x,
                                                aiquatkey.mValue.y,
                                                aiquatkey.mValue.z,
                                                aiquatkey.mValue.w);

                na.rotationKeys.push_back(quatkey);
            }

            animation.channels.push_back(na);
        }

        if (aianimation->mNumMeshChannels > 0)
        {
            fprintf(stdout, "         ERROR: MeshChannels not implemented...\n");
        }
        if (aianimation->mNumMorphMeshChannels > 0)
        {
            fprintf(stdout, "         ERROR: MorphMeshChannels not implemented...\n");
        }

        result->animations.push_back(animation);
    }

    //
    // CAMERA EXPORTING
    //

    for (uint32_t i = 0; i < scene->mNumCameras; i++)
    {
        fprintf(stdout, "[Camera] %i / %i\n", i + 1, scene->mNumCameras);

        aiCamera *aicamera = scene->mCameras[i];

        ITKExtension::Model::Camera camera;

        camera.name = aicamera->mName.data;
        fprintf(stderr, "         name: %s\n", aicamera->mName.data);

        camera.pos = MathCore::vec3f(aicamera->mPosition.x, aicamera->mPosition.y, aicamera->mPosition.z);
        camera.up = MathCore::vec3f(aicamera->mUp.x, aicamera->mUp.y, aicamera->mUp.z);
        camera.forward = MathCore::vec3f(aicamera->mLookAt.x, aicamera->mLookAt.y, aicamera->mLookAt.z);

        camera.horizontalFOVrad = aicamera->mHorizontalFOV;
        camera.nearPlane = aicamera->mClipPlaneNear;
        camera.farPlane = aicamera->mClipPlaneFar;
        camera.aspect = aicamera->mAspect;

        if (camera.aspect == 0.0f)
        {
            camera.aspect = 1.0f;
            camera.verticalFOVrad = camera.horizontalFOVrad;
        }
        else
            camera.verticalFOVrad = camera.horizontalFOVrad / camera.aspect;

        fprintf(stderr, "         pos: %f, %f, %f\n", camera.pos.x, camera.pos.y, camera.pos.z);
        fprintf(stderr, "         up: %f, %f, %f\n", camera.up.x, camera.up.y, camera.up.z);
        fprintf(stderr, "         forward: %f, %f, %f\n", camera.forward.x, camera.forward.y, camera.forward.z);
        fprintf(stderr, "         nearPlane: %f\n", camera.nearPlane);
        fprintf(stderr, "         farPlane: %f\n", camera.farPlane);
        fprintf(stderr, "         aspect: %f\n", camera.aspect);
        fprintf(stderr, "         horizontalFOV: %f\n", MathCore::OP<float>::rad_2_deg(camera.horizontalFOVrad));
        fprintf(stderr, "         verticalFOV: %f\n", MathCore::OP<float>::rad_2_deg(camera.verticalFOVrad));

        result->cameras.push_back(camera);
    }

    //
    // LIGHT EXPORTING
    //

    for (uint32_t i = 0; i < scene->mNumLights; i++)
    {
        fprintf(stdout, "[Light] %i / %i\n", i + 1, scene->mNumLights);

        aiLight *ailight = scene->mLights[i];

        ITKExtension::Model::Light light;

        light.name = ailight->mName.data;

        fprintf(stdout, "        name => %s\n", ailight->mName.data);

        light.type = ITKExtension::Model::LightType_NONE;
        switch (ailight->mType)
        {
        case aiLightSource_UNDEFINED:
            light.type = ITKExtension::Model::LightType_NONE;
            break;
        case aiLightSource_DIRECTIONAL:
            light.type = ITKExtension::Model::LightType_DIRECTIONAL;
            break;
        case aiLightSource_POINT:
            light.type = ITKExtension::Model::LightType_POINT;
            break;
        case aiLightSource_SPOT:
            light.type = ITKExtension::Model::LightType_SPOT;
            break;
        case aiLightSource_AMBIENT:
            light.type = ITKExtension::Model::LightType_AMBIENT;
            break;
        case aiLightSource_AREA:
            light.type = ITKExtension::Model::LightType_AREA;
            break;
        }

        // common attrs
        light.attenuationConstant = ailight->mAttenuationConstant;
        light.attenuationLinear = ailight->mAttenuationLinear;
        light.attenuationQuadratic = ailight->mAttenuationQuadratic;

        fprintf(stdout, "        attenuationConstant => %f\n", light.attenuationConstant);
        fprintf(stdout, "        attenuationLinear => %f\n", light.attenuationLinear);
        fprintf(stdout, "        attenuationQuadratic => %f\n", light.attenuationQuadratic);

        light.colorDiffuse = MathCore::vec3f(ailight->mColorDiffuse.r, ailight->mColorDiffuse.g, ailight->mColorDiffuse.b);
        light.colorSpecular = MathCore::vec3f(ailight->mColorSpecular.r, ailight->mColorSpecular.g, ailight->mColorSpecular.b);
        light.colorAmbient = MathCore::vec3f(ailight->mColorAmbient.r, ailight->mColorAmbient.g, ailight->mColorAmbient.b);

        fprintf(stdout, "        colorDiffuse => vec3( %f, %f, %f )\n", light.colorDiffuse.r, light.colorDiffuse.g, light.colorDiffuse.b);
        fprintf(stdout, "        colorSpecular => vec3( %f, %f, %f )\n", light.colorSpecular.r, light.colorSpecular.g, light.colorSpecular.b);
        fprintf(stdout, "        colorAmbient => vec3( %f, %f, %f )\n", light.colorAmbient.r, light.colorAmbient.g, light.colorAmbient.b);

        fprintf(stdout, "        type => %s\n", ITKExtension::Model::LightTypeToStr(light.type));

        switch (light.type)
        {
        case ITKExtension::Model::LightType_NONE:
            break;
        case ITKExtension::Model::LightType_DIRECTIONAL:
            light.directional.direction = MathCore::vec3f(ailight->mDirection.x, ailight->mDirection.y, ailight->mDirection.z);
            light.directional.up = MathCore::vec3f(ailight->mUp.x, ailight->mUp.y, ailight->mUp.z);

            fprintf(stdout, "        direction => vec3( %f, %f, %f )\n", light.directional.direction.x, light.directional.direction.y, light.directional.direction.z);
            fprintf(stdout, "        up => vec3( %f, %f, %f )\n", light.directional.up.x, light.directional.up.y, light.directional.up.z);

            break;
        case ITKExtension::Model::LightType_POINT:
            light.point.position = MathCore::vec3f(ailight->mPosition.x, ailight->mPosition.y, ailight->mPosition.z);

            fprintf(stdout, "        position => vec3( %f, %f, %f )\n", light.point.position.x, light.point.position.y, light.point.position.z);

            break;
        case ITKExtension::Model::LightType_SPOT:
            light.spot.position = MathCore::vec3f(ailight->mPosition.x, ailight->mPosition.y, ailight->mPosition.z);
            light.spot.direction = MathCore::vec3f(ailight->mDirection.x, ailight->mDirection.y, ailight->mDirection.z);
            light.spot.up = MathCore::vec3f(ailight->mUp.x, ailight->mUp.y, ailight->mUp.z);
            light.spot.angleInnerCone = ailight->mAngleInnerCone;
            light.spot.angleOuterCone = ailight->mAngleOuterCone;

            fprintf(stdout, "        position => vec3( %f, %f, %f )\n", light.spot.position.x, light.spot.position.y, light.spot.position.z);
            fprintf(stdout, "        direction => vec3( %f, %f, %f )\n", light.spot.direction.x, light.spot.direction.y, light.spot.direction.z);
            fprintf(stdout, "        position => vec3( %f, %f, %f )\n", light.spot.up.x, light.spot.up.y, light.spot.up.z);
            fprintf(stdout, "        angleInnerCone => %f\n", light.spot.angleInnerCone);
            fprintf(stdout, "        angleOuterCone => %f\n", light.spot.angleOuterCone);

            break;
        case ITKExtension::Model::LightType_AMBIENT:
            light.ambient.position = MathCore::vec3f(ailight->mPosition.x, ailight->mPosition.y, ailight->mPosition.z);
            light.ambient.direction = MathCore::vec3f(ailight->mDirection.x, ailight->mDirection.y, ailight->mDirection.z);
            light.ambient.up = MathCore::vec3f(ailight->mUp.x, ailight->mUp.y, ailight->mUp.z);

            fprintf(stdout, "        position => vec3( %f, %f, %f )\n", light.ambient.position.x, light.ambient.position.y, light.ambient.position.z);
            fprintf(stdout, "        direction => vec3( %f, %f, %f )\n", light.ambient.direction.x, light.ambient.direction.y, light.ambient.direction.z);
            fprintf(stdout, "        position => vec3( %f, %f, %f )\n", light.ambient.up.x, light.ambient.up.y, light.ambient.up.z);

            break;
        case ITKExtension::Model::LightType_AREA:
            light.area.position = MathCore::vec3f(ailight->mPosition.x, ailight->mPosition.y, ailight->mPosition.z);
            light.area.direction = MathCore::vec3f(ailight->mDirection.x, ailight->mDirection.y, ailight->mDirection.z);
            light.area.up = MathCore::vec3f(ailight->mUp.x, ailight->mUp.y, ailight->mUp.z);
            light.area.size = MathCore::vec2f(ailight->mSize.x, ailight->mSize.y);

            fprintf(stdout, "        position => vec3( %f, %f, %f )\n", light.area.position.x, light.area.position.y, light.area.position.z);
            fprintf(stdout, "        direction => vec3( %f, %f, %f )\n", light.area.direction.x, light.area.direction.y, light.area.direction.z);
            fprintf(stdout, "        position => vec3( %f, %f, %f )\n", light.area.up.x, light.area.up.y, light.area.up.z);
            fprintf(stdout, "        size => vec3( %f, %f )\n", light.area.size.x, light.area.size.y);

            break;
        }

        result->lights.push_back(light);
    }

    //
    // TEXTURE EXPORTING
    //

    for (uint32_t i = 0; i < scene->mNumTextures; i++)
    {
        fprintf(stdout, "[Texture] %i / %i\n", i + 1, scene->mNumTextures);
        fprintf(stderr, "[Texture] not implemented...\n");
    }

    //
    // MATERIAL EXPORTING
    //

    for (uint32_t i = 0; i < scene->mNumMaterials; i++)
    {
        fprintf(stdout, "[Material] %i / %i\n", i + 1, scene->mNumMaterials);
        aiMaterial *aimaterial = scene->mMaterials[i];

        ITKExtension::Model::Material material;

        for (uint32_t j = 0; j < aimaterial->mNumProperties; j++)
        {
            aiMaterialProperty *materialProperty = aimaterial->mProperties[j];

            if (starts_with(materialProperty->mKey.data, "$tex."))
            {
                fprintf(stdout, "         %s => processed ahead...\n", materialProperty->mKey.data);
                continue;
            }
            else if (starts_with(materialProperty->mKey.data, "$raw."))
            {
                fprintf(stdout, "         %s => not processed...\n", materialProperty->mKey.data);
                continue;
            }
            else if (materialProperty->mType == aiPTI_String)
            {
                aiString str;
                if (aimaterial->Get(materialProperty->mKey.data, materialProperty->mSemantic, materialProperty->mIndex, str) == aiReturn_SUCCESS)
                {
                    fprintf(stdout, "         %s => %s\n", materialProperty->mKey.data, str.data);
                    if (strcmp(materialProperty->mKey.data, "?mat.name") == 0)
                    {
                        material.name = str.data;
                        fprintf(stdout, "               storing material name\n");
                    }
                    else
                    {
                        fprintf(stdout, "               not stored\n");
                    }
                }
                else
                {
                    fprintf(stdout, "               ERROR TO READ VALUE...\n");
                }
            }
            else if (materialProperty->mType == aiPTI_Float)
            {
                ai_real floats[16];
                unsigned int iMax = 16;

                aiReturn eRet = aiGetMaterialFloatArray(aimaterial, materialProperty->mKey.data, materialProperty->mSemantic, materialProperty->mIndex, floats, &iMax);

                if (eRet == aiReturn_SUCCESS)
                {
                    // color 3 or 4 floats check
                    if (iMax == 3 && (starts_with(materialProperty->mKey.data, "$clr.")))
                    {
                        iMax = 4;
                        floats[3] = 1.0f;
                    }

                    if (iMax == 1)
                    {
                        float v = floats[0];
                        fprintf(stdout, "         %s => float (%f)\n", materialProperty->mKey.data, v);
                        if (starts_with(materialProperty->mKey.data, "$mat."))
                        {
                            fprintf(stdout, "               storing name: %s\n", &materialProperty->mKey.data[strlen("$mat.")]);
                            material.floatValue[&materialProperty->mKey.data[strlen("$mat.")]] = v;
                        }
                        else
                            material.floatValue[materialProperty->mKey.data] = v;
                    }
                    else if (iMax == 2)
                    {
                        MathCore::vec2f v(floats[0], floats[1]);
                        fprintf(stdout, "         %s => vec2 (%f, %f)\n", materialProperty->mKey.data, v.x, v.y);
                        if (starts_with(materialProperty->mKey.data, "$mat."))
                        {
                            fprintf(stdout, "               storing name: %s\n", &materialProperty->mKey.data[strlen("$mat.")]);
                            material.vec2Value[&materialProperty->mKey.data[strlen("$mat.")]] = v;
                        }
                        else
                            material.vec2Value[materialProperty->mKey.data] = v;
                    }
                    else if (iMax == 3)
                    {
                        MathCore::vec3f v(floats[0], floats[1], floats[2]);
                        fprintf(stdout, "         %s => vec3 (%f, %f, %f)\n", materialProperty->mKey.data, v.x, v.y, v.z);
                        if (starts_with(materialProperty->mKey.data, "$mat."))
                        {
                            fprintf(stdout, "               storing name: %s\n", &materialProperty->mKey.data[strlen("$mat.")]);
                            material.vec3Value[&materialProperty->mKey.data[strlen("$mat.")]] = v;
                        }
                        else
                            material.vec3Value[materialProperty->mKey.data] = v;
                    }
                    else if (iMax == 4)
                    {
                        MathCore::vec4f v(floats[0], floats[1], floats[2], floats[3]);
                        fprintf(stdout, "         %s => vec4 (%f, %f, %f, %f)\n", materialProperty->mKey.data, v.x, v.y, v.z, v.w);
                        if (starts_with(materialProperty->mKey.data, "$mat."))
                        {
                            fprintf(stdout, "               storing name: %s\n", &materialProperty->mKey.data[strlen("$mat.")]);
                            material.vec4Value[&materialProperty->mKey.data[strlen("$mat.")]] = v;
                        }
                        else if (starts_with(materialProperty->mKey.data, "$clr."))
                        {
                            fprintf(stdout, "               storing name: %s\n", &materialProperty->mKey.data[strlen("$clr.")]);
                            material.vec4Value[&materialProperty->mKey.data[strlen("$clr.")]] = v;
                        }
                        else
                            material.vec4Value[materialProperty->mKey.data] = v;
                    }
                    else if (iMax == 16)
                    {
                        /*
                         mat4 m = mat4(
                         floats[0], floats[4], floats[8], floats[12],
                         floats[1], floats[5], floats[9], floats[13],
                         floats[2], floats[6], floats[10], floats[14],
                         floats[3], floats[7], floats[11], floats[15]
                         );
                         */
                        fprintf(stdout, "         %s => mat4 not implemented (...)\n", materialProperty->mKey.data);
                    }
                    else
                    {
                        fprintf(stdout, "         %s => float(%i) not implemented...\n", materialProperty->mKey.data, iMax);
                    }
                }
                else
                {
                    fprintf(stdout, "         %s => ERROR TO READ VALUE...\n", materialProperty->mKey.data);
                }
            }
            else if (materialProperty->mType == aiPTI_Integer)
            {
                int integers[16];
                unsigned int iMax = 16;

                aiReturn eRet = aiGetMaterialIntegerArray(aimaterial, materialProperty->mKey.data, materialProperty->mSemantic, materialProperty->mIndex, integers, &iMax);
                if (eRet == aiReturn_SUCCESS)
                {
                    if (iMax == 1)
                    {
                        int v = integers[0];
                        fprintf(stdout, "         %s => int (%i) \n", materialProperty->mKey.data, v);

                        if (starts_with(materialProperty->mKey.data, "$mat."))
                        {
                            fprintf(stdout, "               storing name: %s\n", &materialProperty->mKey.data[strlen("$mat.")]);
                            material.intValue[&materialProperty->mKey.data[strlen("$mat.")]] = v;
                        }
                        else
                            material.intValue[materialProperty->mKey.data] = v;
                    }
                    else
                    {
                        fprintf(stdout, "         %s => int(%i) not implemented...\n", materialProperty->mKey.data, iMax);
                    }
                }
                else
                {
                    fprintf(stdout, "         %s => ERROR TO READ VALUE...\n", materialProperty->mKey.data);
                }
            }

            else
            {
                fprintf(stdout, "         %s => not processed...\n", materialProperty->mKey.data);
            }
        }

        processTextureType(material, aimaterial, aiTextureType_DIFFUSE, ITKExtension::Model::TextureType_DIFFUSE);
        processTextureType(material, aimaterial, aiTextureType_SPECULAR, ITKExtension::Model::TextureType_SPECULAR);
        processTextureType(material, aimaterial, aiTextureType_AMBIENT, ITKExtension::Model::TextureType_AMBIENT);
        processTextureType(material, aimaterial, aiTextureType_EMISSIVE, ITKExtension::Model::TextureType_EMISSIVE);
        processTextureType(material, aimaterial, aiTextureType_HEIGHT, ITKExtension::Model::TextureType_HEIGHT);
        processTextureType(material, aimaterial, aiTextureType_NORMALS, ITKExtension::Model::TextureType_NORMALS);
        processTextureType(material, aimaterial, aiTextureType_SHININESS, ITKExtension::Model::TextureType_SHININESS);
        processTextureType(material, aimaterial, aiTextureType_OPACITY, ITKExtension::Model::TextureType_OPACITY);
        processTextureType(material, aimaterial, aiTextureType_DISPLACEMENT, ITKExtension::Model::TextureType_DISPLACEMENT);
        processTextureType(material, aimaterial, aiTextureType_LIGHTMAP, ITKExtension::Model::TextureType_LIGHTMAP);
        processTextureType(material, aimaterial, aiTextureType_REFLECTION, ITKExtension::Model::TextureType_REFLECTION);
        processTextureType(material, aimaterial, aiTextureType_UNKNOWN, ITKExtension::Model::TextureType_UNKNOWN);

        result->materials.push_back(material);
    }

    //
    // NODES EXPORTING (Hierarchy)
    //

    fprintf(stdout, "[Hierarchy]\n");
    aiNode *node = scene->mRootNode;
    std::string output = " ";
    recursiveInsertNodes(result, node, output, -1, leftHanded);

    aiReleaseImport(scene);

    return result;
}
