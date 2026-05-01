#include "Corium.h"
#include "CudaGraph.h"

#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Corium::Cuda::Graphs {
	GpuGraph DeviceGraphs::createGraph() {
		CUgraph graph;
		const CUresult result = cuGraphCreate(&graph, 0);

		if (result == CUDA_SUCCESS) {
			GpuGraph ro_Graph;
			ro_Graph.m_GraphHandle = graph;
			return ro_Graph;
		}

		CUDA_ERROR_TRAP(result);
		return GpuGraph{};
	}

	void DeviceGraphs::destroyGraph(GpuGraph& ro_Graph) {
		if (!ro_Graph.isValid()) return;

		const CUresult result = cuGraphDestroy(static_cast<CUgraph>(ro_Graph.m_GraphHandle));
		if (result == CUDA_SUCCESS) {
			ro_Graph.m_GraphHandle = nullptr;
			return;
		}

		CUDA_ERROR_TRAP(result);
	}

	GpuGraphNode DeviceGraphs::addKernelNode(GpuGraph& ro_Graph, const GpuGraphNode* p_Deps, uint32_t v_DepCount, const KernelNodeParams& ro_Params) {
		CORIUM_ASSERT(ro_Graph.isValid());
		CORIUM_ASSERT(ro_Params.m_Function != nullptr);
		CORIUM_ASSERT(v_DepCount <= MAX_GRAPH_DEPS);

		CUgraphNode depBuf[MAX_GRAPH_DEPS];
		for (uint32_t i = 0; i < v_DepCount; ++i)
			depBuf[i] = static_cast<CUgraphNode>(p_Deps[i].m_NodeHandle);

		const CUDA_KERNEL_NODE_PARAMS params = Internal::CUDA_PackingFunctions::packKernelNodeParams(ro_Params);
		CUgraphNode node;
		const CUresult result = cuGraphAddKernelNode(
			&node,
			static_cast<CUgraph>(ro_Graph.m_GraphHandle),
			v_DepCount > 0 ? depBuf : nullptr,
			static_cast<size_t>(v_DepCount),
			&params
		);

		if (result == CUDA_SUCCESS) {
			GpuGraphNode ro_Node;
			ro_Node.m_NodeHandle = node;
			return ro_Node;
		}

		CUDA_ERROR_TRAP(result);
		return GpuGraphNode{};
	}

	GpuGraphNode DeviceGraphs::addMemcpyNode(GpuGraph& ro_Graph, const GpuGraphNode* p_Deps, uint32_t v_DepCount, const MemCpy3DDesc& ro_Desc) {
		CORIUM_ASSERT(ro_Graph.isValid());
		CORIUM_ASSERT(v_DepCount <= MAX_GRAPH_DEPS);

		CUgraphNode depBuf[MAX_GRAPH_DEPS];
		for (uint32_t i = 0; i < v_DepCount; ++i)
			depBuf[i] = static_cast<CUgraphNode>(p_Deps[i].m_NodeHandle);

		const CUDA_MEMCPY3D copyParams = Internal::CUDA_PackingFunctions::pack3dMemcpyDesc(ro_Desc);

		CUcontext ctx{};
		cuCtxGetCurrent(&ctx);

		CUgraphNode node;
		const CUresult result = cuGraphAddMemcpyNode(
			&node,
			static_cast<CUgraph>(ro_Graph.m_GraphHandle),
			v_DepCount > 0 ? depBuf : nullptr,
			static_cast<size_t>(v_DepCount),
			&copyParams,
			ctx
		);

		if (result == CUDA_SUCCESS) {
			GpuGraphNode ro_Node;
			ro_Node.m_NodeHandle = node;
			return ro_Node;
		}

		CUDA_ERROR_TRAP(result);
		return GpuGraphNode{};
	}

	GpuGraphNode DeviceGraphs::addMemsetNode(GpuGraph& ro_Graph, const GpuGraphNode* p_Deps, uint32_t v_DepCount, const MemsetNodeParams& ro_Params) {
		CORIUM_ASSERT(ro_Graph.isValid());
		CORIUM_ASSERT(v_DepCount <= MAX_GRAPH_DEPS);

		CUgraphNode depBuf[MAX_GRAPH_DEPS];
		for (uint32_t i = 0; i < v_DepCount; ++i)
			depBuf[i] = static_cast<CUgraphNode>(p_Deps[i].m_NodeHandle);

		const CUDA_MEMSET_NODE_PARAMS params = Internal::CUDA_PackingFunctions::packMemsetNodeParams(ro_Params);

		CUcontext ctx{};
		cuCtxGetCurrent(&ctx);

		CUgraphNode node;
		const CUresult result = cuGraphAddMemsetNode(
			&node,
			static_cast<CUgraph>(ro_Graph.m_GraphHandle),
			v_DepCount > 0 ? depBuf : nullptr,
			static_cast<size_t>(v_DepCount),
			&params,
			ctx
		);

		if (result == CUDA_SUCCESS) {
			GpuGraphNode ro_Node;
			ro_Node.m_NodeHandle = node;
			return ro_Node;
		}

		CUDA_ERROR_TRAP(result);
		return GpuGraphNode{};
	}

	void DeviceGraphs::addDependencies(GpuGraph& ro_Graph, const GpuGraphNode* p_From, const GpuGraphNode* p_To, uint32_t v_Count) {
		CORIUM_ASSERT(ro_Graph.isValid());
		CORIUM_ASSERT(p_From != nullptr);
		CORIUM_ASSERT(p_To != nullptr);
		CORIUM_ASSERT(v_Count > 0 && v_Count <= MAX_GRAPH_DEPS);

		CUgraphNode fromBuf[MAX_GRAPH_DEPS];
		CUgraphNode toBuf[MAX_GRAPH_DEPS];
		for (uint32_t i = 0; i < v_Count; ++i) {
			fromBuf[i] = static_cast<CUgraphNode>(p_From[i].m_NodeHandle);
			toBuf[i]   = static_cast<CUgraphNode>(p_To[i].m_NodeHandle);
		}

		const CUresult result = cuGraphAddDependencies(
			static_cast<CUgraph>(ro_Graph.m_GraphHandle),
			fromBuf,
			toBuf,
			static_cast<size_t>(v_Count)
		);

		if (result == CUDA_SUCCESS) return;

		CUDA_ERROR_TRAP(result);
	}

	GpuGraphExec DeviceGraphs::instantiate(const GpuGraph& ro_Graph) {
		CORIUM_ASSERT(ro_Graph.isValid());

		CUgraphExec exec;
		const CUresult result = cuGraphInstantiate(&exec, static_cast<CUgraph>(ro_Graph.m_GraphHandle), 0);

		if (result == CUDA_SUCCESS) {
			GpuGraphExec ro_Exec;
			ro_Exec.m_ExecHandle = exec;
			return ro_Exec;
		}

		CUDA_ERROR_TRAP(result);
		return GpuGraphExec{};
	}

	void DeviceGraphs::launch(const GpuGraphExec& ro_Exec, const GpuStream& ro_Stream) {
		CORIUM_ASSERT(ro_Exec.isValid());
		CORIUM_ASSERT(ro_Stream.isValid());

		const CUresult result = cuGraphLaunch(
			static_cast<CUgraphExec>(ro_Exec.m_ExecHandle),
			static_cast<CUstream>(ro_Stream.m_StreamHandle)
		);

		if (result == CUDA_SUCCESS) return;

		CUDA_ERROR_TRAP(result);
	}

	bool DeviceGraphs::execUpdate(GpuGraphExec& ro_Exec, const GpuGraph& ro_NewGraph) {
		CORIUM_ASSERT(ro_Exec.isValid());
		CORIUM_ASSERT(ro_NewGraph.isValid());

		CUgraphExecUpdateResultInfo resultInfo{};
		const CUresult result = cuGraphExecUpdate(
			static_cast<CUgraphExec>(ro_Exec.m_ExecHandle),
			static_cast<CUgraph>(ro_NewGraph.m_GraphHandle),
			&resultInfo
		);

		if (result == CUDA_SUCCESS && resultInfo.result == CU_GRAPH_EXEC_UPDATE_SUCCESS) return true;
		return false;
	}

	void DeviceGraphs::destroyExec(GpuGraphExec& ro_Exec) {
		if (!ro_Exec.isValid()) return;

		const CUresult result = cuGraphExecDestroy(static_cast<CUgraphExec>(ro_Exec.m_ExecHandle));
		if (result == CUDA_SUCCESS) {
			ro_Exec.m_ExecHandle = nullptr;
			return;
		}

		CUDA_ERROR_TRAP(result);
	}
}
