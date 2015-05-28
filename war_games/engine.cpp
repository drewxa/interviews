#include "common.h"

typedef struct _SEGMENT{
	InfInt min;
	InfInt max;
	InfInt result;

	_SEGMENT()
	{
		max = 0L;
		min = 0L;
		result = 0L;
	}
	~_SEGMENT()
	{
		max = 0L;
		min = 0L;
		result = 0L;
	}
} SEGMENT, *PSEGMENT;


void FactSegment(SEGMENT& s)
{
	try
	{
		s.result = s.min;
		for (InfInt i = s.min + 1; i <= s.max; i++)
		{
			s.result *= i;
		}
	}
	catch (const std::exception & ex)
	{
		std::cout << ex.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "exception in " << std::this_thread::get_id() << std::endl;
	}
}

InfInt GetCnk(InfInt _N, InfInt _K, long long & ts)
{
	// fill k_segments
	SEGMENT k_segments;
	k_segments.min = 1;
	k_segments.max = _K;

	// fill n_segments
	SEGMENT n_segments;
	n_segments.min = _N - _K + 1;
	n_segments.max = _N;


	// run all threads
	auto time_step_1 = std::chrono::high_resolution_clock::now();

	// get C(N,k)
	FactSegment(n_segments);
	InfInt N_k_1 = n_segments.result;

	FactSegment(k_segments);
	InfInt k_1 = k_segments.result;

	InfInt c_N_k = N_k_1 / k_1;

	auto time_step_2 = std::chrono::high_resolution_clock::now();
	ts = std::chrono::duration_cast<std::chrono::milliseconds>(time_step_2 - time_step_1).count();

	return c_N_k;
}

//
//#define USE_AUTO_REDUCTION_NUMBER_OF_THREADS

InfInt GetCnk(InfInt _N, InfInt _K, int num_thread, long long & ts)
{
	// get trivial C(n,k)
	if (_K == 0 || _K == _N)
	{
		return InfInt(1);
	}
	if (_K == 1 || _K == (_N - 1))
	{
		return InfInt(0);
	}

	if (num_thread == 1)
		return GetCnk(_N, _K, ts);


#ifdef USE_AUTO_REDUCTION_NUMBER_OF_THREADS
too_many_threads :
#endif

	int num_k_thread = num_thread / 2;
	int num_n_thread = num_thread - num_k_thread;


	// fill k_segments
	PSEGMENT k_segments = new SEGMENT[num_k_thread];
	InfInt _a = _K / num_k_thread;
#	ifdef USE_AUTO_REDUCTION_NUMBER_OF_THREADS
	if (_a.toInt() == 1)
	{
		std::cout << "too many threads" << std::endl;
		num_thread /= 2;
		ReleasePSEGMENT(x)
			goto too_many_threads;
	}
#	endif
	for (int i = 0; i < num_k_thread; i++)
	{
		k_segments[i].min = _a * i + 1;
		k_segments[i].max = k_segments[i].min + _a - 1;
	}
	k_segments[num_k_thread - 1].max = _K;


	// fill n_segments
	PSEGMENT n_segments = new SEGMENT[num_n_thread];
	InfInt _R = _N - _K + 1;
	_a = (_K + 1) / (num_n_thread);
#	ifdef USE_AUTO_REDUCTION_NUMBER_OF_THREADS
	if (_a.toInt() == 1)
	{
		std::cout << "too many threads" << std::endl;
		num_thread /= 2;
		ReleasePSEGMENT(n_segments);
		ReleasePSEGMENT(k_segments);
		goto too_many_threads;
	}
#	endif
	for (int i = 0; i < num_n_thread; i++)
	{
		n_segments[i].min = _R + _a * i;
		n_segments[i].max = n_segments[i].min + _a - 1;
	}
	n_segments[num_n_thread - 1].max = _N;


	// run all threads
	auto time_step_1 = std::chrono::high_resolution_clock::now();

	std::thread * th = new std::thread[num_thread];
	try
	{
		for (int i = 0; i < num_n_thread; i++)
		{
			th[i] = std::thread(FactSegment, std::ref(n_segments[i]));
		}
		for (int i = 0; i < num_k_thread; i++)
		{
			th[num_n_thread + i] = std::thread(FactSegment, std::ref(k_segments[i]));
		}

		for (int i = 0; i < num_thread; i++)
		{
			th[i].join();
		}
	}
	catch (const std::exception & ex)
	{
		std::cout << ex.what() << std::endl;
	}


	// get C(N,k)
	InfInt N_k_1 = n_segments[0].result;
	for (int i = 1; i < num_n_thread; i++)
		N_k_1 *= n_segments[i].result;

	InfInt k_1 = k_segments[0].result;
	for (int i = 1; i < num_k_thread; i++)
		k_1 *= k_segments[i].result;

	InfInt c_N_k = N_k_1 / k_1;

	auto time_step_2 = std::chrono::high_resolution_clock::now();
	ts = std::chrono::duration_cast<std::chrono::milliseconds>(time_step_2 - time_step_1).count();

	ReleasePSEGMENT(n_segments);
	ReleasePSEGMENT(k_segments);
	if (th)
		delete[] th;

	return c_N_k;
}
