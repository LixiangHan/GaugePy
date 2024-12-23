#include "gpu_hpc_profiling.hpp"

bool CreateCounterDataImage(
	std::vector<uint8_t> &counterDataImage,
	std::vector<uint8_t> &counterDataScratchBuffer,
	std::vector<uint8_t> &counterDataImagePrefix)
{

	CUpti_Profiler_CounterDataImageOptions counterDataImageOptions;
	counterDataImageOptions.pCounterDataPrefix = &counterDataImagePrefix[0];
	counterDataImageOptions.counterDataPrefixSize = counterDataImagePrefix.size();
	counterDataImageOptions.maxNumRanges = numRanges;
	counterDataImageOptions.maxNumRangeTreeNodes = numRanges;
	counterDataImageOptions.maxRangeNameLength = 64;

	CUpti_Profiler_CounterDataImage_CalculateSize_Params calculateSizeParams = {CUpti_Profiler_CounterDataImage_CalculateSize_Params_STRUCT_SIZE};

	calculateSizeParams.pOptions = &counterDataImageOptions;
	calculateSizeParams.sizeofCounterDataImageOptions = CUpti_Profiler_CounterDataImageOptions_STRUCT_SIZE;

	CUPTI_API_CALL(cuptiProfilerCounterDataImageCalculateSize(&calculateSizeParams));

	CUpti_Profiler_CounterDataImage_Initialize_Params initializeParams = {CUpti_Profiler_CounterDataImage_Initialize_Params_STRUCT_SIZE};
	initializeParams.sizeofCounterDataImageOptions = CUpti_Profiler_CounterDataImageOptions_STRUCT_SIZE;
	initializeParams.pOptions = &counterDataImageOptions;
	initializeParams.counterDataImageSize = calculateSizeParams.counterDataImageSize;

	counterDataImage.resize(calculateSizeParams.counterDataImageSize);
	initializeParams.pCounterDataImage = &counterDataImage[0];
	CUPTI_API_CALL(cuptiProfilerCounterDataImageInitialize(&initializeParams));

	CUpti_Profiler_CounterDataImage_CalculateScratchBufferSize_Params scratchBufferSizeParams = {CUpti_Profiler_CounterDataImage_CalculateScratchBufferSize_Params_STRUCT_SIZE};
	scratchBufferSizeParams.counterDataImageSize = calculateSizeParams.counterDataImageSize;
	scratchBufferSizeParams.pCounterDataImage = initializeParams.pCounterDataImage;
	CUPTI_API_CALL(cuptiProfilerCounterDataImageCalculateScratchBufferSize(&scratchBufferSizeParams));

	counterDataScratchBuffer.resize(scratchBufferSizeParams.counterDataScratchBufferSize);

	CUpti_Profiler_CounterDataImage_InitializeScratchBuffer_Params initScratchBufferParams = {CUpti_Profiler_CounterDataImage_InitializeScratchBuffer_Params_STRUCT_SIZE};
	initScratchBufferParams.counterDataImageSize = calculateSizeParams.counterDataImageSize;

	initScratchBufferParams.pCounterDataImage = initializeParams.pCounterDataImage;
	initScratchBufferParams.counterDataScratchBufferSize = scratchBufferSizeParams.counterDataScratchBufferSize;
	initScratchBufferParams.pCounterDataScratchBuffer = &counterDataScratchBuffer[0];

	CUPTI_API_CALL(cuptiProfilerCounterDataImageInitializeScratchBuffer(&initScratchBufferParams));

	return true;
}

