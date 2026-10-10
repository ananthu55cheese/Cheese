#include "Model.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include <algorithm>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>


// ============================================================
// CONSTRUCTOR
// ============================================================

Model::Model(const char* file)
{
    // Load JSON
    std::string text = get_file_contents(file);

    if (text.empty())
    {
        throw std::runtime_error(
            std::string("Could not read model file: ") + file
        );
    }

    JSON = json::parse(text);

    // Store model filename
    Model::file = file;

    // Load binary buffer
    data = getData();

    // --------------------------------------------------------
    // Get active scene
    // --------------------------------------------------------

    unsigned int sceneIndex = JSON.value("scene", 0);

    if (!JSON.contains("scenes") ||
        JSON["scenes"].empty())
    {
        throw std::runtime_error(
            "Model contains no scenes."
        );
    }

    if (sceneIndex >= JSON["scenes"].size())
    {
        throw std::runtime_error(
            "Invalid scene index."
        );
    }

    json scene = JSON["scenes"][sceneIndex];

    // --------------------------------------------------------
    // Traverse every root node
    // --------------------------------------------------------

    if (scene.contains("nodes"))
    {
        for (unsigned int nodeIndex : scene["nodes"])
        {
            traverseNode(nodeIndex);
        }
    }

    
}


// ============================================================
// DRAW
// ============================================================

void Model::Draw(
    Shader& shader,
    Camera& camera,
    glm::mat4 modelMatrix
)
{
   

    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        glm::mat4 finalMatrix =
            modelMatrix * matricesMeshes[i];

        meshes[i].Mesh::Draw(
            shader,
            camera,
            finalMatrix
        );
    }
}


// ============================================================
// LOAD MESH
// ============================================================

