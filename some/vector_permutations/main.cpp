#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

template<class Hash = std::hash<std::string>>
struct is_permutation_hasher
{
	auto operator()(const std::string& item) const
	{
		auto sorted{item};
		std::sort(sorted.begin(), sorted.end());
		auto a = Hash { }(sorted);
		return a;
	}
};

template<class Equal = std::equal_to<std::string>>
struct is_permutation_equal_to
{
	auto operator()(const std::string& x, const std::string& y) const
	{
		auto sorted_x {x};
		auto sorted_y {y};
		std::sort(sorted_x.begin(), sorted_x.end());
		std::sort(sorted_y.begin(), sorted_y.end());
		auto b = Equal { }(sorted_x, sorted_y);
		return b;
	}
};

using hashtable = std::unordered_map<std::string, std::vector<std::string>, is_permutation_hasher<>, is_permutation_equal_to<>>;
hashtable do_it(std::vector<std::string> const& input)
{
	using vec_size = std::vector<std::string>::size_type;

	hashtable result;

	for (vec_size i = 0; i < input.size(); ++i)
	{
		result[input[i]].push_back(input[i]);
	}

	return result;
}

std::ostream& operator <<(std::ostream& out, std::vector<std::string> const& vec)
{
	for (const auto & v : vec)
	{
		out << v << ", ";
	}
	return out;
}

int main()
{
	std::vector<std::string> arr = {"rost", "kot", "kto",  "tok", "tors", "adf", "kots", "tokk"};

	auto output = do_it(arr);

	for (auto & v : output)
	{
		std::cout << v.second << std::endl;
	}
}