#include "common.h"

// function is implemented in engine.cpp
// Calculate C(N,k)
InfInt GetCnk(InfInt N, InfInt K, int num_threads, long long &ts);


CArgMap ParseCmdline(int argc, char *argv[])
{
	CArgMap arg;

	for (int i = 1; i < argc; i++)
	{
		String param(argv[i]);
		String::size_type fnd = param.find_first_of("--");
		if (fnd == 0)
		{
			if ((i + 1) < argc)
			{
				String value(argv[++i]);
				String::size_type fnd = value.find_first_of("--");
				if (fnd == 0)
				{
					i--;
					arg[param] = String();
				}
				else
				{
					arg[param] = value;
				}
			}
			else
			{
				arg[param] = String();
			}
		}
	}

	return arg;
}

void Test()
{
	long long ts = 0;
	InfInt C_400_200 = InfInt("102952500135414432972975880320401986757210925381077648234849059575923332372651958598336595518976492951564048597506774120");
	if (C_400_200 != GetCnk(400, 200, 5, ts))
	{
		std::cout << "test C_400_200 failed" << std::endl;
	}
	else
	{
		std::cout << "test C_400_200 passed" << std::endl;
	}

	InfInt C_100000_40 = InfInt("121609356046246957408837899802260200220082168509125116402971749253175713362720942255782332221800074026457663690200246147311866572202925055011043405310000");
	if (C_100000_40 != GetCnk(100000, 40, 32, ts))
	{
		std::cout << "test C_100000_40 failed" << std::endl;
	}
	else
	{
		std::cout << "test C_100000_40 passed" << std::endl;
	}

	InfInt C_500_3 = InfInt("20708500"); 
	if (C_500_3 != GetCnk(500, 3, 1, ts))
	{
		std::cout << "test C_500_3 failed" << std::endl;
	}
	else
	{
		std::cout << "test C_500_3 passed" << std::endl;
	}


	InfInt C_500_1 = InfInt(500);
	if (C_500_1 != GetCnk(500, 1, 1, ts))
	{
		std::cout << "test C_500_1 failed" << std::endl;
	}
	else
	{
		std::cout << "test C_500_1 passed" << std::endl;
	}

	InfInt C_500_499 = InfInt(1);
	if (C_500_499 != GetCnk(500, 499, 1, ts))
	{
		std::cout << "test C_500_499 failed" << std::endl;
	}
	else
	{
		std::cout << "test C_500_499 passed" << std::endl;
	}

	InfInt C_500_0 = InfInt(1);
	if (C_500_0 != GetCnk(500, 0, 1, ts))
	{
		std::cout << "test C_500_0 failed" << std::endl;
	}
	else
	{
		std::cout << "test C_500_0 passed" << std::endl;
	}

	InfInt C_500_500 = InfInt(1);
	if (C_500_500 != GetCnk(500, 500, 1, ts))
	{
		std::cout << "test C_500_500 failed" << std::endl;
	}
	else
	{
		std::cout << "test C_500_500 passed" << std::endl;
	}
}

int main(int argc, char *argv[])
{
	CArgMap arg = ParseCmdline(argc, argv);
	
	if (arg.find("--test") != arg.end())
	{
		Test();
		return 0;
	}

	if (arg.find("--n") == arg.end() || 
		arg.find("--k") == arg.end() ||
		arg.find("--num_thread") == arg.end())
	{
		std::cout << "to calc C(N,k) you should define N and k:" << std::endl;
		std::cout << "    * use --n to define N" << std::endl;
		std::cout << "    * use --k to define k" << std::endl;
		std::cout << "    * use --num_thread to define number of threads" << std::endl;
		std::cout << "    * use --test to run tests" << std::endl;
		return 0;
	}

	// check N and K input data
	InfInt _N = arg["--n"];
	InfInt _K = arg["--k"];
	if (_K > _N || _N < 0 || _K < 0)
	{
		std::cout << "Wrong parametrs: k , N" << std::endl;
		return 0;
	}
	std::cout << "N = " << _N << std::endl;
	std::cout << "k = " << _K << std::endl;

	// check num_thread input data
	int num_thread = InfInt(arg["--num_thread"]).toInt();
	num_thread = (num_thread < 1) ? 1 : num_thread;

	long long ts = 0;
	InfInt c_N_k = GetCnk(_N, _K, num_thread, ts);

	std::cout << "C(N,k) = " << c_N_k << std::endl << std::endl;
	std::cout << "calc C(N,k) tooks " << ts	<< " milliseconds" << std::endl;
	return 0;
}