void Model::loadMesh(unsigned int indMesh)
{
    std::cout
        << "LOADING MESH: "
        << indMesh
        << std::endl;

    if (!JSON.contains("meshes") ||
        indMesh >= JSON["meshes"].size())
    {
        std::cerr
            << "Invalid mesh index: "
            << indMesh
            << std::endl;

        return;
    }

    json meshJSON = JSON["meshes"][indMesh];

    if (!meshJSON.contains("primitives"))
    {
        std::cerr
            << "Mesh has no primitives: "
            << indMesh
            << std::endl;

        return;
    }

    // --------------------------------------------------------
    // A glTF mesh can contain multiple primitives
    // --------------------------------------------------------

    for (unsigned int primitiveIndex = 0;
        primitiveIndex < meshJSON["primitives"].size();
        primitiveIndex++)
    {
        json primitive =
            meshJSON["primitives"][primitiveIndex];

        std::cout
            << "  PRIMITIVE: "
            << primitiveIndex
            << std::endl;

        // ----------------------------------------------------
        // Only triangles are supported by this Mesh class
        // ----------------------------------------------------

        unsigned int mode =
            primitive.value("mode", 4);

        if (mode != 4)
        {
            std::cerr
                << "  WARNING: Primitive mode "
                << mode
                << " is not TRIANGLES. Skipping.\n";

            continue;
        }

        if (!primitive.contains("attributes"))
        {
            std::cerr
                << "  Primitive has no attributes.\n";

            continue;
        }

        json attributes =
            primitive["attributes"];

        // ----------------------------------------------------
        // POSITION is required
        // ----------------------------------------------------

        if (!attributes.contains("POSITION"))
        {
            std::cerr
                << "  Primitive has no POSITION.\n";

            continue;
        }

        unsigned int posAccInd =
            attributes["POSITION"];

        std::vector<float> posVec =
            getFloats(JSON["accessors"][posAccInd]);

        std::vector<glm::vec3> positions =
            groupFloatsVec3(posVec);

        if (positions.empty())
        {
            std::cerr
                << "  No positions found.\n";

            continue;
        }

        // ----------------------------------------------------
        // NORMAL
        // ----------------------------------------------------

        std::vector<glm::vec3> normals;

        if (attributes.contains("NORMAL"))
        {
            unsigned int normalAccInd =
                attributes["NORMAL"];

            std::vector<float> normalVec =
                getFloats(JSON["accessors"][normalAccInd]);

            normals =
                groupFloatsVec3(normalVec);
        }

        // ----------------------------------------------------
        // TEXCOORD_0
        // ----------------------------------------------------

        std::vector<glm::vec2> texUVs;

        if (attributes.contains("TEXCOORD_0"))
        {
            unsigned int texAccInd =
                attributes["TEXCOORD_0"];

            std::vector<float> texVec =
                getFloats(JSON["accessors"][texAccInd]);

            texUVs =
                groupFloatsVec2(texVec);
        }

        // ----------------------------------------------------
        // Generate missing normals
        // ----------------------------------------------------

        if (normals.size() != positions.size())
        {
            normals.resize(
                positions.size(),
                glm::vec3(0.0f)
            );

            // Generate flat normals from triangles later
            // if indices are available.
        }

        // ----------------------------------------------------
        // Generate missing UVs
        // ----------------------------------------------------

        if (texUVs.size() != positions.size())
        {
            texUVs.resize(
                positions.size(),
                glm::vec2(0.0f)
            );
        }

        // ----------------------------------------------------
        // INDICES
        // ----------------------------------------------------

        std::vector<GLuint> indices;

        if (primitive.contains("indices"))
        {
            unsigned int indAccInd =
                primitive["indices"];

            indices =
                getIndices(JSON["accessors"][indAccInd]);
        }
        else
        {
            // Non-indexed primitive
            indices.reserve(positions.size());

            for (unsigned int i = 0;
                i < positions.size();
                i++)
            {
                indices.push_back(i);
            }
        }

        // ----------------------------------------------------
        // Generate normals if missing
        // ----------------------------------------------------

        bool normalsMissing = true;

        if (attributes.contains("NORMAL") &&
            normals.size() == positions.size())
        {
            normalsMissing = false;
        }

        if (normalsMissing)
        {
            normals.assign(
                positions.size(),
                glm::vec3(0.0f)
            );

            for (unsigned int i = 0;
                i + 2 < indices.size();
                i += 3)
            {
                GLuint i0 = indices[i];
                GLuint i1 = indices[i + 1];
                GLuint i2 = indices[i + 2];

                if (i0 >= positions.size() ||
                    i1 >= positions.size() ||
                    i2 >= positions.size())
                {
                    continue;
                }

                glm::vec3 edge1 =
                    positions[i1] - positions[i0];

                glm::vec3 edge2 =
                    positions[i2] - positions[i0];

                glm::vec3 normal =
                    glm::cross(edge1, edge2);

                normals[i0] += normal;
                normals[i1] += normal;
                normals[i2] += normal;
            }

            for (glm::vec3& normal : normals)
            {
                if (glm::length(normal) > 0.00001f)
                {
                    normal =
                        glm::normalize(normal);
                }
                else
                {
                    normal =
                        glm::vec3(0.0f, 1.0f, 0.0f);
                }
            }
        }

        // ----------------------------------------------------
        // Assemble vertices
        // ----------------------------------------------------

        std::vector<Vertex> vertices =
            assembleVertices(
                positions,
                normals,
                texUVs
            );

        // ----------------------------------------------------
        // Textures
        // ----------------------------------------------------

        std::vector<Texture> textures =
            getTextures();

        std::cout
            << "  VERTICES: "
            << vertices.size()
            << std::endl;

        std::cout
            << "  INDICES: "
            << indices.size()
            << std::endl;

        std::cout
            << "  TEXTURES: "
            << textures.size()
            << std::endl;

        // ----------------------------------------------------
        // Create Mesh
        // ----------------------------------------------------

        meshes.push_back(
            Mesh(
                vertices,
                indices,
                textures
            )
        );
    }
}


// ============================================================
// TRAVERSE NODE
// ============================================================

