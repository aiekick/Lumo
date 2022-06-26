/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "NodeUtils.h"
#include <ctools/Logger.h>

std::string NodeUtils::GetStringFromTOperatorEnum(const glslang::TOperator& vTOperator)
{
	static std::array<std::string, glslang::TOperator::EOpReadClockDeviceKHR + 1> TOperatorString = {
	"EOpNull",            // if in a node", should only mean a node is still being built
	"EOpSequence",        // denotes a list of statements", or parameters", etc.
	"EOpLinkerObjects",   // for aggregate node of objects the linker may need", if not reference by the rest of the AST
	"EOpFunctionCall",
	"EOpFunction",        // For function definition
	"EOpParameters",      // an aggregate listing the parameters to a function

	//
	// Unary operators
	//

	"EOpNegative",
	"EOpLogicalNot",
	"EOpVectorLogicalNot",
	"EOpBitwiseNot",

	"EOpPostIncrement",
	"EOpPostDecrement",
	"EOpPreIncrement",
	"EOpPreDecrement",

	"EOpCopyObject",

	// (u)int* -> bool
	"EOpConvInt8ToBool",
	"EOpConvUint8ToBool",
	"EOpConvInt16ToBool",
	"EOpConvUint16ToBool",
	"EOpConvIntToBool",
	"EOpConvUintToBool",
	"EOpConvInt64ToBool",
	"EOpConvUint64ToBool",

	// float* -> bool
	"EOpConvFloat16ToBool",
	"EOpConvFloatToBool",
	"EOpConvDoubleToBool",

	// bool -> (u)int*
	"EOpConvBoolToInt8",
	"EOpConvBoolToUint8",
	"EOpConvBoolToInt16",
	"EOpConvBoolToUint16",
	"EOpConvBoolToInt",
	"EOpConvBoolToUint",
	"EOpConvBoolToInt64",
	"EOpConvBoolToUint64",

	// bool -> float*
	"EOpConvBoolToFloat16",
	"EOpConvBoolToFloat",
	"EOpConvBoolToDouble",

	// int8_t -> (u)int*
	"EOpConvInt8ToInt16",
	"EOpConvInt8ToInt",
	"EOpConvInt8ToInt64",
	"EOpConvInt8ToUint8",
	"EOpConvInt8ToUint16",
	"EOpConvInt8ToUint",
	"EOpConvInt8ToUint64",

	// uint8_t -> (u)int*
	"EOpConvUint8ToInt8",
	"EOpConvUint8ToInt16",
	"EOpConvUint8ToInt",
	"EOpConvUint8ToInt64",
	"EOpConvUint8ToUint16",
	"EOpConvUint8ToUint",
	"EOpConvUint8ToUint64",

	// int8_t -> float*
	"EOpConvInt8ToFloat16",
	"EOpConvInt8ToFloat",
	"EOpConvInt8ToDouble",

	// uint8_t -> float*
	"EOpConvUint8ToFloat16",
	"EOpConvUint8ToFloat",
	"EOpConvUint8ToDouble",

	// int16_t -> (u)int*
	"EOpConvInt16ToInt8",
	"EOpConvInt16ToInt",
	"EOpConvInt16ToInt64",
	"EOpConvInt16ToUint8",
	"EOpConvInt16ToUint16",
	"EOpConvInt16ToUint",
	"EOpConvInt16ToUint64",

	// uint16_t -> (u)int*
	"EOpConvUint16ToInt8",
	"EOpConvUint16ToInt16",
	"EOpConvUint16ToInt",
	"EOpConvUint16ToInt64",
	"EOpConvUint16ToUint8",
	"EOpConvUint16ToUint",
	"EOpConvUint16ToUint64",

	// int16_t -> float*
	"EOpConvInt16ToFloat16",
	"EOpConvInt16ToFloat",
	"EOpConvInt16ToDouble",

	// uint16_t -> float*
	"EOpConvUint16ToFloat16",
	"EOpConvUint16ToFloat",
	"EOpConvUint16ToDouble",

	// int32_t -> (u)int*
	"EOpConvIntToInt8",
	"EOpConvIntToInt16",
	"EOpConvIntToInt64",
	"EOpConvIntToUint8",
	"EOpConvIntToUint16",
	"EOpConvIntToUint",
	"EOpConvIntToUint64",

	// uint32_t -> (u)int*
	"EOpConvUintToInt8",
	"EOpConvUintToInt16",
	"EOpConvUintToInt",
	"EOpConvUintToInt64",
	"EOpConvUintToUint8",
	"EOpConvUintToUint16",
	"EOpConvUintToUint64",

	// int32_t -> float*
	"EOpConvIntToFloat16",
	"EOpConvIntToFloat",
	"EOpConvIntToDouble",

	// uint32_t -> float*
	"EOpConvUintToFloat16",
	"EOpConvUintToFloat",
	"EOpConvUintToDouble",

	// int64_t -> (u)int*
	"EOpConvInt64ToInt8",
	"EOpConvInt64ToInt16",
	"EOpConvInt64ToInt",
	"EOpConvInt64ToUint8",
	"EOpConvInt64ToUint16",
	"EOpConvInt64ToUint",
	"EOpConvInt64ToUint64",

	// uint64_t -> (u)int*
	"EOpConvUint64ToInt8",
	"EOpConvUint64ToInt16",
	"EOpConvUint64ToInt",
	"EOpConvUint64ToInt64",
	"EOpConvUint64ToUint8",
	"EOpConvUint64ToUint16",
	"EOpConvUint64ToUint",

	// int64_t -> float*
	"EOpConvInt64ToFloat16",
	"EOpConvInt64ToFloat",
	"EOpConvInt64ToDouble",

	// uint64_t -> float*
	"EOpConvUint64ToFloat16",
	"EOpConvUint64ToFloat",
	"EOpConvUint64ToDouble",

	// float16_t -> (u)int*
	"EOpConvFloat16ToInt8",
	"EOpConvFloat16ToInt16",
	"EOpConvFloat16ToInt",
	"EOpConvFloat16ToInt64",
	"EOpConvFloat16ToUint8",
	"EOpConvFloat16ToUint16",
	"EOpConvFloat16ToUint",
	"EOpConvFloat16ToUint64",

	// float16_t -> float*
	"EOpConvFloat16ToFloat",
	"EOpConvFloat16ToDouble",

	// float -> (u)int*
	"EOpConvFloatToInt8",
	"EOpConvFloatToInt16",
	"EOpConvFloatToInt",
	"EOpConvFloatToInt64",
	"EOpConvFloatToUint8",
	"EOpConvFloatToUint16",
	"EOpConvFloatToUint",
	"EOpConvFloatToUint64",

	// float -> float*
	"EOpConvFloatToFloat16",
	"EOpConvFloatToDouble",

	// float64 _t-> (u)int*
	"EOpConvDoubleToInt8",
	"EOpConvDoubleToInt16",
	"EOpConvDoubleToInt",
	"EOpConvDoubleToInt64",
	"EOpConvDoubleToUint8",
	"EOpConvDoubleToUint16",
	"EOpConvDoubleToUint",
	"EOpConvDoubleToUint64",

	// float64_t -> float*
	"EOpConvDoubleToFloat16",
	"EOpConvDoubleToFloat",

	// uint64_t <-> pointer
	"EOpConvUint64ToPtr",
	"EOpConvPtrToUint64",

	// uvec2 <-> pointer
	"EOpConvUvec2ToPtr",
	"EOpConvPtrToUvec2",

	//
	// binary operations
	//

	"EOpAdd",
	"EOpSub",
	"EOpMul",
	"EOpDiv",
	"EOpMod",
	"EOpRightShift",
	"EOpLeftShift",
	"EOpAnd",
	"EOpInclusiveOr",
	"EOpExclusiveOr",
	"EOpEqual",
	"EOpNotEqual",
	"EOpVectorEqual",
	"EOpVectorNotEqual",
	"EOpLessThan",
	"EOpGreaterThan",
	"EOpLessThanEqual",
	"EOpGreaterThanEqual",
	"EOpComma",

	"EOpVectorTimesScalar",
	"EOpVectorTimesMatrix",
	"EOpMatrixTimesVector",
	"EOpMatrixTimesScalar",

	"EOpLogicalOr",
	"EOpLogicalXor",
	"EOpLogicalAnd",

	"EOpIndexDirect",
	"EOpIndexIndirect",
	"EOpIndexDirectStruct",

	"EOpVectorSwizzle",

	"EOpMethod",
	"EOpScoping",

	//
	// Built-in functions mapped to operators
	//

	"EOpRadians",
	"EOpDegrees",
	"EOpSin",
	"EOpCos",
	"EOpTan",
	"EOpAsin",
	"EOpAcos",
	"EOpAtan",
	"EOpSinh",
	"EOpCosh",
	"EOpTanh",
	"EOpAsinh",
	"EOpAcosh",
	"EOpAtanh",

	"EOpPow",
	"EOpExp",
	"EOpLog",
	"EOpExp2",
	"EOpLog2",
	"EOpSqrt",
	"EOpInverseSqrt",

	"EOpAbs",
	"EOpSign",
	"EOpFloor",
	"EOpTrunc",
	"EOpRound",
	"EOpRoundEven",
	"EOpCeil",
	"EOpFract",
	"EOpModf",
	"EOpMin",
	"EOpMax",
	"EOpClamp",
	"EOpMix",
	"EOpStep",
	"EOpSmoothStep",

	"EOpIsNan",
	"EOpIsInf",

	"EOpFma",

	"EOpFrexp",
	"EOpLdexp",

	"EOpFloatBitsToInt",
	"EOpFloatBitsToUint",
	"EOpIntBitsToFloat",
	"EOpUintBitsToFloat",
	"EOpDoubleBitsToInt64",
	"EOpDoubleBitsToUint64",
	"EOpInt64BitsToDouble",
	"EOpUint64BitsToDouble",
	"EOpFloat16BitsToInt16",
	"EOpFloat16BitsToUint16",
	"EOpInt16BitsToFloat16",
	"EOpUint16BitsToFloat16",
	"EOpPackSnorm2x16",
	"EOpUnpackSnorm2x16",
	"EOpPackUnorm2x16",
	"EOpUnpackUnorm2x16",
	"EOpPackSnorm4x8",
	"EOpUnpackSnorm4x8",
	"EOpPackUnorm4x8",
	"EOpUnpackUnorm4x8",
	"EOpPackHalf2x16",
	"EOpUnpackHalf2x16",
	"EOpPackDouble2x32",
	"EOpUnpackDouble2x32",
	"EOpPackInt2x32",
	"EOpUnpackInt2x32",
	"EOpPackUint2x32",
	"EOpUnpackUint2x32",
	"EOpPackFloat2x16",
	"EOpUnpackFloat2x16",
	"EOpPackInt2x16",
	"EOpUnpackInt2x16",
	"EOpPackUint2x16",
	"EOpUnpackUint2x16",
	"EOpPackInt4x16",
	"EOpUnpackInt4x16",
	"EOpPackUint4x16",
	"EOpUnpackUint4x16",
	"EOpPack16",
	"EOpPack32",
	"EOpPack64",
	"EOpUnpack32",
	"EOpUnpack16",
	"EOpUnpack8",

	"EOpLength",
	"EOpDistance",
	"EOpDot",
	"EOpCross",
	"EOpNormalize",
	"EOpFaceForward",
	"EOpReflect",
	"EOpRefract",

	"EOpMin3",
	"EOpMax3",
	"EOpMid3",

	"EOpDPdx",            // Fragment only
	"EOpDPdy",            // Fragment only
	"EOpFwidth",          // Fragment only
	"EOpDPdxFine",        // Fragment only
	"EOpDPdyFine",        // Fragment only
	"EOpFwidthFine",      // Fragment only
	"EOpDPdxCoarse",      // Fragment only
	"EOpDPdyCoarse",      // Fragment only
	"EOpFwidthCoarse",    // Fragment only

	"EOpInterpolateAtCentroid", // Fragment only
	"EOpInterpolateAtSample",   // Fragment only
	"EOpInterpolateAtOffset",   // Fragment only
	"EOpInterpolateAtVertex",

	"EOpMatrixTimesMatrix",
	"EOpOuterProduct",
	"EOpDeterminant",
	"EOpMatrixInverse",
	"EOpTranspose",

	"EOpFtransform",

	"EOpNoise",

	"EOpEmitVertex",           // geometry only
	"EOpEndPrimitive",         // geometry only
	"EOpEmitStreamVertex",     // geometry only
	"EOpEndStreamPrimitive",   // geometry only

	"EOpBarrier",
	"EOpMemoryBarrier",
	"EOpMemoryBarrierAtomicCounter",
	"EOpMemoryBarrierBuffer",
	"EOpMemoryBarrierImage",
	"EOpMemoryBarrierShared",  // compute only
	"EOpGroupMemoryBarrier",   // compute only

	"EOpBallot",
	"EOpReadInvocation",
	"EOpReadFirstInvocation",

	"EOpAnyInvocation",
	"EOpAllInvocations",
	"EOpAllInvocationsEqual",

	"EOpSubgroupGuardStart",
	"EOpSubgroupBarrier",
	"EOpSubgroupMemoryBarrier",
	"EOpSubgroupMemoryBarrierBuffer",
	"EOpSubgroupMemoryBarrierImage",
	"EOpSubgroupMemoryBarrierShared", // compute only
	"EOpSubgroupElect",
	"EOpSubgroupAll",
	"EOpSubgroupAny",
	"EOpSubgroupAllEqual",
	"EOpSubgroupBroadcast",
	"EOpSubgroupBroadcastFirst",
	"EOpSubgroupBallot",
	"EOpSubgroupInverseBallot",
	"EOpSubgroupBallotBitExtract",
	"EOpSubgroupBallotBitCount",
	"EOpSubgroupBallotInclusiveBitCount",
	"EOpSubgroupBallotExclusiveBitCount",
	"EOpSubgroupBallotFindLSB",
	"EOpSubgroupBallotFindMSB",
	"EOpSubgroupShuffle",
	"EOpSubgroupShuffleXor",
	"EOpSubgroupShuffleUp",
	"EOpSubgroupShuffleDown",
	"EOpSubgroupAdd",
	"EOpSubgroupMul",
	"EOpSubgroupMin",
	"EOpSubgroupMax",
	"EOpSubgroupAnd",
	"EOpSubgroupOr",
	"EOpSubgroupXor",
	"EOpSubgroupInclusiveAdd",
	"EOpSubgroupInclusiveMul",
	"EOpSubgroupInclusiveMin",
	"EOpSubgroupInclusiveMax",
	"EOpSubgroupInclusiveAnd",
	"EOpSubgroupInclusiveOr",
	"EOpSubgroupInclusiveXor",
	"EOpSubgroupExclusiveAdd",
	"EOpSubgroupExclusiveMul",
	"EOpSubgroupExclusiveMin",
	"EOpSubgroupExclusiveMax",
	"EOpSubgroupExclusiveAnd",
	"EOpSubgroupExclusiveOr",
	"EOpSubgroupExclusiveXor",
	"EOpSubgroupClusteredAdd",
	"EOpSubgroupClusteredMul",
	"EOpSubgroupClusteredMin",
	"EOpSubgroupClusteredMax",
	"EOpSubgroupClusteredAnd",
	"EOpSubgroupClusteredOr",
	"EOpSubgroupClusteredXor",
	"EOpSubgroupQuadBroadcast",
	"EOpSubgroupQuadSwapHorizontal",
	"EOpSubgroupQuadSwapVertical",
	"EOpSubgroupQuadSwapDiagonal",

	"EOpSubgroupPartition",
	"EOpSubgroupPartitionedAdd",
	"EOpSubgroupPartitionedMul",
	"EOpSubgroupPartitionedMin",
	"EOpSubgroupPartitionedMax",
	"EOpSubgroupPartitionedAnd",
	"EOpSubgroupPartitionedOr",
	"EOpSubgroupPartitionedXor",
	"EOpSubgroupPartitionedInclusiveAdd",
	"EOpSubgroupPartitionedInclusiveMul",
	"EOpSubgroupPartitionedInclusiveMin",
	"EOpSubgroupPartitionedInclusiveMax",
	"EOpSubgroupPartitionedInclusiveAnd",
	"EOpSubgroupPartitionedInclusiveOr",
	"EOpSubgroupPartitionedInclusiveXor",
	"EOpSubgroupPartitionedExclusiveAdd",
	"EOpSubgroupPartitionedExclusiveMul",
	"EOpSubgroupPartitionedExclusiveMin",
	"EOpSubgroupPartitionedExclusiveMax",
	"EOpSubgroupPartitionedExclusiveAnd",
	"EOpSubgroupPartitionedExclusiveOr",
	"EOpSubgroupPartitionedExclusiveXor",

	"EOpSubgroupGuardStop",

	"EOpMinInvocations",
	"EOpMaxInvocations",
	"EOpAddInvocations",
	"EOpMinInvocationsNonUniform",
	"EOpMaxInvocationsNonUniform",
	"EOpAddInvocationsNonUniform",
	"EOpMinInvocationsInclusiveScan",
	"EOpMaxInvocationsInclusiveScan",
	"EOpAddInvocationsInclusiveScan",
	"EOpMinInvocationsInclusiveScanNonUniform",
	"EOpMaxInvocationsInclusiveScanNonUniform",
	"EOpAddInvocationsInclusiveScanNonUniform",
	"EOpMinInvocationsExclusiveScan",
	"EOpMaxInvocationsExclusiveScan",
	"EOpAddInvocationsExclusiveScan",
	"EOpMinInvocationsExclusiveScanNonUniform",
	"EOpMaxInvocationsExclusiveScanNonUniform",
	"EOpAddInvocationsExclusiveScanNonUniform",
	"EOpSwizzleInvocations",
	"EOpSwizzleInvocationsMasked",
	"EOpWriteInvocation",
	"EOpMbcnt",

	"EOpCubeFaceIndex",
	"EOpCubeFaceCoord",
	"EOpTime",

	"EOpAtomicAdd",
	"EOpAtomicMin",
	"EOpAtomicMax",
	"EOpAtomicAnd",
	"EOpAtomicOr",
	"EOpAtomicXor",
	"EOpAtomicExchange",
	"EOpAtomicCompSwap",
	"EOpAtomicLoad",
	"EOpAtomicStore",

	"EOpAtomicCounterIncrement", // results in pre-increment value
	"EOpAtomicCounterDecrement", // results in post-decrement value
	"EOpAtomicCounter",
	"EOpAtomicCounterAdd",
	"EOpAtomicCounterSubtract",
	"EOpAtomicCounterMin",
	"EOpAtomicCounterMax",
	"EOpAtomicCounterAnd",
	"EOpAtomicCounterOr",
	"EOpAtomicCounterXor",
	"EOpAtomicCounterExchange",
	"EOpAtomicCounterCompSwap",

	"EOpAny",
	"EOpAll",

	"EOpCooperativeMatrixLoad",
	"EOpCooperativeMatrixStore",
	"EOpCooperativeMatrixMulAdd",

	"EOpBeginInvocationInterlock", // Fragment only
	"EOpEndInvocationInterlock", // Fragment only

	"EOpIsHelperInvocation",

	"EOpDebugPrintf",

	//
	// Branch
	//

	"EOpKill",            // Fragment only
	"EOpReturn",
	"EOpBreak",
	"EOpContinue",
	"EOpCase",
	"EOpDefault",
	"EOpDemote",          // Fragment only

	//
	// Constructors
	//

	"EOpConstructGuardStart",
	"EOpConstructInt",          // these first scalar forms also identify what implicit conversion is needed
	"EOpConstructUint",
	"EOpConstructInt8",
	"EOpConstructUint8",
	"EOpConstructInt16",
	"EOpConstructUint16",
	"EOpConstructInt64",
	"EOpConstructUint64",
	"EOpConstructBool",
	"EOpConstructFloat",
	"EOpConstructDouble",
	// Keep vector and matrix constructors in a consistent relative order for
	// TParseContext::constructBuiltIn", which converts between 8/16/32 bit
	// vector constructors
	"EOpConstructVec2",
	"EOpConstructVec3",
	"EOpConstructVec4",
	"EOpConstructMat2x2",
	"EOpConstructMat2x3",
	"EOpConstructMat2x4",
	"EOpConstructMat3x2",
	"EOpConstructMat3x3",
	"EOpConstructMat3x4",
	"EOpConstructMat4x2",
	"EOpConstructMat4x3",
	"EOpConstructMat4x4",
	"EOpConstructDVec2",
	"EOpConstructDVec3",
	"EOpConstructDVec4",
	"EOpConstructBVec2",
	"EOpConstructBVec3",
	"EOpConstructBVec4",
	"EOpConstructI8Vec2",
	"EOpConstructI8Vec3",
	"EOpConstructI8Vec4",
	"EOpConstructU8Vec2",
	"EOpConstructU8Vec3",
	"EOpConstructU8Vec4",
	"EOpConstructI16Vec2",
	"EOpConstructI16Vec3",
	"EOpConstructI16Vec4",
	"EOpConstructU16Vec2",
	"EOpConstructU16Vec3",
	"EOpConstructU16Vec4",
	"EOpConstructIVec2",
	"EOpConstructIVec3",
	"EOpConstructIVec4",
	"EOpConstructUVec2",
	"EOpConstructUVec3",
	"EOpConstructUVec4",
	"EOpConstructI64Vec2",
	"EOpConstructI64Vec3",
	"EOpConstructI64Vec4",
	"EOpConstructU64Vec2",
	"EOpConstructU64Vec3",
	"EOpConstructU64Vec4",
	"EOpConstructDMat2x2",
	"EOpConstructDMat2x3",
	"EOpConstructDMat2x4",
	"EOpConstructDMat3x2",
	"EOpConstructDMat3x3",
	"EOpConstructDMat3x4",
	"EOpConstructDMat4x2",
	"EOpConstructDMat4x3",
	"EOpConstructDMat4x4",
	"EOpConstructIMat2x2",
	"EOpConstructIMat2x3",
	"EOpConstructIMat2x4",
	"EOpConstructIMat3x2",
	"EOpConstructIMat3x3",
	"EOpConstructIMat3x4",
	"EOpConstructIMat4x2",
	"EOpConstructIMat4x3",
	"EOpConstructIMat4x4",
	"EOpConstructUMat2x2",
	"EOpConstructUMat2x3",
	"EOpConstructUMat2x4",
	"EOpConstructUMat3x2",
	"EOpConstructUMat3x3",
	"EOpConstructUMat3x4",
	"EOpConstructUMat4x2",
	"EOpConstructUMat4x3",
	"EOpConstructUMat4x4",
	"EOpConstructBMat2x2",
	"EOpConstructBMat2x3",
	"EOpConstructBMat2x4",
	"EOpConstructBMat3x2",
	"EOpConstructBMat3x3",
	"EOpConstructBMat3x4",
	"EOpConstructBMat4x2",
	"EOpConstructBMat4x3",
	"EOpConstructBMat4x4",
	"EOpConstructFloat16",
	"EOpConstructF16Vec2",
	"EOpConstructF16Vec3",
	"EOpConstructF16Vec4",
	"EOpConstructF16Mat2x2",
	"EOpConstructF16Mat2x3",
	"EOpConstructF16Mat2x4",
	"EOpConstructF16Mat3x2",
	"EOpConstructF16Mat3x3",
	"EOpConstructF16Mat3x4",
	"EOpConstructF16Mat4x2",
	"EOpConstructF16Mat4x3",
	"EOpConstructF16Mat4x4",
	"EOpConstructStruct",
	"EOpConstructTextureSampler",
	"EOpConstructNonuniform",     // expected to be transformed away", not present in final AST
	"EOpConstructReference",
	"EOpConstructCooperativeMatrix",
	"EOpConstructGuardEnd",

	//
	// moves
	//

	"EOpAssign",
	"EOpAddAssign",
	"EOpSubAssign",
	"EOpMulAssign",
	"EOpVectorTimesMatrixAssign",
	"EOpVectorTimesScalarAssign",
	"EOpMatrixTimesScalarAssign",
	"EOpMatrixTimesMatrixAssign",
	"EOpDivAssign",
	"EOpModAssign",
	"EOpAndAssign",
	"EOpInclusiveOrAssign",
	"EOpExclusiveOrAssign",
	"EOpLeftShiftAssign",
	"EOpRightShiftAssign",

	//
	// Array operators
	//

	// Can apply to arrays", vectors", or matrices.
	// Can be decomposed to a constant at compile time", but this does not always happen",
	// due to link-time effects. So", consumer can expect either a link-time sized or
	// run-time sized array.
	"EOpArrayLength",

	//
	// Image operations
	//

	"EOpImageGuardBegin",

	"EOpImageQuerySize",
	"EOpImageQuerySamples",
	"EOpImageLoad",
	"EOpImageStore",
	"EOpImageLoadLod",
	"EOpImageStoreLod",
	"EOpImageAtomicAdd",
	"EOpImageAtomicMin",
	"EOpImageAtomicMax",
	"EOpImageAtomicAnd",
	"EOpImageAtomicOr",
	"EOpImageAtomicXor",
	"EOpImageAtomicExchange",
	"EOpImageAtomicCompSwap",
	"EOpImageAtomicLoad",
	"EOpImageAtomicStore",

	"EOpSubpassLoad",
	"EOpSubpassLoadMS",
	"EOpSparseImageLoad",
	"EOpSparseImageLoadLod",

	"EOpImageGuardEnd",

	//
	// Texture operations
	//

	"EOpTextureGuardBegin",

	"EOpTextureQuerySize",
	"EOpTextureQueryLod",
	"EOpTextureQueryLevels",
	"EOpTextureQuerySamples",

	"EOpSamplingGuardBegin",

	"EOpTexture",
	"EOpTextureProj",
	"EOpTextureLod",
	"EOpTextureOffset",
	"EOpTextureFetch",
	"EOpTextureFetchOffset",
	"EOpTextureProjOffset",
	"EOpTextureLodOffset",
	"EOpTextureProjLod",
	"EOpTextureProjLodOffset",
	"EOpTextureGrad",
	"EOpTextureGradOffset",
	"EOpTextureProjGrad",
	"EOpTextureProjGradOffset",
	"EOpTextureGather",
	"EOpTextureGatherOffset",
	"EOpTextureGatherOffsets",
	"EOpTextureClamp",
	"EOpTextureOffsetClamp",
	"EOpTextureGradClamp",
	"EOpTextureGradOffsetClamp",
	"EOpTextureGatherLod",
	"EOpTextureGatherLodOffset",
	"EOpTextureGatherLodOffsets",
	"EOpFragmentMaskFetch",
	"EOpFragmentFetch",

	"EOpSparseTextureGuardBegin",

	"EOpSparseTexture",
	"EOpSparseTextureLod",
	"EOpSparseTextureOffset",
	"EOpSparseTextureFetch",
	"EOpSparseTextureFetchOffset",
	"EOpSparseTextureLodOffset",
	"EOpSparseTextureGrad",
	"EOpSparseTextureGradOffset",
	"EOpSparseTextureGather",
	"EOpSparseTextureGatherOffset",
	"EOpSparseTextureGatherOffsets",
	"EOpSparseTexelsResident",
	"EOpSparseTextureClamp",
	"EOpSparseTextureOffsetClamp",
	"EOpSparseTextureGradClamp",
	"EOpSparseTextureGradOffsetClamp",
	"EOpSparseTextureGatherLod",
	"EOpSparseTextureGatherLodOffset",
	"EOpSparseTextureGatherLodOffsets",

	"EOpSparseTextureGuardEnd",

	"EOpImageFootprintGuardBegin",
	"EOpImageSampleFootprintNV",
	"EOpImageSampleFootprintClampNV",
	"EOpImageSampleFootprintLodNV",
	"EOpImageSampleFootprintGradNV",
	"EOpImageSampleFootprintGradClampNV",
	"EOpImageFootprintGuardEnd",
	"EOpSamplingGuardEnd",
	"EOpTextureGuardEnd",

	//
	// Integer operations
	//

	"EOpAddCarry",
	"EOpSubBorrow",
	"EOpUMulExtended",
	"EOpIMulExtended",
	"EOpBitfieldExtract",
	"EOpBitfieldInsert",
	"EOpBitFieldReverse",
	"EOpBitCount",
	"EOpFindLSB",
	"EOpFindMSB",

	"EOpCountLeadingZeros",
	"EOpCountTrailingZeros",
	"EOpAbsDifference",
	"EOpAddSaturate",
	"EOpSubSaturate",
	"EOpAverage",
	"EOpAverageRounded",
	"EOpMul32x16",

	"EOpTrace",
	"EOpReportIntersection",
	"EOpIgnoreIntersection",
	"EOpTerminateRay",
	"EOpExecuteCallable",
	"EOpWritePackedPrimitiveIndices4x8NV",

	//
	// GL_EXT_ray_query operations
	//

	"EOpRayQueryInitialize",
	"EOpRayQueryTerminate",
	"EOpRayQueryGenerateIntersection",
	"EOpRayQueryConfirmIntersection",
	"EOpRayQueryProceed",
	"EOpRayQueryGetIntersectionType",
	"EOpRayQueryGetRayTMin",
	"EOpRayQueryGetRayFlags",
	"EOpRayQueryGetIntersectionT",
	"EOpRayQueryGetIntersectionInstanceCustomIndex",
	"EOpRayQueryGetIntersectionInstanceId",
	"EOpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffset",
	"EOpRayQueryGetIntersectionGeometryIndex",
	"EOpRayQueryGetIntersectionPrimitiveIndex",
	"EOpRayQueryGetIntersectionBarycentrics",
	"EOpRayQueryGetIntersectionFrontFace",
	"EOpRayQueryGetIntersectionCandidateAABBOpaque",
	"EOpRayQueryGetIntersectionObjectRayDirection",
	"EOpRayQueryGetIntersectionObjectRayOrigin",
	"EOpRayQueryGetWorldRayDirection",
	"EOpRayQueryGetWorldRayOrigin",
	"EOpRayQueryGetIntersectionObjectToWorld",
	"EOpRayQueryGetIntersectionWorldToObject",

	//
	// HLSL operations
	//

	"EOpClip",                // discard if input value < 0
	"EOpIsFinite",
	"EOpLog10",               // base 10 log
	"EOpRcp",                 // 1/x
	"EOpSaturate",            // clamp from 0 to 1
	"EOpSinCos",              // sin and cos in out parameters
	"EOpGenMul",              // mul(x",y) on any of mat/vec/scalars
	"EOpDst",                 // x = 1", y=src0.y * src1.y", z=src0.z", w=src1.w
	"EOpInterlockedAdd",      // atomic ops", but uses [optional] out arg instead of return
	"EOpInterlockedAnd",      // ...
	"EOpInterlockedCompareExchange", // ...
	"EOpInterlockedCompareStore",    // ...
	"EOpInterlockedExchange", // ...
	"EOpInterlockedMax",      // ...
	"EOpInterlockedMin",      // ...
	"EOpInterlockedOr",       // ...
	"EOpInterlockedXor",      // ...
	"EOpAllMemoryBarrierWithGroupSync",    // memory barriers without non-hlsl AST equivalents
	"EOpDeviceMemoryBarrier",              // ...
	"EOpDeviceMemoryBarrierWithGroupSync", // ...
	"EOpWorkgroupMemoryBarrier",           // ...
	"EOpWorkgroupMemoryBarrierWithGroupSync", // ...
	"EOpEvaluateAttributeSnapped",         // InterpolateAtOffset with int position on 16x16 grid
	"EOpF32tof16",                         // HLSL conversion: half of a PackHalf2x16
	"EOpF16tof32",                         // HLSL conversion: half of an UnpackHalf2x16
	"EOpLit",                              // HLSL lighting coefficient vector
	"EOpTextureBias",                      // HLSL texture bias: will be lowered to "EOpTexture
	"EOpAsDouble",                         // slightly different from "EOpUint64BitsToDouble
	"EOpD3DCOLORtoUBYTE4",                 // convert and swizzle 4-component color to UBYTE4 range

	"EOpMethodSample",                     // Texture object methods.  These are translated to existing
	"EOpMethodSampleBias",                 // AST methods", and exist to represent HLSL semantics until that
	"EOpMethodSampleCmp",                  // translation is performed.  See HlslParseContext::decomposeSampleMethods().
	"EOpMethodSampleCmpLevelZero",         // ...
	"EOpMethodSampleGrad",                 // ...
	"EOpMethodSampleLevel",                // ...
	"EOpMethodLoad",                       // ...
	"EOpMethodGetDimensions",              // ...
	"EOpMethodGetSamplePosition",          // ...
	"EOpMethodGather",                     // ...
	"EOpMethodCalculateLevelOfDetail",     // ...
	"EOpMethodCalculateLevelOfDetailUnclamped",     // ...

	// Load already defined above for textures
	"EOpMethodLoad2",                      // Structure buffer object methods.  These are translated to existing
	"EOpMethodLoad3",                      // AST methods", and exist to represent HLSL semantics until that
	"EOpMethodLoad4",                      // translation is performed.  See HlslParseContext::decomposeSampleMethods().
	"EOpMethodStore",                      // ...
	"EOpMethodStore2",                     // ...
	"EOpMethodStore3",                     // ...
	"EOpMethodStore4",                     // ...
	"EOpMethodIncrementCounter",           // ...
	"EOpMethodDecrementCounter",           // ...
	// "EOpMethodAppend is defined for geo shaders below
	"EOpMethodConsume",

	// SM5 texture methods
	"EOpMethodGatherRed",                  // These are covered under the above "EOpMethodSample comment about
	"EOpMethodGatherGreen",                // translation to existing AST opcodes.  They exist temporarily
	"EOpMethodGatherBlue",                 // because HLSL arguments are slightly different.
	"EOpMethodGatherAlpha",                // ...
	"EOpMethodGatherCmp",                  // ...
	"EOpMethodGatherCmpRed",               // ...
	"EOpMethodGatherCmpGreen",             // ...
	"EOpMethodGatherCmpBlue",              // ...
	"EOpMethodGatherCmpAlpha",             // ...

	// geometry methods
	"EOpMethodAppend",                     // Geometry shader methods
	"EOpMethodRestartStrip",               // ...

	// matrix
	"EOpMatrixSwizzle",                    // select multiple matrix components (non-column)

	// SM6 wave ops
	"EOpWaveGetLaneCount",                 // Will decompose to gl_SubgroupSize.
	"EOpWaveGetLaneIndex",                 // Will decompose to gl_SubgroupInvocationID.
	"EOpWaveActiveCountBits",              // Will decompose to subgroupBallotBitCount(subgroupBallot()).
	"EOpWavePrefixCountBits",              // Will decompose to subgroupBallotInclusiveBitCount(subgroupBallot()).

	// Shader Clock Ops
	"EOpReadClockSubgroupKHR",
	"EOpReadClockDeviceKHR",
	};
	if (vTOperator != glslang::TOperator::EOpReadClockDeviceKHR + 1)
		return TOperatorString[(int)vTOperator];
	LogVarDebug("Error, one TOperator have no corresponding string, return \"None\"");
	return "NONE";
}
