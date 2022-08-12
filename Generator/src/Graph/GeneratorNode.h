#pragma once

#include <string>
#include <memory>
#include <vector>

enum BaseTypeEnum : uint32_t
{
	BASE_TYPE_None = 0,
	BASE_TYPE_LightGroup,
	BASE_TYPE_Model,
	BASE_TYPE_StorageBuffer,
	BASE_TYPE_TexelBuffer,
	BASE_TYPE_Texture,
	BASE_TYPE_TextureGroup,
	BASE_TYPE_Variable,
	BASE_TYPE_Custom
};

class BaseTypes
{
public:
	std::vector<std::string> m_TypeArray =
	{
		"None",
		"LightGroup",
		"Model",
		"StorageBuffer",
		"TexelBuffer",
		"Texture",
		"TextureGroup",
		"Variable",
		"Custom"
	};
};

#include <ctools/cTools.h>

class GeneratorNode;
typedef ct::cWeak<GeneratorNode> GeneratorNodeWeak;
typedef std::shared_ptr<GeneratorNode> GeneratorNodePtr;

#include <Graph/Graph.h>
#include <Graph/Base/BaseNode.h>
#include <vkFramework/VulkanCore.h>
#include <Project/ProjectFile.h>
#include <Graph/Base/NodeSlot.h>

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

struct SlotStringStruct
{
	std::string node_slot_func;
	std::string node_public_interface;
	std::string include_slot;
	std::string include_interface;
	std::string cpp_func;
	std::string h_func;
};
typedef std::map<BaseTypeEnum, std::map<NodeSlot::PlaceEnum, SlotStringStruct>> SlotDico;

class ProjectFile;
class GeneratorNode : public BaseNode
{
private:
	BaseTypes m_BaseTypes;
	std::map<uint32_t, uint32_t> m_InputSlotCounter;
	std::map<uint32_t, uint32_t> m_OutputSlotCounter;
public:
	// Node
	std::string m_ClassName = "NewClass";
	std::string m_CategoryName = "None";
	std::string m_NodeCreationName = "NEW_NODE"; // node name maj ex : 2D_SIMULATION_GRAY_SCOTT
	std::string m_NodeDisplayName = "New Node"; // node name maj ex : Gray Scott
	
	// Module
	bool m_GenerateAModule = false;
	std::string m_ModuleDisplayName = "New Node"; // node name maj ex : Gray Scott
	std::string m_RendererType = "Comp";
	std::string m_ModuleXmlName = "toto_module";
	std::string m_ModuleRendererInitFunc = "InitCompute2D"; // from m_RendererType
	std::string m_ModuleRendererDisplayType = "Comp"; // from m_RendererType

	// Pass
	bool m_GenerateAPass = false;

public:
	static GeneratorNodePtr Create(vkApi::VulkanCorePtr vVulkanCorePtr);

public:
	GeneratorNode();

public:
	void GenerateNodeClasses(const std::string& vPath, const ProjectFile* vDatas);
	void GenerateModules(const std::string& vPath, const ProjectFile* vDatas);
	void GeneratePasses(const std::string& vPath, const ProjectFile* vDatas);

private:
	std::string GetLicenceHeader();
	std::string GetPVSStudioHeader();
	std::string GetRendererDisplayName();
	
private:
	SlotDico GetSlotDico();
	SlotStringStruct GetSlotNoneInput(NodeSlotInputPtr vSlot);
	SlotStringStruct GetSlotNoneOutput(NodeSlotOutputPtr vSlot);
	SlotStringStruct GetSlotLightGroupInput(NodeSlotInputPtr vSlot);
	SlotStringStruct GetSlotLightGroupOutput(NodeSlotOutputPtr vSlot);
	SlotStringStruct GetSlotModelInput(NodeSlotInputPtr vSlot);
	SlotStringStruct GetSlotModelOutput(NodeSlotOutputPtr vSlot);
	SlotStringStruct GetSlotStorageBufferInput(NodeSlotInputPtr vSlot);
	SlotStringStruct GetSlotStorageBufferOutput(NodeSlotOutputPtr vSlot);
	SlotStringStruct GetSlotTexelBufferInput(NodeSlotInputPtr vSlot);
	SlotStringStruct GetSlotTexelBufferOutput(NodeSlotOutputPtr vSlot);
	SlotStringStruct GetSlotTextureInput(NodeSlotInputPtr vSlot);
	SlotStringStruct GetSlotTextureOutput(NodeSlotOutputPtr vSlot);
	SlotStringStruct GetSlotTextureGroupInput(NodeSlotInputPtr vSlot);
	SlotStringStruct GetSlotTextureGroupOutput(NodeSlotOutputPtr vSlot);
	SlotStringStruct GetSlotVariableInput(NodeSlotInputPtr vSlot);
	SlotStringStruct GetSlotVariableOutput(NodeSlotOutputPtr vSlot);
	SlotStringStruct GetSlotCustomInput(NodeSlotInputPtr vSlot);
	SlotStringStruct GetSlotCustomOutput(NodeSlotOutputPtr vSlot);

private:
	std::string GetNodeSlotsInputFuncs(const SlotDico& vDico);
	std::string GetNodeSlotsOutputFuncs(const SlotDico& vDico);
	std::string GetNodeSlotsInputPublicInterfaces(const SlotDico& vDico);
	std::string GetNodeSlotsOutputPublicInterfaces(const SlotDico& vDico);
	std::string GetNodeSlotsInputIncludesSlots(const SlotDico& vDico);
	std::string GetNodeSlotsOutputIncludesSlots(const SlotDico& vDico);
	std::string GetNodeSlotsInputIncludesInterfaces(const SlotDico& vDico);
	std::string GetNodeSlotsOutputIncludesInterfaces(const SlotDico& vDico);
	std::string GetNodeSlotsInputCppFuncs(const SlotDico& vDico);
	std::string GetNodeSlotsOutputCppFuncs(const SlotDico& vDico);
	std::string GetNodeSlotsInputHFuncs(const SlotDico& vDico);
	std::string GetNodeSlotsOutputHFuncs(const SlotDico& vDico);

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
};