void Model::traverseNode(
    unsigned int nextNode,
    glm::mat4 matrix
)
{
    if (!JSON.contains("nodes") ||
        nextNode >= JSON["nodes"].size())
    {
        std::cerr
            << "Invalid node index: "
            << nextNode
            << std::endl;

        return;
    }

    // Current node
    json node =
        JSON["nodes"][nextNode];

    // --------------------------------------------------------
    // Translation
    // --------------------------------------------------------

    glm::vec3 translation(
        0.0f,
        0.0f,
        0.0f
    );

    if (node.contains("translation"))
    {
        translation =
            glm::vec3(
                node["translation"][0],
                node["translation"][1],
                node["translation"][2]
            );
    }

    // --------------------------------------------------------
    // Rotation
    // glTF quaternion order:
    // x, y, z, w
    // --------------------------------------------------------

    glm::quat rotation(
        1.0f,
        0.0f,
        0.0f,
        0.0f
    );

    if (node.contains("rotation"))
    {
        rotation =
            glm::quat(
                node["rotation"][3],
                node["rotation"][0],
                node["rotation"][1],
                node["rotation"][2]
            );
    }

    // --------------------------------------------------------
    // Scale
    // --------------------------------------------------------

    glm::vec3 scale(
        1.0f,
        1.0f,
        1.0f
    );

    if (node.contains("scale"))
    {
        scale =
            glm::vec3(
                node["scale"][0],
                node["scale"][1],
                node["scale"][2]
            );
    }

    // --------------------------------------------------------
    // Matrix
    // --------------------------------------------------------

    glm::mat4 matNode(
        1.0f
    );

    if (node.contains("matrix"))
    {
        float matValues[16];

        for (unsigned int i = 0;
            i < 16;
            i++)
        {
            matValues[i] =
                node["matrix"][i];
        }

        matNode =
            glm::make_mat4(
                matValues
            );
    }

    // --------------------------------------------------------
    // Build local transform
    // --------------------------------------------------------

    glm::mat4 trans(
        1.0f
    );

    glm::mat4 rot(
        1.0f
    );

    glm::mat4 sca(
        1.0f
    );

    trans =
        glm::translate(
            trans,
            translation
        );

    rot =
        glm::mat4_cast(
            rotation
        );

    sca =
        glm::scale(
            sca,
            scale
        );

    // --------------------------------------------------------
    // glTF node transform
    // --------------------------------------------------------

    glm::mat4 matNextNode;

    if (node.contains("matrix"))
    {
        matNextNode =
            matrix * matNode;
    }
    else
    {
        matNextNode =
            matrix *
            trans *
            rot *
            sca;
    }

    // --------------------------------------------------------
    // Mesh
    // --------------------------------------------------------

    if (node.contains("mesh"))
    {
        unsigned int meshIndex =
            node["mesh"];

        std::cout
            << "FOUND MESH ON NODE: "
            << nextNode
            << " -> MESH "
            << meshIndex
            << std::endl;

        translationsMeshes.push_back(
            translation
        );

        rotationsMeshes.push_back(
            rotation
        );

        scalesMeshes.push_back(
            scale
        );

        matricesMeshes.push_back(
            matNextNode
        );

        loadMesh(meshIndex);
    }

    // --------------------------------------------------------
    // Children
    // --------------------------------------------------------

    if (node.contains("children"))
    {
        for (unsigned int i = 0;
            i < node["children"].size();
            i++)
        {
            traverseNode(
                node["children"][i],
                matNextNode
            );
        }
    }
}


// ============================================================
// GET BINARY DATA
// ============================================================

