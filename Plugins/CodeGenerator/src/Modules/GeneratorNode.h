#pragma once

#include <string>
#include <memory>
#include <vector>

enum BaseTypeEnum : uint32_t
{
    BASE_TYPE_None = 0,
    BASE_TYPE_Task,
    BASE_TYPE_Model,
    BASE_TYPE_Custom,
    BASE_TYPE_Texture,
    BASE_TYPE_Variable,
    BASE_TYPE_LightGroup,
    BASE_TYPE_ShaderPass,
    BASE_TYPE_TexelBuffer,
    BASE_TYPE_TextureCube,
    BASE_TYPE_TextureGroup,
    BASE_TYPE_StorageBuffer,
    BASE_TYPE_AccelStructure,
};

class BaseTypes
{
public:
	std::vector<std::string> m_TypeArray =
	{
		"None",
        "Task",
        "Model",
		"Custom",
        "Texture",
        "Variable",
        "LightGroup",
        "ShaderPass",
        "TexelBuffer",
        "TextureCube",
        "TextureGroup",
		"StorageBuffer",
		"AccelStructure",
	};
	std::vector<std::string> m_VariableTypeArray =
	{
		"WIDGET_BOOLEAN",
		"WIDGET_FLOAT",
		"WIDGET_INT",
		"WIDGET_UINT"
	}; 
	std::vector<std::string> m_VertexStructTypes = 
	{
		"P3_N3_TA3_BTA3_T2_C4", // LEGACY MESH
		"P3_N3_T2_C4",
		"P3_N3_T2",
		"P3_N3_C4_D1",
		"P3_N3_C4",				// BASE PROCEDURAL MESH
		"P3_C4",
		"P2_T2",
	};
};

#include <ctools/cTools.h>

class GeneratorNode;
typedef std::weak_ptr<GeneratorNode> GeneratorNodeWeak;
typedef std::shared_ptr<GeneratorNode> GeneratorNodePtr;

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <Gaia/gaia.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>
#include <Editor/UBOEditor.h>

#ifndef RENDERER_TYPE_NONE
#define RENDERER_TYPE_NONE "None"
#endif
#ifndef RENDERER_TYPE_PIXEL_2D
#define RENDERER_TYPE_PIXEL_2D "Pixel 2D"
#endif
#ifndef RENDERER_TYPE_COMPUTE_1D
#define RENDERER_TYPE_COMPUTE_1D "Compute 1D"
#endif
#ifndef RENDERER_TYPE_COMPUTE_2D
#define RENDERER_TYPE_COMPUTE_2D "Compute 2D"
#endif
#ifndef RENDERER_TYPE_COMPUTE_3D
#define RENDERER_TYPE_COMPUTE_3D "Compute 3D"
#endif
#ifndef RENDERER_TYPE_RTX
#define RENDERER_TYPE_RTX "Rtx"
#endif

#ifndef RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD
#define RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD "Quad"
#endif
#ifndef RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH
#define RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH "Mesh"
#endif
#ifndef RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_TESSELATION
#define RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_TESSELATION "Tesselation"
#endif
#ifndef RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX
#define RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX "Vertex"
#endif

struct SlotStringStruct
{
	std::string node_slot_func;
	std::string node_module_public_interface;
	std::string pass_public_interface;
	std::string include_slot;
	std::string include_interface;
	std::string cpp_node_func;
    std::string cpp_module_private_var;
    std::string cpp_module_func;
    std::string cpp_pass_private_var;
	std::string cpp_pass_func;
	std::string h_func;
};
typedef std::map<BaseTypeEnum, std::map<NodeSlot::PlaceEnum, std::vector<SlotStringStruct>>> SlotDico;

class ProjectFile;
class GeneratorNode : public BaseNode
{
public: // not to save
	BaseTypes m_BaseTypes;

private: // not to save
	std::map<BaseTypeEnum, uint32_t> m_InputSlotCounter;
	std::map<BaseTypeEnum, uint32_t> m_OutputSlotCounter;
	bool m_ShowInputWidgets = false;
	bool m_ShowOutputWidgets = false;

public: // to save
	// Node
	std::string m_ClassName = "NewClass";
	std::string m_CategoryName = "None";
	std::string m_NodeCreationName = "NEW_NODE"; // node name maj ex : 2D_SIMULATION_GRAY_SCOTT
	std::string m_NodeDisplayName = "New Node"; // node name maj ex : Gray Scott
    bool m_IsATask = true;
    bool m_IsAnEffect = false;

	// Module
	bool m_GenerateAModule = true;
	std::string m_ModuleDisplayName = "New Node"; // node name maj ex : Gray Scott
	std::string m_RendererType = "Comp";
    std::string m_ModuleXmlName = "toto_module";
    std::string m_PassXmlName = "toto";
	std::string m_ModuleRendererInitFunc = "InitCompute2D"; // from m_RendererType // dont save
	std::string m_ModuleRendererDisplayType = "Comp"; // from m_RendererType // dont save

