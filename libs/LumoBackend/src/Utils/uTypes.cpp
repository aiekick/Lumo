// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it. PVS-Studio Static Code Analyzer for C, C++ and C#:
// http://www.viva64.com

#include <LumoBackend/Utils/uTypes.h>

namespace uType {
/* Generic Types
genType: floats
genDType: double floats
genIType: signed integers
genUType: unsigned integers
genBType: booleans
mat: float matrices
dmat: double matrices
*/

bool IsTypeSplitable(const uTypeEnum& vType) {
    switch (vType) {
        case uTypeEnum::U_VEC2:
        case uTypeEnum::U_VEC2_ARRAY:
        case uTypeEnum::U_VEC3:
        case uTypeEnum::U_VEC3_ARRAY:
        case uTypeEnum::U_VEC4:
        case uTypeEnum::U_VEC4_ARRAY:
        case uTypeEnum::U_IVEC2:
        case uTypeEnum::U_IVEC3:
        case uTypeEnum::U_IVEC4:
        case uTypeEnum::U_UVEC2:
        case uTypeEnum::U_UVEC3:
        case uTypeEnum::U_UVEC4:
        case uTypeEnum::U_BVEC2:
        case uTypeEnum::U_BVEC3:
        case uTypeEnum::U_BVEC4:
        case uTypeEnum::U_MAT2:
        case uTypeEnum::U_MAT3:
        case uTypeEnum::U_MAT4:
        case uTypeEnum::U_MAT4_ARRAY:
        case uTypeEnum::U_PROGRAM: return true;

        case uTypeEnum::U_VOID:
        case uTypeEnum::U_FLOAT:
        case uTypeEnum::U_INT:
        case uTypeEnum::U_UINT:
        case uTypeEnum::U_BOOL:
        case uTypeEnum::U_IMAGE1D:
        case uTypeEnum::U_IMAGE2D:
        case uTypeEnum::U_IMAGE3D:
        case uTypeEnum::U_SAMPLER1D:
        case uTypeEnum::U_SAMPLER1D_ARRAY:
        case uTypeEnum::U_SAMPLER2D:
        case uTypeEnum::U_SAMPLER2D_ARRAY:
        case uTypeEnum::U_SAMPLER3D:
        case uTypeEnum::U_SAMPLER3D_ARRAY:
        case uTypeEnum::U_SAMPLER2D_TEXTUREARRAY:
        case uTypeEnum::U_SAMPLERCUBE:
        case uTypeEnum::U_FLOAT_ARRAY:
        case uTypeEnum::U_INT_ARRAY:
        case uTypeEnum::U_STRUCT:
        case uTypeEnum::U_TYPE:
        case uTypeEnum::U_VEC:
        case uTypeEnum::U_MAT:
        case uTypeEnum::U_TEXT:
        case uTypeEnum::U_FLOW:
        case uTypeEnum::U_GLOBAL:
        case uTypeEnum::U_Count: return false;
    }

    return false;
}

bool IsTypeCombinable(const uTypeEnum& vType) {
    switch (vType) {
        case uTypeEnum::U_FLOAT:
        case uTypeEnum::U_VEC2:
        case uTypeEnum::U_VEC3:
        case uTypeEnum::U_INT:
        case uTypeEnum::U_IVEC2:
        case uTypeEnum::U_IVEC3:
        case uTypeEnum::U_UINT:
        case uTypeEnum::U_UVEC2:
        case uTypeEnum::U_UVEC3:
        case uTypeEnum::U_BOOL:
        case uTypeEnum::U_BVEC2:
        case uTypeEnum::U_BVEC3:
        case uTypeEnum::U_MAT2:
        case uTypeEnum::U_MAT3: return true;

        case uTypeEnum::U_VOID:
        case uTypeEnum::U_VEC4:
        case uTypeEnum::U_IVEC4:
        case uTypeEnum::U_UVEC4:
        case uTypeEnum::U_BVEC4:
        case uTypeEnum::U_IMAGE1D:
        case uTypeEnum::U_IMAGE2D:
        case uTypeEnum::U_IMAGE3D:
        case uTypeEnum::U_SAMPLER1D:
        case uTypeEnum::U_SAMPLER1D_ARRAY:
        case uTypeEnum::U_SAMPLER2D:
        case uTypeEnum::U_SAMPLER2D_ARRAY:
        case uTypeEnum::U_SAMPLER3D:
        case uTypeEnum::U_SAMPLER3D_ARRAY:
        case uTypeEnum::U_SAMPLER2D_TEXTUREARRAY:
        case uTypeEnum::U_SAMPLERCUBE:
        case uTypeEnum::U_MAT4:
        case uTypeEnum::U_FLOAT_ARRAY:
        case uTypeEnum::U_VEC2_ARRAY:
        case uTypeEnum::U_VEC3_ARRAY:
        case uTypeEnum::U_VEC4_ARRAY:
        case uTypeEnum::U_INT_ARRAY:
        case uTypeEnum::U_MAT4_ARRAY:
        case uTypeEnum::U_STRUCT:
        case uTypeEnum::U_TYPE:
        case uTypeEnum::U_VEC:
        case uTypeEnum::U_MAT:
        case uTypeEnum::U_TEXT:
        case uTypeEnum::U_FLOW:
        case uTypeEnum::U_PROGRAM:
        case uTypeEnum::U_GLOBAL:
        case uTypeEnum::U_Count: return false;
    }

    return false;
}

std::string ConvertUniformsTypeEnumToString(const uTypeEnum& vType) {
    switch (vType) {
        case uTypeEnum::U_TEXT: return "text";
        case uTypeEnum::U_FLOAT:
        case uTypeEnum::U_FLOAT_ARRAY: return "float";
        case uTypeEnum::U_INT: return "int";
        case uTypeEnum::U_UINT: return "uint";
        case uTypeEnum::U_INT_ARRAY: return "int";
        case uTypeEnum::U_VEC2: return "vec2";
        case uTypeEnum::U_IVEC2: return "ivec2";
        case uTypeEnum::U_UVEC2: return "uvec2";
        case uTypeEnum::U_VEC2_ARRAY: return "vec2";
        case uTypeEnum::U_VEC3: return "vec3";
        case uTypeEnum::U_IVEC3: return "ivec3";
        case uTypeEnum::U_UVEC3: return "uvec3";
        case uTypeEnum::U_VEC3_ARRAY: return "vec3";
        case uTypeEnum::U_VEC4: return "vec4";
        case uTypeEnum::U_IVEC4: return "ivec4";
        case uTypeEnum::U_UVEC4: return "uvec4";
        case uTypeEnum::U_BOOL: return "bool";
        case uTypeEnum::U_BVEC2: return "bvec2";
        case uTypeEnum::U_BVEC3: return "bvec3";
        case uTypeEnum::U_BVEC4: return "bvec4";
        case uTypeEnum::U_VEC4_ARRAY: return "vec4";
        case uTypeEnum::U_IMAGE1D: return "image1D";
        case uTypeEnum::U_IMAGE2D: return "image2D";
        case uTypeEnum::U_IMAGE3D: return "image3D";
        case uTypeEnum::U_SAMPLER1D:
        case uTypeEnum::U_SAMPLER1D_ARRAY: return "sampler1D";
        case uTypeEnum::U_SAMPLER2D:
        case uTypeEnum::U_SAMPLER2D_ARRAY: return "sampler2D";
        case uTypeEnum::U_SAMPLER3D:
        case uTypeEnum::U_SAMPLER3D_ARRAY: return "sampler3D";
        case uTypeEnum::U_SAMPLERCUBE: return "samplerCube";
        case uTypeEnum::U_SAMPLER2D_TEXTUREARRAY: return "sampler2DArray";
        case uTypeEnum::U_MAT2: return "mat2";
        case uTypeEnum::U_MAT3: return "mat3";
        case uTypeEnum::U_MAT4:
        case uTypeEnum::U_MAT4_ARRAY: return "mat4";
        case uTypeEnum::U_VOID: return "void";
        case uTypeEnum::U_STRUCT: return "structure";
        case uTypeEnum::U_TYPE: return "type";  // tout les types
        case uTypeEnum::U_VEC: return "vec";    // vec2, vec3, vec4
        case uTypeEnum::U_MAT: return "mat";    // mat2, mat3, mat4
        case uTypeEnum::U_FLOW: return "flow";
        case uTypeEnum::U_PROGRAM: return "program";
        case uTypeEnum::U_GLOBAL: return "global";
        case uTypeEnum::U_Count: break;
    }
    return "";
}

uTypeEnum GetGlslTypeFromString(const std::string& vType, bool vIsArray) {
    if (vType == "text")
        return uTypeEnum::U_TEXT;

    if (vIsArray) {
        if (vType == "mat4")
            return uTypeEnum::U_MAT4_ARRAY;
        if (vType == "float")
            return uTypeEnum::U_FLOAT_ARRAY;
        if (vType == "vec2")
            return uTypeEnum::U_VEC2_ARRAY;
        if (vType == "vec3")
            return uTypeEnum::U_VEC3_ARRAY;
        if (vType == "vec4")
            return uTypeEnum::U_VEC4_ARRAY;
        if (vType == "int")
            return uTypeEnum::U_INT_ARRAY;
        if (vType == "sampler1D")
            return uTypeEnum::U_SAMPLER1D_ARRAY;
        if (vType == "sampler2D")
            return uTypeEnum::U_SAMPLER2D_ARRAY;
        if (vType == "sampler3D")
            return uTypeEnum::U_SAMPLER3D_ARRAY;
    } else {
        if (vType == "float")
            return uTypeEnum::U_FLOAT;
        if (vType == "vec2")
            return uTypeEnum::U_VEC2;
        if (vType == "vec3")
            return uTypeEnum::U_VEC3;
        if (vType == "vec4")
            return uTypeEnum::U_VEC4;
        if (vType == "int")
            return uTypeEnum::U_INT;
        if (vType == "ivec2")
            return uTypeEnum::U_IVEC2;
        if (vType == "ivec3")
            return uTypeEnum::U_IVEC3;
        if (vType == "ivec4")
            return uTypeEnum::U_IVEC4;
        if (vType == "uint")
            return uTypeEnum::U_UINT;
        if (vType == "uvec2")
            return uTypeEnum::U_UVEC2;
        if (vType == "uvec3")
            return uTypeEnum::U_UVEC3;
        if (vType == "uvec4")
            return uTypeEnum::U_UVEC4;
        if (vType == "bool")
            return uTypeEnum::U_BOOL;
        if (vType == "bvec2")
            return uTypeEnum::U_BVEC2;
        if (vType == "bvec3")
            return uTypeEnum::U_BVEC3;
        if (vType == "bvec4")
            return uTypeEnum::U_BVEC4;
        if (vType == "image1D")
            return uTypeEnum::U_IMAGE1D;
        if (vType == "image2D")
            return uTypeEnum::U_IMAGE2D;
        if (vType == "image3D")
            return uTypeEnum::U_IMAGE3D;
        if (vType == "sampler1D")
            return uTypeEnum::U_SAMPLER1D;
        if (vType == "sampler2D")
            return uTypeEnum::U_SAMPLER2D;
        if (vType == "sampler3D")
            return uTypeEnum::U_SAMPLER3D;
    }

    if (vType == "sampler2DArray")
        return uTypeEnum::U_SAMPLER2D_TEXTUREARRAY;
    if (vType == "samplerCube")
        return uTypeEnum::U_SAMPLERCUBE;
    if (vType == "mat2")
        return uTypeEnum::U_MAT2;
    if (vType == "mat3")
        return uTypeEnum::U_MAT3;
    if (vType == "mat4")
        return uTypeEnum::U_MAT4;
    if (vType == "void")
        return uTypeEnum::U_VOID;
    if (vType == "structure")
        return uTypeEnum::U_STRUCT;
    if (vType == "bool")
        return uTypeEnum::U_BOOL;
    if (vType == "bvec2")
        return uTypeEnum::U_BVEC2;
    if (vType == "bvec3")
        return uTypeEnum::U_BVEC3;
    if (vType == "bvec4")
        return uTypeEnum::U_BVEC4;
    if (vType == "type")
        return uTypeEnum::U_TYPE;
    if (vType == "vec")
        return uTypeEnum::U_VEC;
    if (vType == "mat")
        return uTypeEnum::U_MAT;
    if (vType == "flow")
        return uTypeEnum::U_FLOW;
    if (vType == "program")
        return uTypeEnum::U_PROGRAM;
    if (vType == "global")
        return uTypeEnum::U_GLOBAL;

#ifdef _DEBUG
    printf("Type %s not found/supported", vType.c_str());
#endif

    return uTypeEnum::U_VOID;
}

uTypeEnum GetBaseGlslTypeFromString(const std::string& vType, bool vIsArray, uint32_t* vCountChannels) {
    auto type = GetGlslTypeFromString(vType, vIsArray);
    return GetBaseGlslTypeFromType(type, vCountChannels);
}

uint32_t GetCountChannelForType(const uTypeEnum& vType) {
    uint32_t res = 0;

    switch (vType) {
        case uTypeEnum::U_FLOAT:
        case uTypeEnum::U_FLOAT_ARRAY:
        case uTypeEnum::U_INT:
        case uTypeEnum::U_UINT:
        case uTypeEnum::U_INT_ARRAY:
        case uTypeEnum::U_BOOL: res = 1; break;

        case uTypeEnum::U_VEC2:
        case uTypeEnum::U_VEC2_ARRAY:
        case uTypeEnum::U_IVEC2:
        case uTypeEnum::U_UVEC2:
        case uTypeEnum::U_BVEC2: res = 2; break;

        case uTypeEnum::U_VEC3:
        case uTypeEnum::U_VEC3_ARRAY:
        case uTypeEnum::U_IVEC3:
        case uTypeEnum::U_UVEC3:
        case uTypeEnum::U_BVEC3: res = 3; break;

        case uTypeEnum::U_VEC4:
        case uTypeEnum::U_VEC4_ARRAY:
        case uTypeEnum::U_IVEC4:
        case uTypeEnum::U_UVEC4:
        case uTypeEnum::U_BVEC4:
        case uTypeEnum::U_MAT2: res = 4; break;

        case uTypeEnum::U_MAT3: res = 9; break;

        case uTypeEnum::U_MAT4_ARRAY:
        case uTypeEnum::U_MAT4: res = 16; break;

        case uTypeEnum::U_IMAGE1D:
        case uTypeEnum::U_IMAGE2D:
        case uTypeEnum::U_IMAGE3D:

        case uTypeEnum::U_SAMPLER1D:
        case uTypeEnum::U_SAMPLER1D_ARRAY:
        case uTypeEnum::U_SAMPLER2D:
        case uTypeEnum::U_SAMPLER2D_TEXTUREARRAY:
        case uTypeEnum::U_SAMPLER2D_ARRAY:
        case uTypeEnum::U_SAMPLER3D:
        case uTypeEnum::U_SAMPLER3D_ARRAY:
        case uTypeEnum::U_SAMPLERCUBE:

        case uTypeEnum::U_VOID:
        case uTypeEnum::U_STRUCT:
        case uTypeEnum::U_TYPE:
        case uTypeEnum::U_VEC:
        case uTypeEnum::U_MAT:
        case uTypeEnum::U_FLOW:
        case uTypeEnum::U_PROGRAM:
        case uTypeEnum::U_GLOBAL:
        case uTypeEnum::U_TEXT: break;

        case uTypeEnum::U_Count: break;
    }

    return res;
}

uTypeEnum GetBaseGlslTypeFromType(const uTypeEnum& vType, uint32_t* vCountChannels) {
    uTypeEnum res = uTypeEnum::U_VOID;

    switch (vType) {
        case uTypeEnum::U_FLOAT:
        case uTypeEnum::U_FLOAT_ARRAY:
        case uTypeEnum::U_VEC2:
        case uTypeEnum::U_VEC2_ARRAY:
        case uTypeEnum::U_VEC3:
        case uTypeEnum::U_VEC3_ARRAY:
        case uTypeEnum::U_VEC4:
        case uTypeEnum::U_VEC4_ARRAY:
        case uTypeEnum::U_MAT2:
        case uTypeEnum::U_MAT3:
        case uTypeEnum::U_MAT4_ARRAY:
        case uTypeEnum::U_MAT4: res = uTypeEnum::U_FLOAT; break;

        case uTypeEnum::U_INT:
        case uTypeEnum::U_INT_ARRAY:
        case uTypeEnum::U_IVEC2:
        case uTypeEnum::U_IVEC3:
        case uTypeEnum::U_IVEC4: res = uTypeEnum::U_INT; break;

        case uTypeEnum::U_UINT:
        case uTypeEnum::U_UVEC2:
        case uTypeEnum::U_UVEC3:
        case uTypeEnum::U_UVEC4: res = uTypeEnum::U_UINT; break;

        case uTypeEnum::U_BOOL:
        case uTypeEnum::U_BVEC2:
        case uTypeEnum::U_BVEC3:
        case uTypeEnum::U_BVEC4: res = uTypeEnum::U_BOOL; break;

        case uTypeEnum::U_IMAGE1D: res = uTypeEnum::U_IMAGE1D; break;
        case uTypeEnum::U_IMAGE2D: res = uTypeEnum::U_IMAGE2D; break;
        case uTypeEnum::U_IMAGE3D: res = uTypeEnum::U_IMAGE3D; break;

        case uTypeEnum::U_SAMPLER1D:
        case uTypeEnum::U_SAMPLER1D_ARRAY: res = uTypeEnum::U_SAMPLER1D; break;

        case uTypeEnum::U_SAMPLER2D:
        case uTypeEnum::U_SAMPLER2D_TEXTUREARRAY:
        case uTypeEnum::U_SAMPLER2D_ARRAY: res = uTypeEnum::U_SAMPLER2D; break;

        case uTypeEnum::U_SAMPLER3D:
        case uTypeEnum::U_SAMPLER3D_ARRAY: res = uTypeEnum::U_SAMPLER3D; break;

        case uTypeEnum::U_SAMPLERCUBE: res = uTypeEnum::U_SAMPLERCUBE; break;

        case uTypeEnum::U_VOID:
        case uTypeEnum::U_STRUCT:
        case uTypeEnum::U_TYPE:
        case uTypeEnum::U_VEC:
        case uTypeEnum::U_MAT:
        case uTypeEnum::U_FLOW:
        case uTypeEnum::U_PROGRAM:
        case uTypeEnum::U_GLOBAL:
        case uTypeEnum::U_TEXT: break;

        case uTypeEnum::U_Count: break;
    }

    if (vCountChannels) {
        *vCountChannels = GetCountChannelForType(vType);
    }

    return res;
}
}  // namespace uType