std::vector<unsigned char> Model::getData()
{
    std::vector<unsigned char> result;

    if (!JSON.contains("buffers") ||
        JSON["buffers"].empty())
    {
        throw std::runtime_error(
            "Model contains no buffers."
        );
    }

    json buffer =
        JSON["buffers"][0];

    // --------------------------------------------------------
    // External .bin file
    // --------------------------------------------------------

    if (buffer.contains("uri"))
    {
        std::string uri =
            buffer["uri"];

        std::string fileStr =
            std::string(file);

        size_t slash =
            fileStr.find_last_of(
                "/\\"
            );

        std::string fileDirectory;

        if (slash != std::string::npos)
        {
            fileDirectory =
                fileStr.substr(
                    0,
                    slash + 1
                );
        }

        std::string binPath =
            fileDirectory + uri;

        std::cout
            << "LOADING BIN: "
            << binPath
            << std::endl;

        std::ifstream binFile(
            binPath,
            std::ios::binary
        );

        if (!binFile)
        {
            throw std::runtime_error(
                "Could not open binary file: " +
                binPath
            );
        }

        binFile.seekg(
            0,
            std::ios::end
        );

        std::streamsize size =
            binFile.tellg();

        binFile.seekg(
            0,
            std::ios::beg
        );

        result.resize(
            static_cast<size_t>(size)
        );

        if (size > 0)
        {
            binFile.read(
                reinterpret_cast<char*>(
                    result.data()
                    ),
                size
            );
        }

        return result;
    }

    // --------------------------------------------------------
    // Embedded base64 buffer
    // --------------------------------------------------------

    if (buffer.contains("uri"))
    {
        std::string uri =
            buffer["uri"];

        const std::string prefix =
            "data:application/octet-stream;base64,";

        if (uri.rfind(prefix, 0) == 0)
        {
            std::string encoded =
                uri.substr(prefix.length());

            // Base64 decoder
            static const std::string chars =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";

            int val = 0;
            int valb = -8;

            for (unsigned char c : encoded)
            {
                if (c == '=')
                    break;

                size_t pos =
                    chars.find(c);

                if (pos == std::string::npos)
                    continue;

                val =
                    (val << 6) + pos;

                valb += 6;

                if (valb >= 0)
                {
                    result.push_back(
                        static_cast<unsigned char>(
                            (val >> valb) & 0xFF
                            )
                    );

                    valb -= 8;
                }
            }

            return result;
        }
    }

    // --------------------------------------------------------
    // No URI means GLB or unsupported buffer
    // --------------------------------------------------------

    throw std::runtime_error(
        "Buffer has no URI. "
        "GLB loading is not supported by this Model.cpp."
    );
}


// ============================================================
// GET FLOATS
// ============================================================

std::vector<float> Model::getFloats(
    json accessor
)
{
    std::vector<float> floatVec;

    if (!accessor.contains("bufferView"))
    {
        return floatVec;
    }

    unsigned int buffViewInd =
        accessor["bufferView"];

    unsigned int count =
        accessor["count"];

    unsigned int accByteOffset =
        accessor.value(
            "byteOffset",
            0
        );

    std::string type =
        accessor["type"];

    unsigned int numPerVert = 0;

    if (type == "SCALAR")
        numPerVert = 1;
    else if (type == "VEC2")
        numPerVert = 2;
    else if (type == "VEC3")
        numPerVert = 3;
    else if (type == "VEC4")
        numPerVert = 4;
    else
        throw std::invalid_argument(
            "Invalid accessor type: " +
            type
        );

    json bufferView =
        JSON["bufferViews"][buffViewInd];

    unsigned int byteOffset =
        bufferView.value(
            "byteOffset",
            0
        );

    unsigned int byteStride =
        bufferView.value(
            "byteStride",
            numPerVert * sizeof(float)
        );

    unsigned int componentType =
        accessor.value(
            "componentType",
            5126
        );

    unsigned int componentSize = 4;

    switch (componentType)
    {
    case 5120: // BYTE
    case 5121: // UNSIGNED_BYTE
        componentSize = 1;
        break;

    case 5122: // SHORT
    case 5123: // UNSIGNED_SHORT
        componentSize = 2;
        break;

    case 5125: // UNSIGNED_INT
    case 5126: // FLOAT
        componentSize = 4;
        break;

    default:
        throw std::runtime_error(
            "Unsupported accessor component type."
        );
    }

    unsigned int beginningOfData =
        byteOffset +
        accByteOffset;

    floatVec.reserve(
        count * numPerVert
    );

    for (unsigned int i = 0;
        i < count;
        i++)
    {
        unsigned int vertexOffset =
            beginningOfData +
            i * byteStride;

        for (unsigned int j = 0;
            j < numPerVert;
            j++)
        {
            unsigned int offset =
                vertexOffset +
                j * componentSize;

            if (offset + componentSize >
                data.size())
            {
                throw std::runtime_error(
                    "Accessor reads outside binary buffer."
                );
            }

            float value = 0.0f;

            switch (componentType)
            {
            case 5126:
            {
                float temp;

                std::memcpy(
                    &temp,
                    &data[offset],
                    sizeof(float)
                );

                value = temp;
                break;
            }

            case 5120:
            {
                int8_t temp;

                std::memcpy(
                    &temp,
                    &data[offset],
                    sizeof(int8_t)
                );

                value =
                    static_cast<float>(
                        temp
                        );

                break;
            }

            case 5121:
            {
                uint8_t temp;

                std::memcpy(
                    &temp,
                    &data[offset],
                    sizeof(uint8_t)
                );

                value =
                    static_cast<float>(
                        temp
                        );

                break;
            }

            case 5122:
            {
                int16_t temp;

                std::memcpy(
                    &temp,
                    &data[offset],
                    sizeof(int16_t)
                );

                value =
                    static_cast<float>(
                        temp
                        );

                break;
            }

            case 5123:
            {
                uint16_t temp;

                std::memcpy(
                    &temp,
                    &data[offset],
                    sizeof(uint16_t)
                );

                value =
                    static_cast<float>(
                        temp
                        );

                break;
            }

            case 5125:
            {
                uint32_t temp;

                std::memcpy(
                    &temp,
                    &data[offset],
                    sizeof(uint32_t)
                );

                value =
                    static_cast<float>(
                        temp
                        );

                break;
            }
            }

            floatVec.push_back(value);
        }
    }

    return floatVec;
}


