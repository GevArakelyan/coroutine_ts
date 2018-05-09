#include <experimental/resumable>
#include <iostream>
#include <experimental/coroutine>
#include <future>
#include <experimental/generator>

using namespace std;
using namespace experimental;


struct resumable_thing
{
	struct promise_type
	{
		int value;

		resumable_thing get_return_object()
		{
			return resumable_thing(
				coroutine_handle<promise_type>::from_promise(*this));
		}

		auto initial_suspend() { return suspend_never{}; }
		auto final_suspend() { return suspend_always{}; }
		// void return_void(){}
		template<class T>
		void return_value(T&& val) { value = std::forward<T>(val); }
	};

	int get() const { return _coroutine.promise().value; }

	explicit resumable_thing(coroutine_handle<promise_type> coro)
		: _coroutine(coro)
	{
	}

	resumable_thing() = default;
	resumable_thing(resumable_thing const&) = delete;
	resumable_thing& operator=(resumable_thing const&) = delete;

	resumable_thing(resumable_thing&& other) noexcept
		: _coroutine(other._coroutine)
	{
		other._coroutine = nullptr;
	}

	resumable_thing& operator=(resumable_thing&& other) noexcept
	{
		if (std::addressof(other) != this)
		{
			_coroutine = other._coroutine;
			other._coroutine = nullptr;
		}
		return *this;
	}

	~resumable_thing()
	{
		if (_coroutine) { _coroutine.destroy(); }
	}

	void resume() const
	{
		_coroutine.resume();
	}


	coroutine_handle<promise_type> _coroutine = nullptr;
};

resumable_thing get_value()
{
	cout << "get value called\n" << endl;
	co_await suspend_always{};
	cout << "get value resumed\n" << endl;
	co_return 0;
	std::cout << "after coreturn" << endl;
}

future<int> compute_value()
{
	int result = co_await async([]
	{
		return 20;
	});
	co_return result;
}
//////////////////////////////////////////////////////////////////////

// int main(int argc, char* argv[])
// {
// 	auto value = get_value();
//
// 	value.resume();
//
// 	cout << "main : value was " << value.get() << endl;
//
// 	promise<int> p;
//
// 	return 0;
// }
//////////////////////////////////////////////////////////////////////

template <typename T,typename... Args>
struct coroutine_traits<future<T>,Args...>
{
	struct promise_type
	{
		promise<T> _promise;
		future<T> get_return_object() { _promise.get_future(); }
		auto initial_susprend() { return suspend_never{}; }
		auto final_susprend() { return suspend_never{}; }
		template<typename U>
		void return_value(U&& value) { _promise.set_value(forward<U>(value)); }
		void set_exception(std::exception_ptr ex) { _promise.set_exception(std::move(ex)); }


	};
};
// template<typename T>
// struct future_awaiter
// {
// 	future<T>& _f;
//
// 	bool await_ready() { return _f._Is_ready(); }
// 	void await_suspend(coroutine_handle<> ch)
// 	{
// 		_f.then([ch]() { ch.resume(); });
// 	}
// 	auto await_resume() { return _f.get(); }
//
// };
// template <typename T>
// future_awaiter<T> operator co_await(future<T>& value)
// {
// 	return future_awaiter<T>{value};
// }

generator<int> (int first,int last)
{
	for(int i = first;i <=last;++i)
	{
		co_yield i;
	}
}