bool runTest(py::function callback,
			 std::vector<uint8_t> &configImage,
			 std::vector<uint8_t> &counterDataScratchBuffer,
			 std::vector<uint8_t> &counterDataImage,
			 CUpti_ProfilerReplayMode profilerReplayMode,
			 CUpti_ProfilerRange profilerRange)
{
	CUcontext cuContext;
	DRIVER_API_CALL(cuCtxGetCurrent(&cuContext));

	CUpti_Profiler_BeginSession_Params beginSessionParams = {CUpti_Profiler_BeginSession_Params_STRUCT_SIZE};
	CUpti_Profiler_SetConfig_Params setConfigParams = {CUpti_Profiler_SetConfig_Params_STRUCT_SIZE};
	CUpti_Profiler_EnableProfiling_Params enableProfilingParams = {CUpti_Profiler_EnableProfiling_Params_STRUCT_SIZE};
	CUpti_Profiler_DisableProfiling_Params disableProfilingParams = {CUpti_Profiler_DisableProfiling_Params_STRUCT_SIZE};
	CUpti_Profiler_PushRange_Params pushRangeParams = {CUpti_Profiler_PushRange_Params_STRUCT_SIZE};
	CUpti_Profiler_PopRange_Params popRangeParams = {CUpti_Profiler_PopRange_Params_STRUCT_SIZE};

	beginSessionParams.ctx = NULL;
	beginSessionParams.counterDataImageSize = counterDataImage.size();
	beginSessionParams.pCounterDataImage = &counterDataImage[0];
	beginSessionParams.counterDataScratchBufferSize = counterDataScratchBuffer.size();
	beginSessionParams.pCounterDataScratchBuffer = &counterDataScratchBuffer[0];
	beginSessionParams.range = profilerRange;
	beginSessionParams.replayMode = profilerReplayMode;
	beginSessionParams.maxRangesPerPass = numRanges;
	beginSessionParams.maxLaunchesPerPass = numRanges;

	CUPTI_API_CALL(cuptiProfilerBeginSession(&beginSessionParams));

	setConfigParams.pConfig = &configImage[0];
	setConfigParams.configSize = configImage.size();

	setConfigParams.passIndex = 0;
	setConfigParams.minNestingLevel = 1;
	setConfigParams.numNestingLevels = 1;
	CUPTI_API_CALL(cuptiProfilerSetConfig(&setConfigParams));
	/* User takes the resposiblity of replaying the kernel launches */
	CUpti_Profiler_BeginPass_Params beginPassParams = {CUpti_Profiler_BeginPass_Params_STRUCT_SIZE};
	CUpti_Profiler_EndPass_Params endPassParams = {CUpti_Profiler_EndPass_Params_STRUCT_SIZE};
	do
	{
		CUPTI_API_CALL(cuptiProfilerBeginPass(&beginPassParams));
		{
			CUPTI_API_CALL(cuptiProfilerEnableProfiling(&enableProfilingParams));
			std::string rangeName = "userrangeA";
			pushRangeParams.pRangeName = rangeName.c_str();
			CUPTI_API_CALL(cuptiProfilerPushRange(&pushRangeParams));
			{
				callback();
			}
			CUPTI_API_CALL(cuptiProfilerPopRange(&popRangeParams));
			CUPTI_API_CALL(cuptiProfilerDisableProfiling(&disableProfilingParams));
		}
		CUPTI_API_CALL(cuptiProfilerEndPass(&endPassParams));
	} while (!endPassParams.allPassesSubmitted);
	CUpti_Profiler_FlushCounterData_Params flushCounterDataParams = {CUpti_Profiler_FlushCounterData_Params_STRUCT_SIZE};
	CUPTI_API_CALL(cuptiProfilerFlushCounterData(&flushCounterDataParams));
	CUpti_Profiler_UnsetConfig_Params unsetConfigParams = {CUpti_Profiler_UnsetConfig_Params_STRUCT_SIZE};
	CUPTI_API_CALL(cuptiProfilerUnsetConfig(&unsetConfigParams));
	CUpti_Profiler_EndSession_Params endSessionParams = {CUpti_Profiler_EndSession_Params_STRUCT_SIZE};
	CUPTI_API_CALL(cuptiProfilerEndSession(&endSessionParams));

	return true;
}