// ============================================================
// GET INDICES
// ============================================================

std::vector<GLuint> Model::getIndices(
    json accessor
)
{
    std::vector<GLuint> indices;

    if (!accessor.contains("bufferView"))
    {
        return indices;
    }

    unsigned int buffViewInd =
        accessor["bufferView"];

    unsigned int count =
        accessor["count"];

    unsigned int accByteOffset =
        accessor.value(
            "byteOffset",
            0
        );

    unsigned int componentType =
        accessor["componentType"];

    json bufferView =
        JSON["bufferViews"][buffViewInd];

    unsigned int byteOffset =
        bufferView.value(
            "byteOffset",
            0
        );

    unsigned int componentSize;

    if (componentType == 5121)
        componentSize = 1;
    else if (componentType == 5123)
        componentSize = 2;
    else if (componentType == 5125)
        componentSize = 4;
    else
        throw std::runtime_error(
            "Unsupported index component type."
        );

    unsigned int byteStride =
        bufferView.value(
            "byteStride",
            componentSize
        );

    unsigned int beginningOfData =
        byteOffset +
        accByteOffset;

    indices.reserve(count);

    for (unsigned int i = 0;
        i < count;
        i++)
    {
        unsigned int offset =
            beginningOfData +
            i * byteStride;

        if (offset + componentSize >
            data.size())
        {
            throw std::runtime_error(
                "Index accessor reads outside binary buffer."
            );
        }

        GLuint value = 0;

        if (componentType == 5121)
        {
            uint8_t temp;

            std::memcpy(
                &temp,
                &data[offset],
                sizeof(uint8_t)
            );

            value = temp;
        }
        else if (componentType == 5123)
        {
            uint16_t temp;

            std::memcpy(
                &temp,
                &data[offset],
                sizeof(uint16_t)
            );

            value = temp;
        }
        else if (componentType == 5125)
        {
            uint32_t temp;

            std::memcpy(
                &temp,
                &data[offset],
                sizeof(uint32_t)
            );

            value = temp;
        }

        indices.push_back(value);
    }

    return indices;
}


// ============================================================
// GET TEXTURES
// ============================================================