	// Pass
	bool m_GenerateAPass = true;
	std::string m_RendererTypePixel2DSpecializationType = "Quad";
	bool m_UseASbo = false;
	int32_t m_VertexStructTypesIndex = 0;
	UBOEditors m_UBOEditors;

public:
	static GeneratorNodePtr Create(GaiApi::VulkanCorePtr vVulkanCorePtr);
	static std::string GetLicenceHeader();
	static std::string GetPVSStudioHeader();

public:
	GeneratorNode();

public:
	void GenerateNodeClasses(const std::string& vPath);
	void GenerateModules(const std::string& vPath, const SlotDico& vDico);
	void GeneratePasses(const std::string& vPath, const SlotDico& vDico);

private:
	std::string GetRendererDisplayName();
	std::string GetPassRendererFunctionHeader();
	std::string GetPassUpdateLayoutBindingInRessourceDescriptorHeader();
	std::string GetPassUpdateBufferInfoInRessourceDescriptorHeader();
	std::string GetPassShaderCode();
	std::string GetPassShaderHeader();
	
private: // Pass Shader Code
	std::string GetPixel2DSpecializationQuad();
	std::string GetPixel2DSpecializationMesh();
	std::string GetPixel2DSpecializationTesselation();
	std::string GetPixel2DSpecializationVertex();

private:
    SlotDico GetSlotDico();
    
	SlotStringStruct GetSlotTaskInput(NodeSlotInputPtr vSlot);
    SlotStringStruct GetSlotNoneInput(NodeSlotInputPtr vSlot);
    SlotStringStruct GetSlotModelInput(NodeSlotInputPtr vSlot);
    SlotStringStruct GetSlotCustomInput(NodeSlotInputPtr vSlot);
    SlotStringStruct GetSlotTextureInput(NodeSlotInputPtr vSlot);
    SlotStringStruct GetSlotVariableInput(NodeSlotInputPtr vSlot);
    SlotStringStruct GetSlotShaderPassInput(NodeSlotInputPtr vSlot);
    SlotStringStruct GetSlotLightGroupInput(NodeSlotInputPtr vSlot);
    SlotStringStruct GetSlotTextureCubeInput(NodeSlotInputPtr vSlot);
    SlotStringStruct GetSlotTexelBufferInput(NodeSlotInputPtr vSlot);
    SlotStringStruct GetSlotTextureGroupInput(NodeSlotInputPtr vSlot);
    SlotStringStruct GetSlotStorageBufferInput(NodeSlotInputPtr vSlot);
    SlotStringStruct GetSlotAccelStructureInput(NodeSlotInputPtr vSlot);

    SlotStringStruct GetSlotTaskOutput(NodeSlotOutputPtr vSlot);
    SlotStringStruct GetSlotNoneOutput(NodeSlotOutputPtr vSlot);
    SlotStringStruct GetSlotModelOutput(NodeSlotOutputPtr vSlot);
    SlotStringStruct GetSlotCustomOutput(NodeSlotOutputPtr vSlot);
    SlotStringStruct GetSlotTextureOutput(NodeSlotOutputPtr vSlot);
    SlotStringStruct GetSlotVariableOutput(NodeSlotOutputPtr vSlot);
    SlotStringStruct GetSlotShaderPassOutput(NodeSlotOutputPtr vSlot);
    SlotStringStruct GetSlotLightGroupOutput(NodeSlotOutputPtr vSlot);
	SlotStringStruct GetSlotTextureCubeOutput(NodeSlotOutputPtr vSlot);
    SlotStringStruct GetSlotTexelBufferOutput(NodeSlotOutputPtr vSlot);
    SlotStringStruct GetSlotTextureGroupOutput(NodeSlotOutputPtr vSlot);
    SlotStringStruct GetSlotStorageBufferOutput(NodeSlotOutputPtr vSlot);
    SlotStringStruct GetSlotAccelStructureOutput(NodeSlotOutputPtr vSlot);

private:
    std::string GetPassInputCppFuncs(const SlotDico& vDico);
    std::string GetModuleInputCppFuncs(const SlotDico& vDico);
    std::string GetNodeSlotsInputFuncs(const SlotDico& vDico);
    std::string GetPassInputPrivateVars(const SlotDico& vDico);
    std::string GetNodeSlotsInputHFuncs(const SlotDico& vDico);
    std::string GetNodeSlotsInputCppFuncs(const SlotDico& vDico);
    std::string GetModuleInputPrivateVars(const SlotDico& vDico);
    std::string GetPassInputPublicInterfaces(const SlotDico& vDico);
    std::string GetNodeSlotsInputIncludesSlots(const SlotDico& vDico);
    std::string GetNodeSlotsInputPublicInterfaces(const SlotDico& vDico);
    std::string GetNodeSlotsInputIncludesInterfaces(const SlotDico& vDico);
    
	std::string GetPassOutputCppFuncs(const SlotDico& vDico);
	std::string GetModuleOutputCppFuncs(const SlotDico& vDico);
    std::string GetNodeSlotsOutputFuncs(const SlotDico& vDico);
    std::string GetPassOutputPrivateVars(const SlotDico& vDico);
    std::string GetNodeSlotsOutputHFuncs(const SlotDico& vDico);
    std::string GetModuleOutputPrivateVars(const SlotDico& vDico);
    std::string GetNodeSlotsOutputCppFuncs(const SlotDico& vDico);
    std::string GetPassOutputPublicInterfaces(const SlotDico& vDico);
    std::string GetNodeSlotsOutputIncludesSlots(const SlotDico& vDico);
    std::string GetNodeSlotsOutputPublicInterfaces(const SlotDico& vDico);
	std::string GetNodeSlotsOutputIncludesInterfaces(const SlotDico& vDico);
	
public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
};