bool ParseMetricValues(std::map<std::string, float> &metricValues,
					   std::string chipName,
					   const std::vector<uint8_t> &counterDataImage,
					   const std::vector<std::string> &metricNames,
					   const uint8_t *pCounterAvailabilityImage)
{
	if (!counterDataImage.size())
	{
		std::cout << "Counter Data Image is empty!\n";
		return false;
	}

	NVPW_CUDA_MetricsEvaluator_CalculateScratchBufferSize_Params calculateScratchBufferSizeParam = {NVPW_CUDA_MetricsEvaluator_CalculateScratchBufferSize_Params_STRUCT_SIZE};
	calculateScratchBufferSizeParam.pChipName = chipName.c_str();
	calculateScratchBufferSizeParam.pCounterAvailabilityImage = pCounterAvailabilityImage;
	RETURN_IF_NVPW_ERROR(false, NVPW_CUDA_MetricsEvaluator_CalculateScratchBufferSize(&calculateScratchBufferSizeParam));

	std::vector<uint8_t> scratchBuffer(calculateScratchBufferSizeParam.scratchBufferSize);
	NVPW_CUDA_MetricsEvaluator_Initialize_Params metricEvaluatorInitializeParams = {NVPW_CUDA_MetricsEvaluator_Initialize_Params_STRUCT_SIZE};
	metricEvaluatorInitializeParams.scratchBufferSize = scratchBuffer.size();
	metricEvaluatorInitializeParams.pScratchBuffer = scratchBuffer.data();
	metricEvaluatorInitializeParams.pChipName = chipName.c_str();
	metricEvaluatorInitializeParams.pCounterAvailabilityImage = pCounterAvailabilityImage;
	metricEvaluatorInitializeParams.pCounterDataImage = counterDataImage.data();
	metricEvaluatorInitializeParams.counterDataImageSize = counterDataImage.size();
	RETURN_IF_NVPW_ERROR(false, NVPW_CUDA_MetricsEvaluator_Initialize(&metricEvaluatorInitializeParams));
	NVPW_MetricsEvaluator *metricEvaluator = metricEvaluatorInitializeParams.pMetricsEvaluator;

	NVPW_CounterData_GetNumRanges_Params getNumRangesParams = {NVPW_CounterData_GetNumRanges_Params_STRUCT_SIZE};
	getNumRangesParams.pCounterDataImage = counterDataImage.data();
	RETURN_IF_NVPW_ERROR(false, NVPW_CounterData_GetNumRanges(&getNumRangesParams));

	std::string reqName;
	bool isolated = true;
	bool keepInstances = true;
	for (std::string metricName : metricNames)
	{
		NV::Metric::Parser::ParseMetricNameString(metricName, &reqName, &isolated, &keepInstances);
		NVPW_MetricEvalRequest metricEvalRequest;
		NVPW_MetricsEvaluator_ConvertMetricNameToMetricEvalRequest_Params convertMetricToEvalRequest = {NVPW_MetricsEvaluator_ConvertMetricNameToMetricEvalRequest_Params_STRUCT_SIZE};
		convertMetricToEvalRequest.pMetricsEvaluator = metricEvaluator;
		convertMetricToEvalRequest.pMetricName = reqName.c_str();
		convertMetricToEvalRequest.pMetricEvalRequest = &metricEvalRequest;
		convertMetricToEvalRequest.metricEvalRequestStructSize = NVPW_MetricEvalRequest_STRUCT_SIZE;
		RETURN_IF_NVPW_ERROR(false, NVPW_MetricsEvaluator_ConvertMetricNameToMetricEvalRequest(&convertMetricToEvalRequest));

		for (size_t rangeIndex = 0; rangeIndex < getNumRangesParams.numRanges; ++rangeIndex)
		{
			NVPW_Profiler_CounterData_GetRangeDescriptions_Params getRangeDescParams = {NVPW_Profiler_CounterData_GetRangeDescriptions_Params_STRUCT_SIZE};
			getRangeDescParams.pCounterDataImage = counterDataImage.data();
			getRangeDescParams.rangeIndex = rangeIndex;
			RETURN_IF_NVPW_ERROR(false, NVPW_Profiler_CounterData_GetRangeDescriptions(&getRangeDescParams));
			std::vector<const char *> descriptionPtrs(getRangeDescParams.numDescriptions);
			getRangeDescParams.ppDescriptions = descriptionPtrs.data();
			RETURN_IF_NVPW_ERROR(false, NVPW_Profiler_CounterData_GetRangeDescriptions(&getRangeDescParams));

			std::string rangeName;
			for (size_t descriptionIndex = 0; descriptionIndex < getRangeDescParams.numDescriptions; ++descriptionIndex)
			{
				if (descriptionIndex)
				{
					rangeName += "/";
				}
				rangeName += descriptionPtrs[descriptionIndex];
			}

			NVPW_MetricsEvaluator_SetDeviceAttributes_Params setDeviceAttribParams = {NVPW_MetricsEvaluator_SetDeviceAttributes_Params_STRUCT_SIZE};
			setDeviceAttribParams.pMetricsEvaluator = metricEvaluator;
			setDeviceAttribParams.pCounterDataImage = counterDataImage.data();
			setDeviceAttribParams.counterDataImageSize = counterDataImage.size();
			RETURN_IF_NVPW_ERROR(false, NVPW_MetricsEvaluator_SetDeviceAttributes(&setDeviceAttribParams));

			double metricValue;
			NVPW_MetricsEvaluator_EvaluateToGpuValues_Params evaluateToGpuValuesParams = {NVPW_MetricsEvaluator_EvaluateToGpuValues_Params_STRUCT_SIZE};
			evaluateToGpuValuesParams.pMetricsEvaluator = metricEvaluator;
			evaluateToGpuValuesParams.pMetricEvalRequests = &metricEvalRequest;
			evaluateToGpuValuesParams.numMetricEvalRequests = 1;
			evaluateToGpuValuesParams.metricEvalRequestStructSize = NVPW_MetricEvalRequest_STRUCT_SIZE;
			evaluateToGpuValuesParams.metricEvalRequestStrideSize = sizeof(NVPW_MetricEvalRequest);
			evaluateToGpuValuesParams.pCounterDataImage = counterDataImage.data();
			evaluateToGpuValuesParams.counterDataImageSize = counterDataImage.size();
			evaluateToGpuValuesParams.rangeIndex = rangeIndex;
			evaluateToGpuValuesParams.isolated = true;
			evaluateToGpuValuesParams.pMetricValues = &metricValue;
			RETURN_IF_NVPW_ERROR(false, NVPW_MetricsEvaluator_EvaluateToGpuValues(&evaluateToGpuValuesParams));

			metricValues[metricName] = metricValue;
		}
	}

	NVPW_MetricsEvaluator_Destroy_Params metricEvaluatorDestroyParams = {NVPW_MetricsEvaluator_Destroy_Params_STRUCT_SIZE};
	metricEvaluatorDestroyParams.pMetricsEvaluator = metricEvaluator;
	RETURN_IF_NVPW_ERROR(false, NVPW_MetricsEvaluator_Destroy(&metricEvaluatorDestroyParams));
	return true;
}