std::vector<Texture> Model::getTextures()
{
    std::vector<Texture> textures;

    if (!JSON.contains("images"))
    {
        return textures;
    }

    std::string fileStr =
        std::string(file);

    size_t slash =
        fileStr.find_last_of(
            "/\\"
        );

    std::string fileDirectory;

    if (slash != std::string::npos)
    {
        fileDirectory =
            fileStr.substr(
                0,
                slash + 1
            );
    }

    // --------------------------------------------------------
    // Load all images that have URI paths
    // --------------------------------------------------------

    for (unsigned int i = 0;
        i < JSON["images"].size();
        i++)
    {
        json image =
            JSON["images"][i];

        if (!image.contains("uri"))
        {
            // Embedded image / bufferView
            // is not handled by the current
            // Texture class.
            continue;
        }

        std::string texPath =
            image["uri"];

        // ----------------------------------------------------
        // Check if already loaded
        // ----------------------------------------------------

        bool skip = false;

        for (unsigned int j = 0;
            j < loadedTexName.size();
            j++)
        {
            if (loadedTexName[j] == texPath)
            {
                textures.push_back(
                    loadedTex[j]
                );

                skip = true;
                break;
            }
        }

        if (skip)
            continue;

        // ----------------------------------------------------
        // Determine texture type
        //
        // This is still limited by the existing Texture class.
        // We use the filename only as a fallback.
        // ----------------------------------------------------

        std::string textureType =
            "diffuse";

        if (texPath.find(
            "metallicRoughness"
        ) != std::string::npos)
        {
            textureType =
                "specular";
        }

        Texture texture(
            (
                fileDirectory +
                texPath
                ).c_str(),
            textureType.c_str(),
            loadedTex.size()
        );

        textures.push_back(
            texture
        );

        loadedTex.push_back(
            texture
        );

        loadedTexName.push_back(
            texPath
        );
    }

    return textures;
}


// ============================================================
// ASSEMBLE VERTICES
// ============================================================

std::vector<Vertex> Model::assembleVertices(
    std::vector<glm::vec3> positions,
    std::vector<glm::vec3> normals,
    std::vector<glm::vec2> texUVs
)
{
    std::vector<Vertex> vertices;

    vertices.reserve(
        positions.size()
    );

    for (unsigned int i = 0;
        i < positions.size();
        i++)
    {
        glm::vec3 normal(
            0.0f,
            1.0f,
            0.0f
        );

        glm::vec2 uv(
            0.0f,
            0.0f
        );

        if (i < normals.size())
        {
            normal =
                normals[i];
        }

        if (i < texUVs.size())
        {
            uv =
                texUVs[i];
        }

        vertices.push_back(
            Vertex
            {
                positions[i],
                normal,
                glm::vec3(
                    1.0f,
                    1.0f,
                    1.0f
                ),
                uv
            }
        );
    }

    return vertices;
}


// ============================================================
// GROUP FLOATS -> VEC2
// ============================================================

std::vector<glm::vec2>
Model::groupFloatsVec2(
    std::vector<float> floatVec
)
{
    const unsigned int floatsPerVector = 2;

    std::vector<glm::vec2> vectors;

    vectors.reserve(
        floatVec.size() /
        floatsPerVector
    );

    for (unsigned int i = 0;
        i + 1 < floatVec.size();
        i += floatsPerVector)
    {
        vectors.push_back(
            glm::vec2(
                floatVec[i],
                floatVec[i + 1]
            )
        );
    }

    return vectors;
}


// ============================================================
// GROUP FLOATS -> VEC3
// ============================================================

std::vector<glm::vec3>
Model::groupFloatsVec3(
    std::vector<float> floatVec
)
{
    const unsigned int floatsPerVector = 3;

    std::vector<glm::vec3> vectors;

    vectors.reserve(
        floatVec.size() /
        floatsPerVector
    );

    for (unsigned int i = 0;
        i + 2 < floatVec.size();
        i += floatsPerVector)
    {
        vectors.push_back(
            glm::vec3(
                floatVec[i],
                floatVec[i + 1],
                floatVec[i + 2]
            )
        );
    }

    return vectors;
}


// ============================================================
// GROUP FLOATS -> VEC4
// ============================================================

std::vector<glm::vec4>
Model::groupFloatsVec4(
    std::vector<float> floatVec
)
{
    const unsigned int floatsPerVector = 4;

    std::vector<glm::vec4> vectors;

    vectors.reserve(
        floatVec.size() /
        floatsPerVector
    );

    for (unsigned int i = 0;
        i + 3 < floatVec.size();
        i += floatsPerVector)
    {
        vectors.push_back(
            glm::vec4(
                floatVec[i],
                floatVec[i + 1],
                floatVec[i + 2],
                floatVec[i + 3]
            )
        );
    }

    return vectors;
}