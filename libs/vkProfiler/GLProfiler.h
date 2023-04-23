#pragma once

#include <cstdint>
#ifdef USE_OPENGL
#include <glad/glad.h>
#endif
#include <ctools/cTools.h>

#include <set>
#include <unordered_map>
#include <memory> // smart pointer

// a main zone for the frame must always been defined for the frame
#define GLPNewFrame(section, fmt, ...) auto __GLP__ScopedMainZone = glp::ScopedZone(true, section, fmt, ## __VA_ARGS__); (void)__GLP__ScopedMainZone
#define GLPScoped(section, fmt, ...) auto __GLP__ScopedSubZone = glp::ScopedZone(false, section, fmt, ## __VA_ARGS__); (void)__GLP__ScopedSubZone

namespace glp
{
	class AverageValue
	{
	private:
		double prPerFrame[60];
		int prPerFrameIdx = 0;
		double prPerFrameAccum = 0.0;
		double prAverageValue = 0.0;

	public:
		AverageValue();
		void AddValue(double vValue);
		double GetAverage();
	};

	class QueryZone
	{
	public:
		static uint32_t sMaxDepthToOpen;
		static bool sShowLeafMode;
		static float sContrastRatio;
		static bool sActivateLogger;

	public:
		uint32_t puDepth = 0U; // the puDepth of the QueryZone
		uint32_t puIds[2] = { 0U, 0U };
		std::vector<std::shared_ptr<QueryZone>> puZonesOrdered;
		std::unordered_map<std::string, std::shared_ptr<QueryZone>> puZonesDico; // main container
		std::string puName;
		std::shared_ptr<QueryZone> puThis = nullptr;
		std::shared_ptr<QueryZone> puTopQuery = nullptr;

	private:
		bool prIsRoot = false;
		double prElapsedTime = 0.0;
		double prStartTime = 0.0;
		double prEndTime = 0.0;
		uint32_t prStartFrameId = 0;
		uint32_t prEndFrameId = 0;
		uint64_t prStartTimeStamp = 0;
		uint64_t prEndTimeStamp = 0;
		ct::ActionTime prActionTime;
		bool prExpanded = false;
		bool prHighlighted = false;
		AverageValue prAverageStartValue;
		AverageValue prAverageEndValue;
		GuiBackend_Window prThread;
		std::string prBarLabel;
		std::string prSectionName;
		float prBarWidth = 0.0f;
		float prBarPos = 0.0f;

	public:
		QueryZone(const GuiBackend_Window& vThread, const std::string& vName, const std::string& vSectionName, const bool& vIsRoot = false);
		~QueryZone();
		void Clear();
		void SetStartTimeStamp(const uint64_t& vValue);
		void SetEndTimeStamp(const uint64_t& vValue);
		void ComputeElapsedTime();
		void DrawMetricLabels();
		bool DrawMetricGraph(std::shared_ptr<QueryZone> vParent = nullptr, uint32_t vDepth = 0);
	};

	class GLContext
	{
	private:
		GuiBackend_Window prThread;
		std::shared_ptr<QueryZone> prRootZone = nullptr;
		std::unordered_map<uint32_t, std::shared_ptr<QueryZone>> prQueryIDToZone; // Get the zone for a query id because a query have to id's : start and end
		std::unordered_map<uint32_t, std::shared_ptr<QueryZone>> prDepthToLastZone; // last zone registered at this puDepth
		std::set<uint32_t> prPendingUpdate; // some queries msut but retrieveds

	public:
		GLContext(const GuiBackend_Window& vThread);
		void Clear();
		void Init();
		void Unit();
		void Collect();
		void Draw();
		std::shared_ptr<QueryZone> GetQueryZoneForName(const std::string& vName, const std::string& vSection = "", const bool& vIsRoot = false);

	private:
		void SetQueryZoneForDepth(std::shared_ptr<QueryZone> vQueryZone, uint32_t vDepth);
		std::shared_ptr<QueryZone> GetQueryZoneFromDepth(uint32_t vDepth);
	};

	class GLProfiler
	{
	public:
		bool puIsActive = false;	
		bool puIsPaused = false;

	private:
		std::unordered_map<intptr_t, std::shared_ptr<GLContext>> prContexts;

	public:
		void Clear();
		void Init();
		void Unit();
		void Collect();
		void Draw();
		std::shared_ptr<GLContext> GetContext(const GuiBackend_Window& vThread);

	public:
		static GLProfiler* Instance()
		{
			static GLProfiler _instance;
			return &_instance;
		}

	protected:
		GLProfiler() { Init(); }; // Prevent construction
		GLProfiler(const GLProfiler&) {}; // Prevent construction by copying
		GLProfiler& operator =(const GLProfiler&) { return *this; }; // Prevent assignment
		~GLProfiler() { Unit(); }; // Prevent unwanted destruction
	};

	class ScopedZone
	{
	public:
		static uint32_t sCurrentDepth; // Current Depth catched Profiler
		static uint32_t sMaxDepth; // max puDepth encountered ever

	public:
		std::shared_ptr<QueryZone> query = nullptr;

	public:
		ScopedZone(bool vIsRoot, std::string vSection, const char* fmt, ...);
		~ScopedZone();
	};
}