std::map<std::string, float> gpu_hpc_profiling(py::function callback, const std::vector<std::string> &metricNames)
{
	CUdevice cuDevice;
	std::map<std::string, float> metricValues;
	std::vector<uint8_t> counterDataImagePrefix;
	std::vector<uint8_t> configImage;
	std::vector<uint8_t> counterDataImage;
	std::vector<uint8_t> counterDataScratchBuffer;
	std::vector<uint8_t> counterAvailabilityImage;
	CUpti_ProfilerReplayMode profilerReplayMode = CUPTI_UserReplay;
	CUpti_ProfilerRange profilerRange = CUPTI_UserRange;
	int deviceCount, deviceNum = 0;
	int computeCapabilityMajor = 0, computeCapabilityMinor = 0;

	DRIVER_API_CALL(cuInit(0));
	DRIVER_API_CALL(cuDeviceGetCount(&deviceCount));

	if (deviceCount == 0)
	{
		std::cout << "There is no device supporting CUDA." << std::endl;
		exit(-1);
	}
	std::cout << "CUDA Device Number: " << deviceNum << std::endl;

	DRIVER_API_CALL(cuDeviceGet(&cuDevice, deviceNum));
	DRIVER_API_CALL(cuDeviceGetAttribute(&computeCapabilityMajor, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, cuDevice));
	DRIVER_API_CALL(cuDeviceGetAttribute(&computeCapabilityMinor, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, cuDevice));

	std::cout << "Compute Capability of Device: " << computeCapabilityMajor << "." << computeCapabilityMinor << std::endl;

	if (computeCapabilityMajor < 7)
	{
		std::cout << "Profiling API is unsupported on Device with compute capability < 7.0" << std::endl;
		exit(-1);
	}

	CUcontext cuContext;
	DRIVER_API_CALL(cuCtxCreate(&cuContext, 0, cuDevice));

	CUpti_Profiler_Initialize_Params profilerInitializeParams = {CUpti_Profiler_Initialize_Params_STRUCT_SIZE};
	CUPTI_API_CALL(cuptiProfilerInitialize(&profilerInitializeParams));
	/* Get chip name for the cuda  device */
	CUpti_Device_GetChipName_Params getChipNameParams = {CUpti_Device_GetChipName_Params_STRUCT_SIZE};
	getChipNameParams.deviceIndex = deviceNum;
	CUPTI_API_CALL(cuptiDeviceGetChipName(&getChipNameParams));
	std::string chipName(getChipNameParams.pChipName);

	CUpti_Profiler_GetCounterAvailability_Params getCounterAvailabilityParams = {CUpti_Profiler_GetCounterAvailability_Params_STRUCT_SIZE};
	getCounterAvailabilityParams.ctx = cuContext;
	CUPTI_API_CALL(cuptiProfilerGetCounterAvailability(&getCounterAvailabilityParams));

	counterAvailabilityImage.clear();
	counterAvailabilityImage.resize(getCounterAvailabilityParams.counterAvailabilityImageSize);
	getCounterAvailabilityParams.pCounterAvailabilityImage = counterAvailabilityImage.data();
	CUPTI_API_CALL(cuptiProfilerGetCounterAvailability(&getCounterAvailabilityParams));

	/* Generate configuration for metrics, this can also be done offline*/
	NVPW_InitializeHost_Params initializeHostParams = {NVPW_InitializeHost_Params_STRUCT_SIZE};
	NVPW_API_CALL(NVPW_InitializeHost(&initializeHostParams));

	if (metricNames.size())
	{
		if (!NV::Metric::Config::GetConfigImage(chipName, metricNames, configImage, counterAvailabilityImage.data()))
		{
			std::cout << "Failed to create configImage" << std::endl;
			exit(-1);
		}
		if (!NV::Metric::Config::GetCounterDataPrefixImage(chipName, metricNames, counterDataImagePrefix))
		{
			std::cout << "Failed to create counterDataImagePrefix" << std::endl;
			exit(-1);
		}
	}
	else
	{
		std::cout << "No metrics provided to profile" << std::endl;
		exit(-1);
	}

	if (!CreateCounterDataImage(counterDataImage, counterDataScratchBuffer, counterDataImagePrefix))
	{
		std::cout << "Failed to create counterDataImage" << std::endl;
		exit(-1);
	}

	if (!runTest(callback, configImage, counterDataScratchBuffer, counterDataImage, profilerReplayMode, profilerRange))
	{
		std::cout << "Failed to run sample" << std::endl;
		exit(-1);
	}
	CUpti_Profiler_DeInitialize_Params profilerDeInitializeParams = {CUpti_Profiler_DeInitialize_Params_STRUCT_SIZE};
	CUPTI_API_CALL(cuptiProfilerDeInitialize(&profilerDeInitializeParams));

	DRIVER_API_CALL(cuCtxDestroy(cuContext));

	/* Evaluation of metrics collected in counterDataImage, this can also be done offline*/

	if (!ParseMetricValues(metricValues, chipName, counterDataImage, metricNames, counterAvailabilityImage.data()))
	{
		std::cout << "Failed to parse metric values" << std::endl;
		exit(-1);
	}

	return metricValues;
}
