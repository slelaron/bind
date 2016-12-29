#include <iostream>

template <size_t N>
struct place_holder
{};

place_holder<1> _1;
place_holder<2> _2;
place_holder<3> _3;
place_holder<4> _4;
place_holder<5> _5;
place_holder<6> _6;

struct null_pointer
{};

template <typename F, typename... Args>
struct bind_function;

template <size_t k, typename U>
struct kth_proj
{
	typedef typename kth_proj <k - 1, typename U::next_list>::ret_value ret_value;

	ret_value&& operator()(U lst)
	{
		return std::forward <ret_value> (kth_proj <k - 1, typename U::next_list> ()(lst.nxt));
	}
};

template <typename U>
struct kth_proj <1, U>
{
	typedef typename U::ret_value ret_value;
	
	ret_value&& operator() (U lst)
	{
		return std::forward <ret_value> (lst.val);
	}
};

template <typename F, typename T, typename... Args>
struct data
{
	data <F, Args...> nxt;
	T& val;
	data(F func, T&& _val, Args&&... args):
		nxt(data <F, Args...> (func, std::forward <Args> (args)...)),
		val(_val)
	{}

	template <typename U, typename... New>
	auto operator()(U lst, New&&... args)
	{
		return nxt(lst, std::forward <New> (args)..., val);
	}
};

template <typename F, size_t N, typename... Args>
struct data <F, place_holder <N>&, Args...>
{
	data <F, Args...> nxt;
	data(F func, place_holder <N>& ph, Args&&... args):
		nxt(data <F, Args...> (func, std::forward <Args> (args)...))
	{}

	template <typename U, typename... New>
	auto operator()(U lst, New&&... args)
	{
		return nxt(lst, std::forward <New> (args)..., std::forward <typename kth_proj <N, U>::ret_value> (kth_proj <N, U> ()(lst)));
	}
};

template <typename F, typename F1, typename... Args1, typename... Args>
struct data <F, bind_function <F1, Args1...>, Args...>
{
	data <F, Args...> nxt;
	bind_function <F1, Args1...> val;
	data(F func, bind_function <F1, Args1...> _val, Args&&... args):
		nxt(data <F, Args...> (func, std::forward <Args> (args)...)),
		val(_val)
	{}

	template <typename U, typename... New>
	auto operator()(U lst, New&&... args)
	{
		return nxt(lst, std::forward <New> (args)..., val.start_with_list(lst));
	}
};

template <typename F>
struct data <F, null_pointer>
{
	F func;
	data(F _func, null_pointer):
		func(_func)
	{}

	template <typename U, typename... New>
	auto operator()(U lst, New&&... args)
	{
		return func(std::forward <New> (args)...);
	}
};

template <typename... Args>
struct list
{};

template <typename T, typename... Args>
struct list <T, Args...>
{
	T& val;
	typedef T ret_value;
	typedef list <Args...> next_list;
	const list <Args...> nxt;
	list(T&& arg, Args&&... args):
		val(arg),
		nxt(list <Args...> (std::forward <Args> (args)...))
	{}
	//list (list <T, Args...> lst):
		//val(std::forward <T> (lst.val)),
		//nxt(std::forward <Args...> (lst.nxt))
	//{
	//}
};

template <typename T>
struct list <T>
{
	typedef T ret_value;
	T& val;
	list(T&& arg):
		val(arg)
	{}
};

template <typename F, typename... Args>
struct bind_function
{
	data <F, Args..., null_pointer> nxt;
	bind_function(F _func, Args&&... args):
		nxt(data <F, Args..., null_pointer> (_func, std::forward <Args> (args)..., null_pointer()))
	{}

	template <typename... New>
	auto operator()(New&&... args)
	{
		return nxt(list <New...> (std::forward <New> (args)...));
	}

	template <typename U>
	auto start_with_list(U lst)
	{
		return nxt(lst);
	}
};

template <typename F, typename... Args>
auto bind(F func, Args&&... args)
{
	return bind_function <F, Args...>(func, std::forward <Args> (args)...);
}

using namespace std;

int add(int a, int b, int c, int d)
{
	return a + b + c + d;
}

struct mull
{
	int operator()(int a, int b)
	{
		return a * b;
	}
};

int mul(int a, int b)
{
	return a * b;
}

void print()
{
	cout << "Hello!" << endl;
}

int main()
{
	cout << bind(add, bind(mull(), _1, _2), _2, _4, 4)(2, 2, 3, 6) << endl;
	bind(print)();
	return 0;
}
