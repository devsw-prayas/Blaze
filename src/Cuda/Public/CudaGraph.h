#pragma once
#include "Corium.h"
#include "CoriumCompiler.h"
#include "CudaUtils.h"

namespace Corium::Cuda::Graphs {
	using namespace Utils;

	static constexpr uint32_t MAX_GRAPH_DEPS = 64;

	class CORIUM_RUNTIME_API DeviceGraphs final {
	public:
		// Graph lifecycle
		static GpuGraph     createGraph();
		static void         destroyGraph(GpuGraph& ro_Graph);

		// Node addition — returns the newly created node handle.
		// p_Deps / v_DepCount: optional array of predecessor nodes (nullptr + 0 = root node).
		static GpuGraphNode addKernelNode(GpuGraph& ro_Graph, const GpuGraphNode* p_Deps, uint32_t v_DepCount, const KernelNodeParams& ro_Params);
		static GpuGraphNode addMemcpyNode(GpuGraph& ro_Graph, const GpuGraphNode* p_Deps, uint32_t v_DepCount, const MemCpy3DDesc& ro_Desc);
		static GpuGraphNode addMemsetNode(GpuGraph& ro_Graph, const GpuGraphNode* p_Deps, uint32_t v_DepCount, const MemsetNodeParams& ro_Params);

		// Explicit dependency edges between already-added nodes.
		// p_From[i] must complete before p_To[i] begins. v_Count <= MAX_GRAPH_DEPS.
		static void addDependencies(GpuGraph& ro_Graph, const GpuGraphNode* p_From, const GpuGraphNode* p_To, const GpuGraphEdgeData* p_Data = nullptr, uint32_t v_Count = 0);

		// Compilation & execution
		static GpuGraphExec instantiate(const GpuGraph& ro_Graph);
		static void         launch(const GpuGraphExec& ro_Exec, const GpuStream& ro_Stream);

		// Hot-patch kernel parameters without full re-instantiation.
		// Returns true on success; false means topology changed — destroy and re-instantiate.
		static bool execUpdate(GpuGraphExec& ro_Exec, const GpuGraph& ro_NewGraph);
		static void destroyExec(GpuGraphExec& ro_Exec);
	};
}
