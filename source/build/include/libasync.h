// Copyright (c) 2015 Amanieu d'Antras
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef ASYNCXX_H_
#define ASYNCXX_H_

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

// Export declaration to make symbols visible for dll/so
#ifdef LIBASYNC_STATIC
# define LIBASYNC_EXPORT
# define LIBASYNC_EXPORT_EXCEPTION
#else
# ifdef _WIN32
#  ifdef LIBASYNC_BUILD
#   define LIBASYNC_EXPORT __declspec(dllexport)
#  else
#   define LIBASYNC_EXPORT __declspec(dllimport)
#  endif
#  define LIBASYNC_EXPORT_EXCEPTION
# else
#  define LIBASYNC_EXPORT __attribute__((visibility("default")))
#  define LIBASYNC_EXPORT_EXCEPTION __attribute__((visibility("default")))
# endif
#endif

// Support compiling without exceptions
#ifndef LIBASYNC_NO_EXCEPTIONS
# ifdef __clang__
#  if !defined(__EXCEPTIONS) || !__has_feature(cxx_exceptions)
#   define LIBASYNC_NO_EXCEPTIONS
#  endif
# elif defined(__GNUC__) && !defined(__EXCEPTIONS)
#  define LIBASYNC_NO_EXCEPTIONS
# elif defined(_MSC_VER) && defined(_HAS_EXCEPTIONS) && !_HAS_EXCEPTIONS
#  define LIBASYNC_NO_EXCEPTIONS
# endif
#endif
#ifdef LIBASYNC_NO_EXCEPTIONS
# define LIBASYNC_THROW(...) std::abort()
# define LIBASYNC_RETHROW() do {} while (false)
# define LIBASYNC_RETHROW_EXCEPTION(except) std::terminate()
# define LIBASYNC_TRY if (true)
# define LIBASYNC_CATCH(...) else if (false)
#else
# define LIBASYNC_THROW(...) throw __VA_ARGS__
# define LIBASYNC_RETHROW() throw
# define LIBASYNC_RETHROW_EXCEPTION(except) std::rethrow_exception(except)
# define LIBASYNC_TRY try
# define LIBASYNC_CATCH(...) catch (__VA_ARGS__)
#endif

// Optional debug assertions. If exceptions are enabled then use those, but
// otherwise fall back to an assert message.
#ifndef NDEBUG
# ifndef LIBASYNC_NO_EXCEPTIONS
#  define LIBASYNC_ASSERT(pred, except, message) ((pred) ? ((void)0) : throw except(message))
# else
#  define LIBASYNC_ASSERT(pred, except, message) ((pred) ? ((void)0) : assert(message))
# endif
#else
# define LIBASYNC_ASSERT(pred, except, message) ((void)0)
#endif

// Annotate move constructors and move assignment with noexcept to allow objects
// to be moved if they are in containers. Compilers which don't support noexcept
// will usually move regardless.
#if defined(__GNUC__) || _MSC_VER >= 1900
# define LIBASYNC_NOEXCEPT noexcept
#else
# define LIBASYNC_NOEXCEPT throw()
#endif

// Cacheline alignment to avoid false sharing between different threads
#define LIBASYNC_CACHELINE_SIZE 64
#ifdef __GNUC__
# define LIBASYNC_CACHELINE_ALIGN __attribute__((aligned(LIBASYNC_CACHELINE_SIZE)))
#elif defined(_MSC_VER)
# define LIBASYNC_CACHELINE_ALIGN __declspec(align(LIBASYNC_CACHELINE_SIZE))
#else
# define LIBASYNC_CACHELINE_ALIGN alignas(LIBASYNC_CACHELINE_SIZE)
#endif

// Force symbol visibility to hidden unless explicity exported
#ifndef LIBASYNC_STATIC
#if defined(__GNUC__) && !defined(_WIN32)
# pragma GCC visibility push(hidden)
#endif
#endif

// Some forward declarations
namespace async {

template<typename Result>
class task;
template<typename Result>
class shared_task;
template<typename Result>
class event_task;

} // namespace async

// Include sub-headers

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Pseudo-void type: it takes up no space but can be moved and copied
struct fake_void {};
template<typename T>
struct void_to_fake_void {
	typedef T type;
};
template<>
struct void_to_fake_void<void> {
	typedef fake_void type;
};
template<typename T>
T fake_void_to_void(T&& x)
{
	return std::forward<T>(x);
}
inline void fake_void_to_void(fake_void) {}

// Check if type is a task type, used to detect task unwraping
template<typename T>
struct is_task: public std::false_type {};
template<typename T>
struct is_task<task<T>>: public std::true_type {};
template<typename T>
struct is_task<const task<T>>: public std::true_type {};
template<typename T>
struct is_task<shared_task<T>>: public std::true_type {};
template<typename T>
struct is_task<const shared_task<T>>: public std::true_type {};

// Extract the result type of a task if T is a task, otherwise just return T
template<typename T>
struct remove_task {
	typedef T type;
};
template<typename T>
struct remove_task<task<T>> {
	typedef T type;
};
template<typename T>
struct remove_task<const task<T>> {
	typedef T type;
};
template<typename T>
struct remove_task<shared_task<T>> {
	typedef T type;
};
template<typename T>
struct remove_task<const shared_task<T>> {
	typedef T type;
};

// Check if a type is callable with the given arguments
typedef char one[1];
typedef char two[2];
template<typename Func, typename... Args, typename = decltype(std::declval<Func>()(std::declval<Args>()...))>
two& is_callable_helper(int);
template<typename Func, typename... Args>
one& is_callable_helper(...);
template<typename T>
struct is_callable;
template<typename Func, typename... Args>
struct is_callable<Func(Args...)>: public std::integral_constant<bool, sizeof(is_callable_helper<Func, Args...>(0)) - 1> {};

// Wrapper to run a function object with an optional parameter:
// - void returns are turned into fake_void
// - fake_void parameter will invoke the function with no arguments
template<typename Func, typename = typename std::enable_if<!std::is_void<decltype(std::declval<Func>()())>::value>::type>
decltype(std::declval<Func>()()) invoke_fake_void(Func&& f)
{
	return std::forward<Func>(f)();
}
template<typename Func, typename = typename std::enable_if<std::is_void<decltype(std::declval<Func>()())>::value>::type>
fake_void invoke_fake_void(Func&& f)
{
	std::forward<Func>(f)();
	return fake_void();
}
template<typename Func, typename Param>
typename void_to_fake_void<decltype(std::declval<Func>()(std::declval<Param>()))>::type invoke_fake_void(Func&& f, Param&& p)
{
	return detail::invoke_fake_void([&f, &p] {return std::forward<Func>(f)(std::forward<Param>(p));});
}
template<typename Func>
typename void_to_fake_void<decltype(std::declval<Func>()())>::type invoke_fake_void(Func&& f, fake_void)
{
	return detail::invoke_fake_void(std::forward<Func>(f));
}

// Various properties of a continuation function
template<typename Func, typename Parent, typename = decltype(std::declval<Func>()())>
fake_void is_value_cont_helper(const Parent&, int, int);
template<typename Func, typename Parent, typename = decltype(std::declval<Func>()(std::declval<Parent>().get()))>
std::true_type is_value_cont_helper(const Parent&, int, int);
template<typename Func, typename = decltype(std::declval<Func>()())>
std::true_type is_value_cont_helper(const task<void>&, int, int);
template<typename Func, typename = decltype(std::declval<Func>()())>
std::true_type is_value_cont_helper(const shared_task<void>&, int, int);
template<typename Func, typename Parent, typename = decltype(std::declval<Func>()(std::declval<Parent>()))>
std::false_type is_value_cont_helper(const Parent&, int, ...);
template<typename Func, typename Parent>
void is_value_cont_helper(const Parent&, ...);
template<typename Parent, typename Func>
struct continuation_traits {
	typedef typename std::decay<Func>::type decay_func;
	typedef decltype(detail::is_value_cont_helper<decay_func>(std::declval<Parent>(), 0, 0)) is_value_cont;
	static_assert(!std::is_void<is_value_cont>::value, "Parameter type for continuation function is invalid for parent task type");
	typedef typename std::conditional<std::is_same<is_value_cont, fake_void>::value, fake_void, typename std::conditional<std::is_same<is_value_cont, std::true_type>::value, typename void_to_fake_void<decltype(std::declval<Parent>().get())>::type, Parent>::type>::type param_type;
	typedef decltype(detail::fake_void_to_void(detail::invoke_fake_void(std::declval<decay_func>(), std::declval<param_type>()))) result_type;
	typedef task<typename remove_task<result_type>::type> task_type;
};

} // namespace detail
} // namespace async

namespace async {
namespace detail {

// Allocate an aligned block of memory
static inline void* aligned_alloc(std::size_t size, std::size_t align);

// Free an aligned block of memory
static inline void aligned_free(void* addr) LIBASYNC_NOEXCEPT;

// Class representing an aligned array and its length
template<typename T, std::size_t Align = std::alignment_of<T>::value>
class aligned_array {
	std::size_t length;
	T* ptr;

public:
	aligned_array()
		: length(0), ptr(nullptr) {}
	aligned_array(std::nullptr_t)
		: length(0), ptr(nullptr) {}
	explicit aligned_array(std::size_t length)
		: length(length)
	{
		ptr = static_cast<T*>(aligned_alloc(length * sizeof(T), Align));
		std::size_t i;
		LIBASYNC_TRY {
			for (i = 0; i < length; i++)
				new(ptr + i) T;
		} LIBASYNC_CATCH(...) {
			for (std::size_t j = 0; j < i; j++)
				ptr[i].~T();
			aligned_free(ptr);
			LIBASYNC_RETHROW();
		}
	}
	aligned_array(aligned_array&& other) LIBASYNC_NOEXCEPT
		: length(other.length), ptr(other.ptr)
	{
		other.ptr = nullptr;
		other.length = 0;
	}
	aligned_array& operator=(aligned_array&& other) LIBASYNC_NOEXCEPT
	{
		aligned_array(std::move(*this));
		std::swap(ptr, other.ptr);
		std::swap(length, other.length);
		return *this;
	}
	aligned_array& operator=(std::nullptr_t)
	{
		return *this = aligned_array();
	}
	~aligned_array()
	{
		for (std::size_t i = 0; i < length; i++)
			ptr[i].~T();
		aligned_free(ptr);
	}

	T& operator[](std::size_t i) const
	{
		return ptr[i];
	}
	std::size_t size() const
	{
		return length;
	}
	T* get() const
	{
		return ptr;
	}
	explicit operator bool() const
	{
		return ptr != nullptr;
	}
};

} // namespace detail
} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Default deleter which just uses the delete keyword
template<typename T>
struct default_deleter {
	static void do_delete(T* p)
	{
		delete p;
	}
};

// Reference-counted object base class
template<typename T, typename Deleter = default_deleter<T>>
struct ref_count_base {
	std::atomic<std::size_t> ref_count;

	// By default the reference count is initialized to 1
	explicit ref_count_base(std::size_t count = 1)
		: ref_count(count) {}

	void add_ref(std::size_t count = 1)
	{
		ref_count.fetch_add(count, std::memory_order_relaxed);
	}
	void remove_ref(std::size_t count = 1)
	{
		if (ref_count.fetch_sub(count, std::memory_order_release) == count) {
			std::atomic_thread_fence(std::memory_order_acquire);
			Deleter::do_delete(static_cast<T*>(this));
		}
	}
	void add_ref_unlocked()
	{
		ref_count.store(ref_count.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
	}
	bool is_unique_ref(std::memory_order order)
	{
		return ref_count.load(order) == 1;
	}
};

// Pointer to reference counted object, based on boost::intrusive_ptr
template<typename T>
class ref_count_ptr {
	T* p;

public:
	// Note that this doesn't increment the reference count, instead it takes
	// ownership of a pointer which you already own a reference to.
	explicit ref_count_ptr(T* t)
		: p(t) {}

	ref_count_ptr()
		: p(nullptr) {}
	ref_count_ptr(std::nullptr_t)
		: p(nullptr) {}
	ref_count_ptr(const ref_count_ptr& other) LIBASYNC_NOEXCEPT
		: p(other.p)
	{
		if (p)
			p->add_ref();
	}
	ref_count_ptr(ref_count_ptr&& other) LIBASYNC_NOEXCEPT
		: p(other.p)
	{
		other.p = nullptr;
	}
	ref_count_ptr& operator=(std::nullptr_t)
	{
		if (p)
			p->remove_ref();
		p = nullptr;
		return *this;
	}
	ref_count_ptr& operator=(const ref_count_ptr& other) LIBASYNC_NOEXCEPT
	{
		if (p) {
			p->remove_ref();
			p = nullptr;
		}
		p = other.p;
		if (p)
			p->add_ref();
		return *this;
	}
	ref_count_ptr& operator=(ref_count_ptr&& other) LIBASYNC_NOEXCEPT
	{
		if (p) {
			p->remove_ref();
			p = nullptr;
		}
		p = other.p;
		other.p = nullptr;
		return *this;
	}
	~ref_count_ptr()
	{
		if (p)
			p->remove_ref();
	}

	T& operator*() const
	{
		return *p;
	}
	T* operator->() const
	{
		return p;
	}
	T* get() const
	{
		return p;
	}
	T* release()
	{
		T* out = p;
		p = nullptr;
		return out;
	}

	explicit operator bool() const
	{
		return p != nullptr;
	}
	friend bool operator==(const ref_count_ptr& a, const ref_count_ptr& b)
	{
		return a.p == b.p;
	}
	friend bool operator!=(const ref_count_ptr& a, const ref_count_ptr& b)
	{
		return a.p != b.p;
	}
	friend bool operator==(const ref_count_ptr& a, std::nullptr_t)
	{
		return a.p == nullptr;
	}
	friend bool operator!=(const ref_count_ptr& a, std::nullptr_t)
	{
		return a.p != nullptr;
	}
	friend bool operator==(std::nullptr_t, const ref_count_ptr& a)
	{
		return a.p == nullptr;
	}
	friend bool operator!=(std::nullptr_t, const ref_count_ptr& a)
	{
		return a.p != nullptr;
	}
};

} // namespace detail
} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {

// Forward declarations
class task_run_handle;
class threadpool_scheduler;

// Scheduler interface:
// A scheduler is any type that implements this function:
// void schedule(async::task_run_handle t);
// This function should result in t.run() being called at some future point.

namespace detail {

// Detect whether an object is a scheduler
template<typename T, typename = decltype(std::declval<T>().schedule(std::declval<task_run_handle>()))>
two& is_scheduler_helper(int);
template<typename T>
one& is_scheduler_helper(...);
template<typename T>
struct is_scheduler: public std::integral_constant<bool, sizeof(is_scheduler_helper<T>(0)) - 1> {};

// Singleton scheduler classes
class thread_scheduler_impl {
public:
	LIBASYNC_EXPORT static void schedule(task_run_handle t);
};
class inline_scheduler_impl {
public:
	static void schedule(task_run_handle t);
};

// Reference counted pointer to task data
struct task_base;
typedef ref_count_ptr<task_base> task_ptr;

// Helper function to schedule a task using a scheduler
template<typename Sched>
void schedule_task(Sched& sched, task_ptr t);

// Wait for the given task to finish. This will call the wait handler currently
// active for this thread, which causes the thread to sleep by default.
LIBASYNC_EXPORT void wait_for_task(task_base* wait_task);

// Forward-declaration for data used by threadpool_scheduler
struct threadpool_data;

} // namespace detail

// Run a task in the current thread as soon as it is scheduled
inline detail::inline_scheduler_impl& inline_scheduler()
{
	static detail::inline_scheduler_impl instance;
	return instance;
}

// Run a task in a separate thread. Note that this scheduler does not wait for
// threads to finish at process exit. You must ensure that all threads finish
// before ending the process.
inline detail::thread_scheduler_impl& thread_scheduler()
{
	static detail::thread_scheduler_impl instance;
	return instance;
}

// Built-in thread pool scheduler with a size that is configurable from the
// LIBASYNC_NUM_THREADS environment variable. If that variable does not exist
// then the number of CPUs in the system is used instead.
LIBASYNC_EXPORT threadpool_scheduler& default_threadpool_scheduler();

// Default scheduler that is used when one isn't specified. This defaults to
// default_threadpool_scheduler(), but can be overriden by defining
// LIBASYNC_CUSTOM_DEFAULT_SCHEDULER before including async++.h. Keep in mind
// that in that case async::default_scheduler should be declared before
// including async++.h.
#ifndef LIBASYNC_CUSTOM_DEFAULT_SCHEDULER
inline threadpool_scheduler& default_scheduler()
{
	return default_threadpool_scheduler();
}
#endif

// Scheduler that holds a list of tasks which can then be explicitly executed
// by a thread. Both adding and running tasks are thread-safe operations.
class fifo_scheduler {
	struct internal_data;
	std::unique_ptr<internal_data> impl;

public:
	LIBASYNC_EXPORT fifo_scheduler();
	LIBASYNC_EXPORT ~fifo_scheduler();

	// Add a task to the queue
	LIBASYNC_EXPORT void schedule(task_run_handle t);

	// Try running one task from the queue. Returns false if the queue was empty.
	LIBASYNC_EXPORT bool try_run_one_task();

	// Run all tasks in the queue
	LIBASYNC_EXPORT void run_all_tasks();
};

// Scheduler that runs tasks in a work-stealing thread pool of the given size.
// Note that destroying the thread pool before all tasks have completed may
// result in some tasks not being executed.
class threadpool_scheduler {
	std::unique_ptr<detail::threadpool_data> impl;

public:
	LIBASYNC_EXPORT threadpool_scheduler(threadpool_scheduler&& other);

	// Create a thread pool with the given number of threads
	LIBASYNC_EXPORT threadpool_scheduler(std::size_t num_threads);

	// Create a thread pool with the given number of threads. Call `prerun`
    // function before execution loop and `postrun` after.
	LIBASYNC_EXPORT threadpool_scheduler(std::size_t num_threads,
                                         std::function<void()>&& prerun_,
                                         std::function<void()>&& postrun_);

	// Destroy the thread pool, tasks that haven't been started are dropped
	LIBASYNC_EXPORT ~threadpool_scheduler();

	// Schedule a task to be run in the thread pool
	LIBASYNC_EXPORT void schedule(task_run_handle t);
};

namespace detail {

// Work-around for Intel compiler handling decltype poorly in function returns
typedef std::remove_reference<decltype(::async::default_scheduler())>::type default_scheduler_type;

} // namespace detail
} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Compress the flags in the low bits of the pointer if the structures are
// suitably aligned. Fall back to a separate flags variable otherwise.
template<std::uintptr_t Mask, bool Enable>
class compressed_ptr {
	void* ptr;
	std::uintptr_t flags;

public:
	compressed_ptr() = default;
	compressed_ptr(void* ptr, std::uintptr_t flags)
		: ptr(ptr), flags(flags) {}

	template<typename T>
	T* get_ptr() const
	{
		return static_cast<T*>(ptr);
	}
	std::uintptr_t get_flags() const
	{
		return flags;
	}

	void set_ptr(void* p)
	{
		ptr = p;
	}
	void set_flags(std::uintptr_t f)
	{
		flags = f;
	}
};
template<std::uintptr_t Mask>
class compressed_ptr<Mask, true> {
	std::uintptr_t data;

public:
	compressed_ptr() = default;
	compressed_ptr(void* ptr, std::uintptr_t flags)
		: data(reinterpret_cast<std::uintptr_t>(ptr) | flags) {}

	template<typename T>
	T* get_ptr() const
	{
		return reinterpret_cast<T*>(data & ~Mask);
	}
	std::uintptr_t get_flags() const
	{
		return data & Mask;
	}

	void set_ptr(void* p)
	{
		data = reinterpret_cast<std::uintptr_t>(p) | (data & Mask);
	}
	void set_flags(std::uintptr_t f)
	{
		data = (data & ~Mask) | f;
	}
};

// Thread-safe vector of task_ptr which is optimized for the common case of
// only having a single continuation.
class continuation_vector {
	// Heap-allocated data for the slow path
	struct vector_data {
		std::vector<task_base*> vector;
		std::mutex lock;
	};

	// Flags to describe the state of the vector
	enum flags {
		// If set, no more changes are allowed to internal_data
		is_locked = 1,

		// If set, the pointer is a vector_data* instead of a task_base*. If
		// there are 0 or 1 elements in the vector, the task_base* form is used.
		is_vector = 2
	};
	static const std::uintptr_t flags_mask = 3;

	// Embed the two bits in the data if they are suitably aligned. We only
	// check the alignment of vector_data here because task_base isn't defined
	// yet. Since we align task_base to LIBASYNC_CACHELINE_SIZE just use that.
	typedef compressed_ptr<flags_mask, (LIBASYNC_CACHELINE_SIZE & flags_mask) == 0 &&
	                                   (std::alignment_of<vector_data>::value & flags_mask) == 0> internal_data;

	// All changes to the internal data are atomic
	std::atomic<internal_data> atomic_data;

public:
	// Start unlocked with zero elements in the fast path
	continuation_vector()
	{
		// Workaround for a bug in certain versions of clang with libc++
		// error: no viable conversion from 'async::detail::compressed_ptr<3, true>' to '_Atomic(async::detail::compressed_ptr<3, true>)'
		atomic_data.store(internal_data(nullptr, 0), std::memory_order_relaxed);
	}

	// Free any left over data
	~continuation_vector()
	{
		// Converting to task_ptr instead of using remove_ref because task_base
		// isn't defined yet at this point.
		internal_data data = atomic_data.load(std::memory_order_relaxed);
		if (data.get_flags() & flags::is_vector) {
			// No need to lock the mutex, we are the only thread at this point
			for (task_base* i: data.get_ptr<vector_data>()->vector)
				(task_ptr(i));
			delete data.get_ptr<vector_data>();
		} else {
			// If the data is locked then the inline pointer is already gone
			if (!(data.get_flags() & flags::is_locked))
				task_ptr tmp(data.get_ptr<task_base>());
		}
	}

	// Try adding an element to the vector. This fails and returns false if
	// the vector has been locked. In that case t is not modified.
	bool try_add(task_ptr&& t)
	{
		// Cache to avoid re-allocating vector_data multiple times. This is
		// automatically freed if it is not successfully saved to atomic_data.
		std::unique_ptr<vector_data> vector;

		// Compare-exchange loop on atomic_data
		internal_data data = atomic_data.load(std::memory_order_relaxed);
		internal_data new_data;
		do {
			// Return immediately if the vector is locked
			if (data.get_flags() & flags::is_locked)
				return false;

			if (data.get_flags() & flags::is_vector) {
				// Larger vectors use a mutex, so grab the lock
				std::atomic_thread_fence(std::memory_order_acquire);
				std::lock_guard<std::mutex> locked(data.get_ptr<vector_data>()->lock);

				// We need to check again if the vector has been locked here
				// to avoid a race condition with flush_and_lock
				if (atomic_data.load(std::memory_order_relaxed).get_flags() & flags::is_locked)
					return false;

				// Add the element to the vector and return
				data.get_ptr<vector_data>()->vector.push_back(t.release());
				return true;
			} else {
				if (data.get_ptr<task_base>()) {
					// Going from 1 to 2 elements, allocate a vector_data
					if (!vector)
						vector.reset(new vector_data{{data.get_ptr<task_base>(), t.get()}, {}});
					new_data = {vector.get(), flags::is_vector};
				} else {
					// Going from 0 to 1 elements
					new_data = {t.get(), 0};
				}
			}
		} while (!atomic_data.compare_exchange_weak(data, new_data, std::memory_order_release, std::memory_order_relaxed));

		// If we reach this point then atomic_data was successfully changed.
		// Since the pointers are now saved in the vector, release them from
		// the smart pointers.
		t.release();
		vector.release();
		return true;
	}

	// Lock the vector and flush all elements through the given function
	template<typename Func> void flush_and_lock(Func&& func)
	{
		// Try to lock the vector using a compare-exchange loop
		internal_data data = atomic_data.load(std::memory_order_relaxed);
		internal_data new_data;
		do {
			new_data = data;
			new_data.set_flags(data.get_flags() | flags::is_locked);
		} while (!atomic_data.compare_exchange_weak(data, new_data, std::memory_order_acquire, std::memory_order_relaxed));

		if (data.get_flags() & flags::is_vector) {
			// If we are using vector_data, lock it and flush all elements
			std::lock_guard<std::mutex> locked(data.get_ptr<vector_data>()->lock);
			for (auto i: data.get_ptr<vector_data>()->vector)
				func(task_ptr(i));

			// Clear the vector to save memory. Note that we don't actually free
			// the vector_data here because other threads may still be using it.
			// This isn't a very significant cost since multiple continuations
			// are relatively rare.
			data.get_ptr<vector_data>()->vector.clear();
		} else {
			// If there is an inline element, just pass it on
			if (data.get_ptr<task_base>())
				func(task_ptr(data.get_ptr<task_base>()));
		}
	}
};

} // namespace detail
} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Task states
enum class task_state: unsigned char {
	pending, // Task has not completed yet
	locked, // Task is locked (used by event_task to prevent double set)
	unwrapped, // Task is waiting for an unwrapped task to finish
	completed, // Task has finished execution and a result is available
	canceled // Task has been canceled and an exception is available
};

// Determine whether a task is in a final state
inline bool is_finished(task_state s)
{
	return s == task_state::completed || s == task_state::canceled;
}

// Virtual function table used to allow dynamic dispatch for task objects.
// While this is very similar to what a compiler would generate with virtual
// functions, this scheme was found to result in significantly smaller
// generated code size.
struct task_base_vtable {
	// Destroy the function and result
	void (*destroy)(task_base*) LIBASYNC_NOEXCEPT;

	// Run the associated function
	void (*run)(task_base*) LIBASYNC_NOEXCEPT;

	// Cancel the task with an exception
	void (*cancel)(task_base*, std::exception_ptr&&) LIBASYNC_NOEXCEPT;

	// Schedule the task using its scheduler
	void (*schedule)(task_base* parent, task_ptr t);
};

// Type-generic base task object
struct task_base_deleter;
struct LIBASYNC_CACHELINE_ALIGN task_base: public ref_count_base<task_base, task_base_deleter> {
	// Task state
	std::atomic<task_state> state;

	// Whether get_task() was already called on an event_task
	bool event_task_got_task;

	// Vector of continuations
	continuation_vector continuations;

	// Virtual function table used for dynamic dispatch
	const task_base_vtable* vtable;

	// Use aligned memory allocation
	static void* operator new(std::size_t size)
	{
		return aligned_alloc(size, LIBASYNC_CACHELINE_SIZE);
	}
	static void operator delete(void* ptr)
	{
		aligned_free(ptr);
	}

	// Initialize task state
	task_base()
		: state(task_state::pending) {}

	// Check whether the task is ready and include an acquire barrier if it is
	bool ready() const
	{
		return is_finished(state.load(std::memory_order_acquire));
	}

	// Run a single continuation
	template<typename Sched>
	void run_continuation(Sched& sched, task_ptr&& cont)
	{
		LIBASYNC_TRY {
			detail::schedule_task(sched, std::move(cont));
		} LIBASYNC_CATCH(...) {
			// This is suboptimal, but better than letting the exception leak
			cont->vtable->cancel(cont.get(), std::current_exception());
		}
	}

	// Run all of the task's continuations after it has completed or canceled.
	// The list of continuations is emptied and locked to prevent any further
	// continuations from being added.
	void run_continuations()
	{
		continuations.flush_and_lock([this](task_ptr t) {
			const task_base_vtable* vtable = t->vtable;
			vtable->schedule(this, std::move(t));
		});
	}

	// Add a continuation to this task
	template<typename Sched>
	void add_continuation(Sched& sched, task_ptr cont)
	{
		// Check for task completion
		task_state current_state = state.load(std::memory_order_relaxed);
		if (!is_finished(current_state)) {
			// Try to add the task to the continuation list. This can fail only
			// if the task has just finished, in which case we run it directly.
			if (continuations.try_add(std::move(cont)))
				return;
		}

		// Otherwise run the continuation directly
		std::atomic_thread_fence(std::memory_order_acquire);
		run_continuation(sched, std::move(cont));
	}

	// Finish the task after it has been executed and the result set
	void finish()
	{
		state.store(task_state::completed, std::memory_order_release);
		run_continuations();
	}

	// Wait for the task to finish executing
	task_state wait()
	{
		task_state s = state.load(std::memory_order_acquire);
		if (!is_finished(s)) {
			wait_for_task(this);
			s = state.load(std::memory_order_relaxed);
		}
		return s;
	}
};

// Deleter for task_ptr
struct task_base_deleter {
	static void do_delete(task_base* p)
	{
		// Go through the vtable to delete p with its proper type
		p->vtable->destroy(p);
	}
};

// Result type-specific task object
template<typename Result>
struct task_result_holder: public task_base {
	union {
		typename std::aligned_storage<sizeof(Result), std::alignment_of<Result>::value>::type result;
		std::aligned_storage<sizeof(std::exception_ptr), std::alignment_of<std::exception_ptr>::value>::type except;

		// Scheduler that should be used to schedule this task. The scheduler
		// type has been erased and is held by vtable->schedule.
		void* sched;
	};

	template<typename T>
	void set_result(T&& t)
	{
		new(&result) Result(std::forward<T>(t));
	}

	// Return a result using an lvalue or rvalue reference depending on the task
	// type. The task parameter is not used, it is just there for overload resolution.
	template<typename T>
	Result&& get_result(const task<T>&)
	{
		return std::move(*reinterpret_cast<Result*>(&result));
	}
	template<typename T>
	const Result& get_result(const shared_task<T>&)
	{
		return *reinterpret_cast<Result*>(&result);
	}

	// Destroy the result
	~task_result_holder()
	{
		// Result is only present if the task completed successfully
		if (state.load(std::memory_order_relaxed) == task_state::completed)
			reinterpret_cast<Result*>(&result)->~Result();
	}
};

// Specialization for references
template<typename Result>
struct task_result_holder<Result&>: public task_base {
	union {
		// Store as pointer internally
		Result* result;
		std::aligned_storage<sizeof(std::exception_ptr), std::alignment_of<std::exception_ptr>::value>::type except;
		void* sched;
	};

	void set_result(Result& obj)
	{
		result = std::addressof(obj);
	}

	template<typename T>
	Result& get_result(const task<T>&)
	{
		return *result;
	}
	template<typename T>
	Result& get_result(const shared_task<T>&)
	{
		return *result;
	}
};

// Specialization for void
template<>
struct task_result_holder<fake_void>: public task_base {
	union {
		std::aligned_storage<sizeof(std::exception_ptr), std::alignment_of<std::exception_ptr>::value>::type except;
		void* sched;
	};

	void set_result(fake_void) {}

	// Get the result as fake_void so that it can be passed to set_result and
	// continuations
	template<typename T>
	fake_void get_result(const task<T>&)
	{
		return fake_void();
	}
	template<typename T>
	fake_void get_result(const shared_task<T>&)
	{
		return fake_void();
	}
};

template<typename Result>
struct task_result: public task_result_holder<Result> {
	// Virtual function table for task_result
	static const task_base_vtable vtable_impl;
	task_result()
	{
		this->vtable = &vtable_impl;
	}

	// Destroy the exception
	~task_result()
	{
		// Exception is only present if the task was canceled
		if (this->state.load(std::memory_order_relaxed) == task_state::canceled)
			reinterpret_cast<std::exception_ptr*>(&this->except)->~exception_ptr();
	}

	// Cancel a task with the given exception
	void cancel_base(std::exception_ptr&& except)
	{
		set_exception(std::move(except));
		this->state.store(task_state::canceled, std::memory_order_release);
		this->run_continuations();
	}

	// Set the exception value of the task
	void set_exception(std::exception_ptr&& except)
	{
		new(&this->except) std::exception_ptr(std::move(except));
	}

	// Get the exception a task was canceled with
	std::exception_ptr& get_exception()
	{
		return *reinterpret_cast<std::exception_ptr*>(&this->except);
	}

	// Wait and throw the exception if the task was canceled
	void wait_and_throw()
	{
		if (this->wait() == task_state::canceled)
			LIBASYNC_RETHROW_EXCEPTION(get_exception());
	}

	// Delete the task using its proper type
	static void destroy(task_base* t) LIBASYNC_NOEXCEPT
	{
		delete static_cast<task_result<Result>*>(t);
	}
};
template<typename Result>
const task_base_vtable task_result<Result>::vtable_impl = {
	task_result<Result>::destroy, // destroy
	nullptr, // run
	nullptr, // cancel
	nullptr // schedule
};

// Class to hold a function object, with empty base class optimization
template<typename Func, typename = void>
struct func_base {
	Func func;

	template<typename F>
	explicit func_base(F&& f)
		: func(std::forward<F>(f)) {}
	Func& get_func()
	{
		return func;
	}
};
template<typename Func>
struct func_base<Func, typename std::enable_if<std::is_empty<Func>::value>::type> {
	template<typename F>
	explicit func_base(F&& f)
	{
		new(this) Func(std::forward<F>(f));
	}
	~func_base()
	{
		get_func().~Func();
	}
	Func& get_func()
	{
		return *reinterpret_cast<Func*>(this);
	}
};

// Class to hold a function object and initialize/destroy it at any time
template<typename Func, typename = void>
struct func_holder {
	typename std::aligned_storage<sizeof(Func), std::alignment_of<Func>::value>::type func;

	Func& get_func()
	{
		return *reinterpret_cast<Func*>(&func);
	}
	template<typename... Args>
	void init_func(Args&&... args)
	{
		new(&func) Func(std::forward<Args>(args)...);
	}
	void destroy_func()
	{
		get_func().~Func();
	}
};
template<typename Func>
struct func_holder<Func, typename std::enable_if<std::is_empty<Func>::value>::type> {
	Func& get_func()
	{
		return *reinterpret_cast<Func*>(this);
	}
	template<typename... Args>
	void init_func(Args&&... args)
	{
		new(this) Func(std::forward<Args>(args)...);
	}
	void destroy_func()
	{
		get_func().~Func();
	}
};

// Task object with an associated function object
// Using private inheritance so empty Func doesn't take up space
template<typename Sched, typename Func, typename Result>
struct task_func: public task_result<Result>, func_holder<Func> {
	// Virtual function table for task_func
	static const task_base_vtable vtable_impl;
	template<typename... Args>
	explicit task_func(Args&&... args)
	{
		this->vtable = &vtable_impl;
		this->init_func(std::forward<Args>(args)...);
	}

	// Run the stored function
	static void run(task_base* t) LIBASYNC_NOEXCEPT
	{
		LIBASYNC_TRY {
			// Dispatch to execution function
			static_cast<task_func<Sched, Func, Result>*>(t)->get_func()(t);
		} LIBASYNC_CATCH(...) {
			cancel(t, std::current_exception());
		}
	}

	// Cancel the task
	static void cancel(task_base* t, std::exception_ptr&& except) LIBASYNC_NOEXCEPT
	{
		// Destroy the function object when canceling since it won't be
		// used anymore.
		static_cast<task_func<Sched, Func, Result>*>(t)->destroy_func();
		static_cast<task_func<Sched, Func, Result>*>(t)->cancel_base(std::move(except));
	}

	// Schedule a continuation task using its scheduler
	static void schedule(task_base* parent, task_ptr t)
	{
		void* sched = static_cast<task_func<Sched, Func, Result>*>(t.get())->sched;
		parent->run_continuation(*static_cast<Sched*>(sched), std::move(t));
	}

	// Free the function
	~task_func()
	{
		// If the task hasn't completed yet, destroy the function object. Note
		// that an unwrapped task has already destroyed its function object.
		if (this->state.load(std::memory_order_relaxed) == task_state::pending)
			this->destroy_func();
	}

	// Delete the task using its proper type
	static void destroy(task_base* t) LIBASYNC_NOEXCEPT
	{
		delete static_cast<task_func<Sched, Func, Result>*>(t);
	}
};
template<typename Sched, typename Func, typename Result>
const task_base_vtable task_func<Sched, Func, Result>::vtable_impl = {
	task_func<Sched, Func, Result>::destroy, // destroy
	task_func<Sched, Func, Result>::run, // run
	task_func<Sched, Func, Result>::cancel, // cancel
	task_func<Sched, Func, Result>::schedule // schedule
};

// Helper functions to access the internal_task member of a task object, which
// avoids us having to specify half of the functions in the detail namespace
// as friend. Also, internal_task is downcast to the appropriate task_result<>.
template<typename Task>
typename Task::internal_task_type* get_internal_task(const Task& t)
{
	return static_cast<typename Task::internal_task_type*>(t.internal_task.get());
}
template<typename Task>
void set_internal_task(Task& t, task_ptr p)
{
	t.internal_task = std::move(p);
}

// Common code for task unwrapping
template<typename Result, typename Child>
struct unwrapped_func {
	explicit unwrapped_func(task_ptr t)
		: parent_task(std::move(t)) {}
	void operator()(Child child_task) const
	{
		// Forward completion state and result to parent task
		task_result<Result>* parent = static_cast<task_result<Result>*>(parent_task.get());
		LIBASYNC_TRY {
			if (get_internal_task(child_task)->state.load(std::memory_order_relaxed) == task_state::completed) {
				parent->set_result(get_internal_task(child_task)->get_result(child_task));
				parent->finish();
			} else {
				// We don't call the generic cancel function here because
				// the function of the parent task has already been destroyed.
				parent->cancel_base(std::exception_ptr(get_internal_task(child_task)->get_exception()));
			}
		} LIBASYNC_CATCH(...) {
			// If the copy/move constructor of the result threw, propagate the exception
			parent->cancel_base(std::current_exception());
		}
	}
	task_ptr parent_task;
};
template<typename Sched, typename Result, typename Func, typename Child>
void unwrapped_finish(task_base* parent_base, Child child_task)
{
	// Destroy the parent task's function since it has been executed
	parent_base->state.store(task_state::unwrapped, std::memory_order_relaxed);
	static_cast<task_func<Sched, Func, Result>*>(parent_base)->destroy_func();

	// Set up a continuation on the child to set the result of the parent
	LIBASYNC_TRY {
		parent_base->add_ref();
		child_task.then(inline_scheduler(), unwrapped_func<Result, Child>(task_ptr(parent_base)));
	} LIBASYNC_CATCH(...) {
		// Use cancel_base here because the function object is already destroyed.
		static_cast<task_result<Result>*>(parent_base)->cancel_base(std::current_exception());
	}
}

// Execution functions for root tasks:
// - With and without task unwraping
template<typename Sched, typename Result, typename Func, bool Unwrap>
struct root_exec_func: private func_base<Func> {
	template<typename F>
	explicit root_exec_func(F&& f)
		: func_base<Func>(std::forward<F>(f)) {}
	void operator()(task_base* t)
	{
		static_cast<task_result<Result>*>(t)->set_result(detail::invoke_fake_void(std::move(this->get_func())));
		static_cast<task_func<Sched, root_exec_func, Result>*>(t)->destroy_func();
		t->finish();
	}
};
template<typename Sched, typename Result, typename Func>
struct root_exec_func<Sched, Result, Func, true>: private func_base<Func> {
	template<typename F>
	explicit root_exec_func(F&& f)
		: func_base<Func>(std::forward<F>(f)) {}
	void operator()(task_base* t)
	{
		unwrapped_finish<Sched, Result, root_exec_func>(t, std::move(this->get_func())());
	}
};

// Execution functions for continuation tasks:
// - With and without task unwraping
// - For void, value-based and task-based continuations
template<typename Sched, typename Parent, typename Result, typename Func, typename ValueCont, bool Unwrap>
struct continuation_exec_func: private func_base<Func> {
	template<typename F, typename P>
	continuation_exec_func(F&& f, P&& p)
		: func_base<Func>(std::forward<F>(f)), parent(std::forward<P>(p)) {}
	void operator()(task_base* t)
	{
		static_cast<task_result<Result>*>(t)->set_result(detail::invoke_fake_void(std::move(this->get_func()), std::move(parent)));
		static_cast<task_func<Sched, continuation_exec_func, Result>*>(t)->destroy_func();
		t->finish();
	}
	Parent parent;
};
template<typename Sched, typename Parent, typename Result, typename Func>
struct continuation_exec_func<Sched, Parent, Result, Func, std::true_type, false>: private func_base<Func> {
	template<typename F, typename P>
	continuation_exec_func(F&& f, P&& p)
		: func_base<Func>(std::forward<F>(f)), parent(std::forward<P>(p)) {}
	void operator()(task_base* t)
	{
		if (get_internal_task(parent)->state.load(std::memory_order_relaxed) == task_state::canceled)
			task_func<Sched, continuation_exec_func, Result>::cancel(t, std::exception_ptr(get_internal_task(parent)->get_exception()));
		else {
			static_cast<task_result<Result>*>(t)->set_result(detail::invoke_fake_void(std::move(this->get_func()), get_internal_task(parent)->get_result(parent)));
			static_cast<task_func<Sched, continuation_exec_func, Result>*>(t)->destroy_func();
			t->finish();
		}
	}
	Parent parent;
};
template<typename Sched, typename Parent, typename Result, typename Func>
struct continuation_exec_func<Sched, Parent, Result, Func, fake_void, false>: private func_base<Func> {
	template<typename F, typename P>
	continuation_exec_func(F&& f, P&& p)
		: func_base<Func>(std::forward<F>(f)), parent(std::forward<P>(p)) {}
	void operator()(task_base* t)
	{
		if (get_internal_task(parent)->state.load(std::memory_order_relaxed) == task_state::canceled)
			task_func<Sched, continuation_exec_func, Result>::cancel(t, std::exception_ptr(get_internal_task(parent)->get_exception()));
		else {
			static_cast<task_result<Result>*>(t)->set_result(detail::invoke_fake_void(std::move(this->get_func()), fake_void()));
			static_cast<task_func<Sched, continuation_exec_func, Result>*>(t)->destroy_func();
			t->finish();
		}
	}
	Parent parent;
};
template<typename Sched, typename Parent, typename Result, typename Func>
struct continuation_exec_func<Sched, Parent, Result, Func, std::false_type, true>: private func_base<Func> {
	template<typename F, typename P>
	continuation_exec_func(F&& f, P&& p)
		: func_base<Func>(std::forward<F>(f)), parent(std::forward<P>(p)) {}
	void operator()(task_base* t)
	{
		unwrapped_finish<Sched, Result, continuation_exec_func>(t, detail::invoke_fake_void(std::move(this->get_func()), std::move(parent)));
	}
	Parent parent;
};
template<typename Sched, typename Parent, typename Result, typename Func>
struct continuation_exec_func<Sched, Parent, Result, Func, std::true_type, true>: private func_base<Func> {
	template<typename F, typename P>
	continuation_exec_func(F&& f, P&& p)
		: func_base<Func>(std::forward<F>(f)), parent(std::forward<P>(p)) {}
	void operator()(task_base* t)
	{
		if (get_internal_task(parent)->state.load(std::memory_order_relaxed) == task_state::canceled)
			task_func<Sched, continuation_exec_func, Result>::cancel(t, std::exception_ptr(get_internal_task(parent)->get_exception()));
		else
			unwrapped_finish<Sched, Result, continuation_exec_func>(t, detail::invoke_fake_void(std::move(this->get_func()), get_internal_task(parent)->get_result(parent)));
	}
	Parent parent;
};
template<typename Sched, typename Parent, typename Result, typename Func>
struct continuation_exec_func<Sched, Parent, Result, Func, fake_void, true>: private func_base<Func> {
	template<typename F, typename P>
	continuation_exec_func(F&& f, P&& p)
		: func_base<Func>(std::forward<F>(f)), parent(std::forward<P>(p)) {}
	void operator()(task_base* t)
	{
		if (get_internal_task(parent)->state.load(std::memory_order_relaxed) == task_state::canceled)
			task_func<Sched, continuation_exec_func, Result>::cancel(t, std::exception_ptr(get_internal_task(parent)->get_exception()));
		else
			unwrapped_finish<Sched, Result, continuation_exec_func>(t, detail::invoke_fake_void(std::move(this->get_func()), fake_void()));
	}
	Parent parent;
};

} // namespace detail
} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {

// Improved version of std::hardware_concurrency:
// - It never returns 0, 1 is returned instead.
// - It is guaranteed to remain constant for the duration of the program.
LIBASYNC_EXPORT std::size_t hardware_concurrency() LIBASYNC_NOEXCEPT;

// Task handle used by a wait handler
class task_wait_handle {
	detail::task_base* handle;

	// Allow construction in wait_for_task()
	friend LIBASYNC_EXPORT void detail::wait_for_task(detail::task_base* t);
	task_wait_handle(detail::task_base* t)
		: handle(t) {}

	// Execution function for use by wait handlers
	template<typename Func>
	struct wait_exec_func: private detail::func_base<Func> {
		template<typename F>
		explicit wait_exec_func(F&& f)
			: detail::func_base<Func>(std::forward<F>(f)) {}
		void operator()(detail::task_base*)
		{
			// Just call the function directly, all this wrapper does is remove
			// the task_base* parameter.
			this->get_func()();
		}
	};

public:
	task_wait_handle()
		: handle(nullptr) {}

	// Check if the handle is valid
	explicit operator bool() const
	{
		return handle != nullptr;
	}

	// Check if the task has finished executing
	bool ready() const
	{
		return detail::is_finished(handle->state.load(std::memory_order_acquire));
	}

	// Queue a function to be executed when the task has finished executing.
	template<typename Func>
	void on_finish(Func&& func)
	{
		// Make sure the function type is callable
		static_assert(detail::is_callable<Func()>::value, "Invalid function type passed to on_finish()");

		auto cont = new detail::task_func<typename std::remove_reference<decltype(inline_scheduler())>::type, wait_exec_func<typename std::decay<Func>::type>, detail::fake_void>(std::forward<Func>(func));
		cont->sched = std::addressof(inline_scheduler());
		handle->add_continuation(inline_scheduler(), detail::task_ptr(cont));
	}
};

// Wait handler function prototype
typedef void (*wait_handler)(task_wait_handle t);

// Set a wait handler to control what a task does when it has "free time", which
// is when it is waiting for another task to complete. The wait handler can do
// other work, but should return when it detects that the task has completed.
// The previously installed handler is returned.
LIBASYNC_EXPORT wait_handler set_thread_wait_handler(wait_handler w) LIBASYNC_NOEXCEPT;

// Exception thrown if a task_run_handle is destroyed without being run
struct LIBASYNC_EXPORT_EXCEPTION task_not_executed {};

// Task handle used in scheduler, acts as a unique_ptr to a task object
class task_run_handle {
	detail::task_ptr handle;

	// Allow construction in schedule_task()
	template<typename Sched>
	friend void detail::schedule_task(Sched& sched, detail::task_ptr t);
	explicit task_run_handle(detail::task_ptr t)
		: handle(std::move(t)) {}

public:
	// Movable but not copyable
	task_run_handle() = default;
	task_run_handle(task_run_handle&& other) LIBASYNC_NOEXCEPT
		: handle(std::move(other.handle)) {}
	task_run_handle& operator=(task_run_handle&& other) LIBASYNC_NOEXCEPT
	{
		handle = std::move(other.handle);
		return *this;
	}

	// If the task is not executed, cancel it with an exception
	~task_run_handle()
	{
		if (handle)
			handle->vtable->cancel(handle.get(), std::make_exception_ptr(task_not_executed()));
	}

	// Check if the handle is valid
	explicit operator bool() const
	{
		return handle != nullptr;
	}

	// Run the task and release the handle
	void run()
	{
		handle->vtable->run(handle.get());
		handle = nullptr;
	}

	// Run the task but run the given wait handler when waiting for a task,
	// instead of just sleeping.
	void run_with_wait_handler(wait_handler handler)
	{
		wait_handler old = set_thread_wait_handler(handler);
		run();
		set_thread_wait_handler(old);
	}

	// Conversion to and from void pointer. This allows the task handle to be
	// sent through C APIs which don't preserve types.
	void* to_void_ptr()
	{
		return handle.release();
	}
	static task_run_handle from_void_ptr(void* ptr)
	{
		return task_run_handle(detail::task_ptr(static_cast<detail::task_base*>(ptr)));
	}
};

namespace detail {

// Schedule a task for execution using its scheduler
template<typename Sched>
void schedule_task(Sched& sched, task_ptr t)
{
	static_assert(is_scheduler<Sched>::value, "Type is not a valid scheduler");
	sched.schedule(task_run_handle(std::move(t)));
}

// Inline scheduler implementation
inline void inline_scheduler_impl::schedule(task_run_handle t)
{
	t.run();
}

} // namespace detail
} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {

// Exception thrown when an event_task is destroyed without setting a value
struct LIBASYNC_EXPORT_EXCEPTION abandoned_event_task {};

namespace detail {

// Common code for task and shared_task
template<typename Result>
class basic_task {
	// Reference counted internal task object
	detail::task_ptr internal_task;

	// Real result type, with void turned into fake_void
	typedef typename void_to_fake_void<Result>::type internal_result;

	// Type-specific task object
	typedef task_result<internal_result> internal_task_type;

	// Friend access
	friend async::task<Result>;
	friend async::shared_task<Result>;
	template<typename T>
	friend typename T::internal_task_type* get_internal_task(const T& t);
	template<typename T>
	friend void set_internal_task(T& t, task_ptr p);

	// Common code for get()
	void get_internal() const
	{
		LIBASYNC_ASSERT(internal_task, std::invalid_argument, "Use of empty task object");

		// If the task was canceled, throw the associated exception
		get_internal_task(*this)->wait_and_throw();
	}

	// Common code for then()
	template<typename Sched, typename Func, typename Parent>
	typename continuation_traits<Parent, Func>::task_type then_internal(Sched& sched, Func&& f, Parent&& parent) const
	{
		LIBASYNC_ASSERT(internal_task, std::invalid_argument, "Use of empty task object");

		// Save a copy of internal_task because it might get moved into exec_func
		task_base* my_internal = internal_task.get();

		// Create continuation
		typedef continuation_traits<Parent, Func> traits;
		typedef typename void_to_fake_void<typename traits::task_type::result_type>::type cont_internal_result;
		typedef continuation_exec_func<Sched, typename std::decay<Parent>::type, cont_internal_result, typename traits::decay_func, typename traits::is_value_cont, is_task<typename traits::result_type>::value> exec_func;
		typename traits::task_type cont;
		set_internal_task(cont, task_ptr(new task_func<Sched, exec_func, cont_internal_result>(std::forward<Func>(f), std::forward<Parent>(parent))));

		// Add the continuation to this task
		// Avoid an expensive ref-count modification since the task isn't shared yet
		get_internal_task(cont)->add_ref_unlocked();
		get_internal_task(cont)->sched = std::addressof(sched);
		my_internal->add_continuation(sched, task_ptr(get_internal_task(cont)));

		return cont;
	}

public:
	// Task result type
	typedef Result result_type;

	// Check if this task is not empty
	bool valid() const
	{
		return internal_task != nullptr;
	}

	// Query whether the task has finished executing
	bool ready() const
	{
		LIBASYNC_ASSERT(internal_task, std::invalid_argument, "Use of empty task object");
		return internal_task->ready();
	}

	// Query whether the task has been canceled with an exception
	bool canceled() const
	{
		LIBASYNC_ASSERT(internal_task, std::invalid_argument, "Use of empty task object");
		return internal_task->state.load(std::memory_order_acquire) == task_state::canceled;
	}

	// Wait for the task to complete
	void wait() const
	{
		LIBASYNC_ASSERT(internal_task, std::invalid_argument, "Use of empty task object");
		internal_task->wait();
	}

	// Get the exception associated with a canceled task
	std::exception_ptr get_exception() const
	{
		LIBASYNC_ASSERT(internal_task, std::invalid_argument, "Use of empty task object");
		if (internal_task->wait() == task_state::canceled)
			return get_internal_task(*this)->get_exception();
		else
			return std::exception_ptr();
	}
};

// Common code for event_task specializations
template<typename Result>
class basic_event {
	// Reference counted internal task object
	detail::task_ptr internal_task;

	// Real result type, with void turned into fake_void
	typedef typename detail::void_to_fake_void<Result>::type internal_result;

	// Type-specific task object
	typedef detail::task_result<internal_result> internal_task_type;

	// Friend access
	friend async::event_task<Result>;
	template<typename T>
	friend typename T::internal_task_type* get_internal_task(const T& t);

	// Common code for set()
	template<typename T>
	bool set_internal(T&& result) const
	{
		LIBASYNC_ASSERT(internal_task, std::invalid_argument, "Use of empty event_task object");

		// Only allow setting the value once
		detail::task_state expected = detail::task_state::pending;
		if (!internal_task->state.compare_exchange_strong(expected, detail::task_state::locked, std::memory_order_acquire, std::memory_order_relaxed))
			return false;

		LIBASYNC_TRY {
			// Store the result and finish
			get_internal_task(*this)->set_result(std::forward<T>(result));
			internal_task->finish();
		} LIBASYNC_CATCH(...) {
			// At this point we have already committed to setting a value, so
			// we can't return the exception to the caller. If we did then it
			// could cause concurrent set() calls to fail, thinking a value has
			// already been set. Instead, we simply cancel the task with the
			// exception we just got.
			get_internal_task(*this)->cancel_base(std::current_exception());
		}
		return true;
	}

public:
	// Movable but not copyable
	basic_event(basic_event&& other) LIBASYNC_NOEXCEPT
		: internal_task(std::move(other.internal_task)) {}
	basic_event& operator=(basic_event&& other) LIBASYNC_NOEXCEPT
	{
		internal_task = std::move(other.internal_task);
		return *this;
	}

	// Main constructor
	basic_event()
		: internal_task(new internal_task_type)
	{
		internal_task->event_task_got_task = false;
	}

	// Cancel events if they are destroyed before they are set
	~basic_event()
	{
		// This check isn't thread-safe but set_exception does a proper check
		if (internal_task && !internal_task->ready() && !internal_task->is_unique_ref(std::memory_order_relaxed)) {
#ifdef LIBASYNC_NO_EXCEPTIONS
			// This will result in an abort if the task result is read
			set_exception(std::exception_ptr());
#else
			set_exception(std::make_exception_ptr(abandoned_event_task()));
#endif
		}
	}

	// Get the task linked to this event. This can only be called once.
	task<Result> get_task()
	{
		LIBASYNC_ASSERT(internal_task, std::invalid_argument, "Use of empty event_task object");
		LIBASYNC_ASSERT(!internal_task->event_task_got_task, std::logic_error, "get_task() called twice on event_task");

		// Even if we didn't trigger an assert, don't return a task if one has
		// already been returned.
		task<Result> out;
		if (!internal_task->event_task_got_task)
			set_internal_task(out, internal_task);
		internal_task->event_task_got_task = true;
		return out;
	}

	// Cancel the event with an exception and cancel continuations
	bool set_exception(std::exception_ptr except) const
	{
		LIBASYNC_ASSERT(internal_task, std::invalid_argument, "Use of empty event_task object");

		// Only allow setting the value once
		detail::task_state expected = detail::task_state::pending;
		if (!internal_task->state.compare_exchange_strong(expected, detail::task_state::locked, std::memory_order_acquire, std::memory_order_relaxed))
			return false;

		// Cancel the task
		get_internal_task(*this)->cancel_base(std::move(except));
		return true;
	}
};

} // namespace detail

template<typename Result>
class task: public detail::basic_task<Result> {
public:
	// Movable but not copyable
	task() = default;
	task(task&& other) LIBASYNC_NOEXCEPT
		: detail::basic_task<Result>(std::move(other)) {}
	task& operator=(task&& other) LIBASYNC_NOEXCEPT
	{
		detail::basic_task<Result>::operator=(std::move(other));
		return *this;
	}

	// Get the result of the task
	Result get()
	{
		this->get_internal();

		// Move the internal state pointer so that the task becomes invalid,
		// even if an exception is thrown.
		detail::task_ptr my_internal = std::move(this->internal_task);
		return detail::fake_void_to_void(static_cast<typename task::internal_task_type*>(my_internal.get())->get_result(*this));
	}

	// Add a continuation to the task
	template<typename Sched, typename Func>
	typename detail::continuation_traits<task, Func>::task_type then(Sched& sched, Func&& f)
	{
		return this->then_internal(sched, std::forward<Func>(f), std::move(*this));
	}
	template<typename Func>
	typename detail::continuation_traits<task, Func>::task_type then(Func&& f)
	{
		return then(::async::default_scheduler(), std::forward<Func>(f));
	}

	// Create a shared_task from this task
	shared_task<Result> share()
	{
		LIBASYNC_ASSERT(this->internal_task, std::invalid_argument, "Use of empty task object");

		shared_task<Result> out;
		detail::set_internal_task(out, std::move(this->internal_task));
		return out;
	}
};

template<typename Result>
class shared_task: public detail::basic_task<Result> {
	// get() return value: const Result& -or- void
	typedef typename std::conditional<
		std::is_void<Result>::value,
		void,
		typename std::add_lvalue_reference<
			typename std::add_const<Result>::type
		>::type
	>::type get_result;

public:
	// Movable and copyable
	shared_task() = default;

	// Get the result of the task
	get_result get() const
	{
		this->get_internal();
		return detail::fake_void_to_void(detail::get_internal_task(*this)->get_result(*this));
	}

	// Add a continuation to the task
	template<typename Sched, typename Func>
	typename detail::continuation_traits<shared_task, Func>::task_type then(Sched& sched, Func&& f) const
	{
		return this->then_internal(sched, std::forward<Func>(f), *this);
	}
	template<typename Func>
	typename detail::continuation_traits<shared_task, Func>::task_type then(Func&& f) const
	{
		return then(::async::default_scheduler(), std::forward<Func>(f));
	}
};

// Special task type which can be triggered manually rather than when a function executes.
template<typename Result>
class event_task: public detail::basic_event<Result> {
public:
	// Movable but not copyable
	event_task() = default;
	event_task(event_task&& other) LIBASYNC_NOEXCEPT
		: detail::basic_event<Result>(std::move(other)) {}
	event_task& operator=(event_task&& other) LIBASYNC_NOEXCEPT
	{
		detail::basic_event<Result>::operator=(std::move(other));
		return *this;
	}

	// Set the result of the task, mark it as completed and run its continuations
	bool set(const Result& result) const
	{
		return this->set_internal(result);
	}
	bool set(Result&& result) const
	{
		return this->set_internal(std::move(result));
	}
};

// Specialization for references
template<typename Result>
class event_task<Result&>: public detail::basic_event<Result&> {
public:
	// Movable but not copyable
	event_task() = default;
	event_task(event_task&& other) LIBASYNC_NOEXCEPT
		: detail::basic_event<Result&>(std::move(other)) {}
	event_task& operator=(event_task&& other) LIBASYNC_NOEXCEPT
	{
		detail::basic_event<Result&>::operator=(std::move(other));
		return *this;
	}

	// Set the result of the task, mark it as completed and run its continuations
	bool set(Result& result) const
	{
		return this->set_internal(result);
	}
};

// Specialization for void
template<>
class event_task<void>: public detail::basic_event<void> {
public:
	// Movable but not copyable
	event_task() = default;
	event_task(event_task&& other) LIBASYNC_NOEXCEPT
		: detail::basic_event<void>(std::move(other)) {}
	event_task& operator=(event_task&& other) LIBASYNC_NOEXCEPT
	{
		detail::basic_event<void>::operator=(std::move(other));
		return *this;
	}

	// Set the result of the task, mark it as completed and run its continuations
	bool set()
	{
		return this->set_internal(detail::fake_void());
	}
};

// Task type returned by local_spawn()
template<typename Sched, typename Func>
class local_task {
	// Make sure the function type is callable
	typedef typename std::decay<Func>::type decay_func;
	static_assert(detail::is_callable<decay_func()>::value, "Invalid function type passed to local_spawn()");

	// Task result type
	typedef typename detail::remove_task<decltype(std::declval<decay_func>()())>::type result_type;
	typedef typename detail::void_to_fake_void<result_type>::type internal_result;

	// Task execution function type
	typedef detail::root_exec_func<Sched, internal_result, decay_func, detail::is_task<decltype(std::declval<decay_func>()())>::value> exec_func;

	// Task object embedded directly. The ref-count is initialized to 1 so it
	// will never be freed using delete, only when the local_task is destroyed.
	detail::task_func<Sched, exec_func, internal_result> internal_task;

	// Friend access for local_spawn
	template<typename S, typename F>
	friend local_task<S, F> local_spawn(S& sched, F&& f);
	template<typename F>
	friend local_task<detail::default_scheduler_type, F> local_spawn(F&& f);

	// Constructor, used by local_spawn
	local_task(Sched& sched, Func&& f)
		: internal_task(std::forward<Func>(f))
	{
		// Avoid an expensive ref-count modification since the task isn't shared yet
		internal_task.add_ref_unlocked();
		detail::schedule_task(sched, detail::task_ptr(&internal_task));
	}

public:
	// Non-movable and non-copyable
	local_task(const local_task&) = delete;
	local_task& operator=(const local_task&) = delete;

	// Wait for the task to complete when destroying
	~local_task()
	{
		wait();

		// Now spin until the reference count drops to 1, since the scheduler
		// may still have a reference to the task.
		while (!internal_task.is_unique_ref(std::memory_order_acquire)) {
#if defined(__GLIBCXX__) && __GLIBCXX__ <= 20140612
			// Some versions of libstdc++ (4.7 and below) don't include a
			// definition of std::this_thread::yield().
			sched_yield();
#else
			std::this_thread::yield();
#endif
		}
	}

	// Query whether the task has finished executing
	bool ready() const
	{
		return internal_task.ready();
	}

	// Query whether the task has been canceled with an exception
	bool canceled() const
	{
		return internal_task.state.load(std::memory_order_acquire) == detail::task_state::canceled;
	}

	// Wait for the task to complete
	void wait()
	{
		internal_task.wait();
	}

	// Get the result of the task
	result_type get()
	{
		internal_task.wait_and_throw();
		return detail::fake_void_to_void(internal_task.get_result(task<result_type>()));
	}

	// Get the exception associated with a canceled task
	std::exception_ptr get_exception() const
	{
		if (internal_task.wait() == detail::task_state::canceled)
			return internal_task.get_exception();
		else
			return std::exception_ptr();
	}
};

// Spawn a function asynchronously
template<typename Sched, typename Func>
task<typename detail::remove_task<typename std::result_of<typename std::decay<Func>::type()>::type>::type> spawn(Sched& sched, Func&& f)
{
	// Using result_of in the function return type to work around bugs in the Intel
	// C++ compiler.

	// Make sure the function type is callable
	typedef typename std::decay<Func>::type decay_func;
	static_assert(detail::is_callable<decay_func()>::value, "Invalid function type passed to spawn()");

	// Create task
	typedef typename detail::void_to_fake_void<typename detail::remove_task<decltype(std::declval<decay_func>()())>::type>::type internal_result;
	typedef detail::root_exec_func<Sched, internal_result, decay_func, detail::is_task<decltype(std::declval<decay_func>()())>::value> exec_func;
	task<typename detail::remove_task<decltype(std::declval<decay_func>()())>::type> out;
	detail::set_internal_task(out, detail::task_ptr(new detail::task_func<Sched, exec_func, internal_result>(std::forward<Func>(f))));

	// Avoid an expensive ref-count modification since the task isn't shared yet
	detail::get_internal_task(out)->add_ref_unlocked();
	detail::schedule_task(sched, detail::task_ptr(detail::get_internal_task(out)));

	return out;
}
template<typename Func>
decltype(async::spawn(::async::default_scheduler(), std::declval<Func>())) spawn(Func&& f)
{
	return async::spawn(::async::default_scheduler(), std::forward<Func>(f));
}

// Create a completed task containing a value
template<typename T>
task<typename std::decay<T>::type> make_task(T&& value)
{
	task<typename std::decay<T>::type> out;

	detail::set_internal_task(out, detail::task_ptr(new detail::task_result<typename std::decay<T>::type>));
	detail::get_internal_task(out)->set_result(std::forward<T>(value));
	detail::get_internal_task(out)->state.store(detail::task_state::completed, std::memory_order_relaxed);

	return out;
}
template<typename T>
task<T&> make_task(std::reference_wrapper<T> value)
{
	task<T&> out;

	detail::set_internal_task(out, detail::task_ptr(new detail::task_result<T&>));
	detail::get_internal_task(out)->set_result(value.get());
	detail::get_internal_task(out)->state.store(detail::task_state::completed, std::memory_order_relaxed);

	return out;
}
inline task<void> make_task()
{
	task<void> out;

	detail::set_internal_task(out, detail::task_ptr(new detail::task_result<detail::fake_void>));
	detail::get_internal_task(out)->state.store(detail::task_state::completed, std::memory_order_relaxed);

	return out;
}

// Create a canceled task containing an exception
template<typename T>
task<T> make_exception_task(std::exception_ptr except)
{
	task<T> out;

	detail::set_internal_task(out, detail::task_ptr(new detail::task_result<typename detail::void_to_fake_void<T>::type>));
	detail::get_internal_task(out)->set_exception(std::move(except));
	detail::get_internal_task(out)->state.store(detail::task_state::canceled, std::memory_order_relaxed);

	return out;
}

// Spawn a very limited task which is restricted to the current function and
// joins on destruction. Because local_task is not movable, the result must
// be captured in a reference, like this:
// auto&& x = local_spawn(...);
template<typename Sched, typename Func>
#ifdef __GNUC__
__attribute__((warn_unused_result))
#endif
local_task<Sched, Func> local_spawn(Sched& sched, Func&& f)
{
	// Since local_task is not movable, we construct it in-place and let the
	// caller extend the lifetime of the returned object using a reference.
	return {sched, std::forward<Func>(f)};
}
template<typename Func>
#ifdef __GNUC__
__attribute__((warn_unused_result))
#endif
local_task<detail::default_scheduler_type, Func> local_spawn(Func&& f)
{
	return {::async::default_scheduler(), std::forward<Func>(f)};
}

} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {

// Result type for when_any
template<typename Result>
struct when_any_result {
	// Index of the task that finished first
	std::size_t index;

	// List of tasks that were passed in
	Result tasks;
};

namespace detail {

// Shared state for when_all
template<typename Result>
struct when_all_state: public ref_count_base<when_all_state<Result>> {
	event_task<Result> event;
	Result result;

	when_all_state(std::size_t count)
		: ref_count_base<when_all_state<Result>>(count) {}

	// When all references are dropped, signal the event
	~when_all_state()
	{
		event.set(std::move(result));
	}
};

// Execution functions for when_all, for ranges and tuples
template<typename Task, typename Result>
struct when_all_func_range {
	std::size_t index;
	ref_count_ptr<when_all_state<Result>> state;

	when_all_func_range(std::size_t index, ref_count_ptr<when_all_state<Result>> state)
		: index(index), state(std::move(state)) {}

	// Copy the completed task object to the shared state. The event is
	// automatically signaled when all references are dropped.
	void operator()(Task t) const
	{
		state->result[index] = std::move(t);
	}
};
template<std::size_t index, typename Task, typename Result>
struct when_all_func_tuple {
	ref_count_ptr<when_all_state<Result>> state;

	when_all_func_tuple(ref_count_ptr<when_all_state<Result>> state)
		: state(std::move(state)) {}

	// Copy the completed task object to the shared state. The event is
	// automatically signaled when all references are dropped.
	void operator()(Task t) const
	{
		std::get<index>(state->result) = std::move(t);
	}
};

// Shared state for when_any
template<typename Result>
struct when_any_state: public ref_count_base<when_any_state<Result>> {
	event_task<when_any_result<Result>> event;
	Result result;

	when_any_state(std::size_t count)
		: ref_count_base<when_any_state<Result>>(count) {}

	// Signal the event when the first task reaches here
	void set(std::size_t i)
	{
		event.set({i, std::move(result)});
	}
};

// Execution function for when_any
template<typename Task, typename Result>
struct when_any_func {
	std::size_t index;
	ref_count_ptr<when_any_state<Result>> state;

	when_any_func(std::size_t index, ref_count_ptr<when_any_state<Result>> state)
		: index(index), state(std::move(state)) {}

	// Simply tell the state that our task has finished, it already has a copy
	// of the task object.
	void operator()(Task) const
	{
		state->set(index);
	}
};

// Internal implementation of when_all for variadic arguments
template<std::size_t index, typename Result>
void when_all_variadic(when_all_state<Result>*) {}
template<std::size_t index, typename Result, typename First, typename... T>
void when_all_variadic(when_all_state<Result>* state, First&& first, T&&... tasks)
{
	typedef typename std::decay<First>::type task_type;

	// Add a continuation to the task
	LIBASYNC_TRY {
		first.then(inline_scheduler(), detail::when_all_func_tuple<index, task_type, Result>(detail::ref_count_ptr<detail::when_all_state<Result>>(state)));
	} LIBASYNC_CATCH(...) {
		// Make sure we don't leak memory if then() throws
		state->remove_ref(sizeof...(T));
		LIBASYNC_RETHROW();
	}

	// Add continuations to remaining tasks
	detail::when_all_variadic<index + 1>(state, std::forward<T>(tasks)...);
}

// Internal implementation of when_any for variadic arguments
template<std::size_t index, typename Result>
void when_any_variadic(when_any_state<Result>*) {}
template<std::size_t index, typename Result, typename First, typename... T>
void when_any_variadic(when_any_state<Result>* state, First&& first, T&&... tasks)
{
	typedef typename std::decay<First>::type task_type;

	// Add a copy of the task to the results because the event may be
	// set before all tasks have finished.
	detail::task_base* t = detail::get_internal_task(first);
	t->add_ref();
	detail::set_internal_task(std::get<index>(state->result), detail::task_ptr(t));

	// Add a continuation to the task
	LIBASYNC_TRY {
		first.then(inline_scheduler(), detail::when_any_func<task_type, Result>(index, detail::ref_count_ptr<detail::when_any_state<Result>>(state)));
	} LIBASYNC_CATCH(...) {
		// Make sure we don't leak memory if then() throws
		state->remove_ref(sizeof...(T));
		LIBASYNC_RETHROW();
	}

	// Add continuations to remaining tasks
	detail::when_any_variadic<index + 1>(state, std::forward<T>(tasks)...);
}

} // namespace detail

// Combine a set of tasks into one task which is signaled when all specified tasks finish
template<typename Iter>
task<std::vector<typename std::decay<typename std::iterator_traits<Iter>::value_type>::type>> when_all(Iter begin, Iter end)
{
	typedef typename std::decay<typename std::iterator_traits<Iter>::value_type>::type task_type;
	typedef std::vector<task_type> result_type;

	// Handle empty ranges
	if (begin == end)
		return make_task(result_type());

	// Create shared state, initialized with the proper reference count
	std::size_t count = std::distance(begin, end);
	auto* state = new detail::when_all_state<result_type>(count);
	state->result.resize(count);
	auto out = state->event.get_task();

	// Add a continuation to each task to add its result to the shared state
	// Last task sets the event result
	for (std::size_t i = 0; begin != end; i++, ++begin) {
		LIBASYNC_TRY {
			(*begin).then(inline_scheduler(), detail::when_all_func_range<task_type, result_type>(i, detail::ref_count_ptr<detail::when_all_state<result_type>>(state)));
		} LIBASYNC_CATCH(...) {
			// Make sure we don't leak memory if then() throws
			state->remove_ref(std::distance(begin, end) - 1);
			LIBASYNC_RETHROW();
		}
	}

	return out;
}

// Combine a set of tasks into one task which is signaled when one of the tasks finishes
template<typename Iter>
task<when_any_result<std::vector<typename std::decay<typename std::iterator_traits<Iter>::value_type>::type>>> when_any(Iter begin, Iter end)
{
	typedef typename std::decay<typename std::iterator_traits<Iter>::value_type>::type task_type;
	typedef std::vector<task_type> result_type;

	// Handle empty ranges
	if (begin == end)
		return make_task(when_any_result<result_type>());

	// Create shared state, initialized with the proper reference count
	std::size_t count = std::distance(begin, end);
	auto* state = new detail::when_any_state<result_type>(count);
	state->result.resize(count);
	auto out = state->event.get_task();

	// Add a continuation to each task to set the event. First one wins.
	for (std::size_t i = 0; begin != end; i++, ++begin) {
		// Add a copy of the task to the results because the event may be
		// set before all tasks have finished.
		detail::task_base* t = detail::get_internal_task(*begin);
		t->add_ref();
		detail::set_internal_task(state->result[i], detail::task_ptr(t));

		LIBASYNC_TRY {
			(*begin).then(inline_scheduler(), detail::when_any_func<task_type, result_type>(i, detail::ref_count_ptr<detail::when_any_state<result_type>>(state)));
		} LIBASYNC_CATCH(...) {
			// Make sure we don't leak memory if then() throws
			state->remove_ref(std::distance(begin, end) - 1);
			LIBASYNC_RETHROW();
		}
	}

	return out;
}

// when_all wrapper accepting ranges
template<typename T>
decltype(async::when_all(std::begin(std::declval<T>()), std::end(std::declval<T>()))) when_all(T&& tasks)
{
	return async::when_all(std::begin(std::forward<T>(tasks)), std::end(std::forward<T>(tasks)));
}

// when_any wrapper accepting ranges
template<typename T>
decltype(async::when_any(std::begin(std::declval<T>()), std::end(std::declval<T>()))) when_any(T&& tasks)
{
	return async::when_any(std::begin(std::forward<T>(tasks)), std::end(std::forward<T>(tasks)));
}

// when_all with variadic arguments
inline task<std::tuple<>> when_all()
{
	return async::make_task(std::tuple<>());
}
template<typename... T>
task<std::tuple<typename std::decay<T>::type...>> when_all(T&&... tasks)
{
	typedef std::tuple<typename std::decay<T>::type...> result_type;

	// Create shared state
	auto state = new detail::when_all_state<result_type>(sizeof...(tasks));
	auto out = state->event.get_task();

	// Register all the tasks on the event
	detail::when_all_variadic<0>(state, std::forward<T>(tasks)...);

	return out;
}

// when_any with variadic arguments
inline task<when_any_result<std::tuple<>>> when_any()
{
	return async::make_task(when_any_result<std::tuple<>>());
}
template<typename... T>
task<when_any_result<std::tuple<typename std::decay<T>::type...>>> when_any(T&&... tasks)
{
	typedef std::tuple<typename std::decay<T>::type...> result_type;

	// Create shared state
	auto state = new detail::when_any_state<result_type>(sizeof...(tasks));
	auto out = state->event.get_task();

	// Register all the tasks on the event
	detail::when_any_variadic<0>(state, std::forward<T>(tasks)...);

	return out;
}

} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {

// Exception thrown by cancel_current_task()
struct LIBASYNC_EXPORT_EXCEPTION task_canceled {};

// A flag which can be used to request cancellation
class cancellation_token {
	std::atomic<bool> state;

public:
	cancellation_token()
		: state(false) {}

	// Non-copyable and non-movable
	cancellation_token(const cancellation_token&) = delete;
	cancellation_token& operator=(const cancellation_token&) = delete;

	bool is_canceled() const
	{
		bool s = state.load(std::memory_order_relaxed);
		if (s)
			std::atomic_thread_fence(std::memory_order_acquire);
		return s;
	}

	void cancel()
	{
		state.store(true, std::memory_order_release);
	}

	void reset()
	{
		state.store(false, std::memory_order_relaxed);
	}
};

// Interruption point, throws task_canceled if the specified token is set.
inline void interruption_point(const cancellation_token& token)
{
	if (token.is_canceled())
		LIBASYNC_THROW(task_canceled());
}

} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {

// Range type representing a pair of iterators
template<typename Iter>
class range {
	Iter iter_begin, iter_end;

public:
	range() = default;
	range(Iter a, Iter b)
		: iter_begin(a), iter_end(b) {}

	Iter begin() const
	{
		return iter_begin;
	}
	Iter end() const
	{
		return iter_end;
	}
};

// Construct a range from 2 iterators
template<typename Iter>
range<Iter> make_range(Iter begin, Iter end)
{
	return {begin, end};
}

// A range of integers
template<typename T>
class int_range {
	T value_begin, value_end;

	static_assert(std::is_integral<T>::value, "int_range can only be used with integral types");

public:
	class iterator {
		T current;

		explicit iterator(T a)
			: current(a) {}
		friend class int_range<T>;

	public:
		typedef T value_type;
		typedef std::ptrdiff_t difference_type;
		typedef iterator pointer;
		typedef T reference;
		typedef std::random_access_iterator_tag iterator_category;

		iterator() = default;

		T operator*() const
		{
			return current;
		}
		T operator[](difference_type offset) const
		{
			return current + offset;
		}

		iterator& operator++()
		{
			++current;
			return *this;
		}
		iterator operator++(int)
		{
			return iterator(current++);
		}
		iterator& operator--()
		{
			--current;
			return *this;
		}
		iterator operator--(int)
		{
			return iterator(current--);
		}

		iterator& operator+=(difference_type offset)
		{
			current += offset;
			return *this;
		}
		iterator& operator-=(difference_type offset)
		{
			current -= offset;
			return *this;
		}

		iterator operator+(difference_type offset) const
		{
			return iterator(current + offset);
		}
		iterator operator-(difference_type offset) const
		{
			return iterator(current - offset);
		}

		friend iterator operator+(difference_type offset, iterator other)
		{
			return other + offset;
		}

		friend difference_type operator-(iterator a, iterator b)
		{
			return a.current - b.current;
		}

		friend bool operator==(iterator a, iterator b)
		{
			return a.current == b.current;
		}
		friend bool operator!=(iterator a, iterator b)
		{
			return a.current != b.current;
		}
		friend bool operator>(iterator a, iterator b)
		{
			return a.current > b.current;
		}
		friend bool operator<(iterator a, iterator b)
		{
			return a.current < b.current;
		}
		friend bool operator>=(iterator a, iterator b)
		{
			return a.current >= b.current;
		}
		friend bool operator<=(iterator a, iterator b)
		{
			return a.current <= b.current;
		}
	};

	int_range(T begin, T end)
		: value_begin(begin), value_end(end) {}

	iterator begin() const
	{
		return iterator(value_begin);
	}
	iterator end() const
	{
		return iterator(value_end);
	}
};

// Construct an int_range between 2 values
template<typename T, typename U>
int_range<typename std::common_type<T, U>::type> irange(T begin, U end)
{
	return {begin, end};
}

} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Partitioners are essentially ranges with an extra split() function. The
// split() function returns a partitioner containing a range to be executed in a
// child task and modifies the parent partitioner's range to represent the rest
// of the original range. If the range cannot be split any more then split()
// should return an empty range.

// Detect whether a range is a partitioner
template<typename T, typename = decltype(std::declval<T>().split())>
two& is_partitioner_helper(int);
template<typename T>
one& is_partitioner_helper(...);
template<typename T>
struct is_partitioner: public std::integral_constant<bool, sizeof(is_partitioner_helper<T>(0)) - 1> {};

// Automatically determine a grain size for a sequence length
inline std::size_t auto_grain_size(std::size_t dist)
{
	// Determine the grain size automatically using a heuristic
	std::size_t grain = dist / (8 * hardware_concurrency());
	if (grain < 1)
		grain = 1;
	if (grain > 2048)
		grain = 2048;
	return grain;
}

template<typename Iter>
class static_partitioner_impl {
	Iter iter_begin, iter_end;
	std::size_t grain;

public:
	static_partitioner_impl(Iter begin, Iter end, std::size_t grain)
		: iter_begin(begin), iter_end(end), grain(grain) {}
	Iter begin() const
	{
		return iter_begin;
	}
	Iter end() const
	{
		return iter_end;
	}
	static_partitioner_impl split()
	{
		// Don't split if below grain size
		std::size_t length = std::distance(iter_begin, iter_end);
		static_partitioner_impl out(iter_end, iter_end, grain);
		if (length <= grain)
			return out;

		// Split our range in half
		iter_end = iter_begin;
		std::advance(iter_end, (length + 1) / 2);
		out.iter_begin = iter_end;
		return out;
	}
};

template<typename Iter>
class auto_partitioner_impl {
	Iter iter_begin, iter_end;
	std::size_t grain;
	std::size_t num_threads;
	std::thread::id last_thread;

public:
	// thread_id is initialized to "no thread" and will be set on first split
	auto_partitioner_impl(Iter begin, Iter end, std::size_t grain)
		: iter_begin(begin), iter_end(end), grain(grain) {}
	Iter begin() const
	{
		return iter_begin;
	}
	Iter end() const
	{
		return iter_end;
	}
	auto_partitioner_impl split()
	{
		// Don't split if below grain size
		std::size_t length = std::distance(iter_begin, iter_end);
		auto_partitioner_impl out(iter_end, iter_end, grain);
		if (length <= grain)
			return out;

		// Check if we are in a different thread than we were before
		std::thread::id current_thread = std::this_thread::get_id();
		if (current_thread != last_thread)
			num_threads = hardware_concurrency();

		// If we only have one thread, don't split
		if (num_threads <= 1)
			return out;

		// Split our range in half
		iter_end = iter_begin;
		std::advance(iter_end, (length + 1) / 2);
		out.iter_begin = iter_end;
		out.last_thread = current_thread;
		last_thread = current_thread;
		out.num_threads = num_threads / 2;
		num_threads -= out.num_threads;
		return out;
	}
};

} // namespace detail

// A simple partitioner which splits until a grain size is reached. If a grain
// size is not specified, one is chosen automatically.
template<typename Range>
detail::static_partitioner_impl<decltype(std::begin(std::declval<Range>()))> static_partitioner(Range&& range, std::size_t grain)
{
	return {std::begin(range), std::end(range), grain};
}
template<typename Range>
detail::static_partitioner_impl<decltype(std::begin(std::declval<Range>()))> static_partitioner(Range&& range)
{
	std::size_t grain = detail::auto_grain_size(std::distance(std::begin(range), std::end(range)));
	return {std::begin(range), std::end(range), grain};
}

// A more advanced partitioner which initially divides the range into one chunk
// for each available thread. The range is split further if a chunk gets stolen
// by a different thread.
template<typename Range>
detail::auto_partitioner_impl<decltype(std::begin(std::declval<Range>()))> auto_partitioner(Range&& range)
{
	std::size_t grain = detail::auto_grain_size(std::distance(std::begin(range), std::end(range)));
	return {std::begin(range), std::end(range), grain};
}

// Wrap a range in a partitioner. If the input is already a partitioner then it
// is returned unchanged. This allows parallel algorithms to accept both ranges
// and partitioners as parameters.
template<typename Partitioner>
typename std::enable_if<detail::is_partitioner<typename std::decay<Partitioner>::type>::value, Partitioner&&>::type to_partitioner(Partitioner&& partitioner)
{
	return std::forward<Partitioner>(partitioner);
}
template<typename Range>
typename std::enable_if<!detail::is_partitioner<typename std::decay<Range>::type>::value, detail::auto_partitioner_impl<decltype(std::begin(std::declval<Range>()))>>::type to_partitioner(Range&& range)
{
	return async::auto_partitioner(std::forward<Range>(range));
}

// Overloads with std::initializer_list
template<typename T>
detail::static_partitioner_impl<decltype(std::declval<std::initializer_list<T>>().begin())> static_partitioner(std::initializer_list<T> range)
{
	return async::static_partitioner(async::make_range(range.begin(), range.end()));
}
template<typename T>
detail::static_partitioner_impl<decltype(std::declval<std::initializer_list<T>>().begin())> static_partitioner(std::initializer_list<T> range, std::size_t grain)
{
	return async::static_partitioner(async::make_range(range.begin(), range.end()), grain);
}
template<typename T>
detail::auto_partitioner_impl<decltype(std::declval<std::initializer_list<T>>().begin())> auto_partitioner(std::initializer_list<T> range)
{
	return async::auto_partitioner(async::make_range(range.begin(), range.end()));
}
template<typename T>
detail::auto_partitioner_impl<decltype(std::declval<std::initializer_list<T>>().begin())> to_partitioner(std::initializer_list<T> range)
{
	return async::auto_partitioner(async::make_range(range.begin(), range.end()));
}

} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Recursively split the arguments so tasks are spawned in parallel
template<std::size_t Start, std::size_t Count>
struct parallel_invoke_internal {
	template<typename Sched, typename Tuple>
	static void run(Sched& sched, const Tuple& args)
	{
		auto&& t = async::local_spawn(sched, [&sched, &args] {
			parallel_invoke_internal<Start + Count / 2, Count - Count / 2>::run(sched, args);
		});
		parallel_invoke_internal<Start, Count / 2>::run(sched, args);
		t.get();
	}
};
template<std::size_t Index>
struct parallel_invoke_internal<Index, 1> {
	template<typename Sched, typename Tuple>
	static void run(Sched&, const Tuple& args)
	{
		// Make sure to preserve the rvalue/lvalue-ness of the original parameter
		std::forward<typename std::tuple_element<Index, Tuple>::type>(std::get<Index>(args))();
	}
};
template<std::size_t Index>
struct parallel_invoke_internal<Index, 0> {
	template<typename Sched, typename Tuple>
	static void run(Sched&, const Tuple&) {}
};

} // namespace detail

// Run several functions in parallel, optionally using the specified scheduler.
template<typename Sched, typename... Args>
typename std::enable_if<detail::is_scheduler<Sched>::value>::type parallel_invoke(Sched& sched, Args&&... args)
{
	detail::parallel_invoke_internal<0, sizeof...(Args)>::run(sched, std::forward_as_tuple(std::forward<Args>(args)...));
}
template<typename... Args>
void parallel_invoke(Args&&... args)
{
	async::parallel_invoke(::async::default_scheduler(), std::forward<Args>(args)...);
}

} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Internal implementation of parallel_for that only accepts a partitioner
// argument.
template<typename Sched, typename Partitioner, typename Func>
void internal_parallel_for(Sched& sched, Partitioner partitioner, const Func& func)
{
	// Split the partition, run inline if no more splits are possible
	auto subpart = partitioner.split();
	if (subpart.begin() == subpart.end()) {
		for (auto&& i: partitioner)
			func(std::forward<decltype(i)>(i));
		return;
	}

	// Run the function over each half in parallel
	auto&& t = async::local_spawn(sched, [&sched, &subpart, &func] {
		detail::internal_parallel_for(sched, std::move(subpart), func);
	});
	detail::internal_parallel_for(sched, std::move(partitioner), func);
	t.get();
}

} // namespace detail

// Run a function for each element in a range
template<typename Sched, typename Range, typename Func>
void parallel_for(Sched& sched, Range&& range, const Func& func)
{
	detail::internal_parallel_for(sched, async::to_partitioner(std::forward<Range>(range)), func);
}

// Overload with default scheduler
template<typename Range, typename Func>
void parallel_for(Range&& range, const Func& func)
{
	async::parallel_for(::async::default_scheduler(), range, func);
}

// Overloads with std::initializer_list
template<typename Sched, typename T, typename Func>
void parallel_for(Sched& sched, std::initializer_list<T> range, const Func& func)
{
	async::parallel_for(sched, async::make_range(range.begin(), range.end()), func);
}
template<typename T, typename Func>
void parallel_for(std::initializer_list<T> range, const Func& func)
{
	async::parallel_for(async::make_range(range.begin(), range.end()), func);
}

} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Default map function which simply passes its parameter through unmodified
struct default_map {
	template<typename T>
	T&& operator()(T&& x) const
	{
		return std::forward<T>(x);
	}
};

// Internal implementation of parallel_map_reduce that only accepts a
// partitioner argument.
template<typename Sched, typename Partitioner, typename Result, typename MapFunc, typename ReduceFunc>
Result internal_parallel_map_reduce(Sched& sched, Partitioner partitioner, Result init, const MapFunc& map, const ReduceFunc& reduce)
{
	// Split the partition, run inline if no more splits are possible
	auto subpart = partitioner.split();
	if (subpart.begin() == subpart.end()) {
		Result out = init;
		for (auto&& i: partitioner)
			out = reduce(std::move(out), map(std::forward<decltype(i)>(i)));
		return out;
	}

	// Run the function over each half in parallel
	auto&& t = async::local_spawn(sched, [&sched, &subpart, init, &map, &reduce] {
		return detail::internal_parallel_map_reduce(sched, std::move(subpart), init, map, reduce);
	});
	Result out = detail::internal_parallel_map_reduce(sched, std::move(partitioner), init, map, reduce);
	return reduce(std::move(out), t.get());
}

} // namespace detail

// Run a function for each element in a range and then reduce the results of that function to a single value
template<typename Sched, typename Range, typename Result, typename MapFunc, typename ReduceFunc>
Result parallel_map_reduce(Sched& sched, Range&& range, Result init, const MapFunc& map, const ReduceFunc& reduce)
{
	return detail::internal_parallel_map_reduce(sched, async::to_partitioner(std::forward<Range>(range)), init, map, reduce);
}

// Overload with default scheduler
template<typename Range, typename Result, typename MapFunc, typename ReduceFunc>
Result parallel_map_reduce(Range&& range, Result init, const MapFunc& map, const ReduceFunc& reduce)
{
	return async::parallel_map_reduce(::async::default_scheduler(), range, init, map, reduce);
}

// Overloads with std::initializer_list
template<typename Sched, typename T, typename Result, typename MapFunc, typename ReduceFunc>
Result parallel_map_reduce(Sched& sched, std::initializer_list<T> range, Result init, const MapFunc& map, const ReduceFunc& reduce)
{
	return async::parallel_map_reduce(sched, async::make_range(range.begin(), range.end()), init, map, reduce);
}
template<typename T, typename Result, typename MapFunc, typename ReduceFunc>
Result parallel_map_reduce(std::initializer_list<T> range, Result init, const MapFunc& map, const ReduceFunc& reduce)
{
	return async::parallel_map_reduce(async::make_range(range.begin(), range.end()), init, map, reduce);
}

// Variant with identity map operation
template<typename Sched, typename Range, typename Result, typename ReduceFunc>
Result parallel_reduce(Sched& sched, Range&& range, Result init, const ReduceFunc& reduce)
{
	return async::parallel_map_reduce(sched, range, init, detail::default_map(), reduce);
}
template<typename Range, typename Result, typename ReduceFunc>
Result parallel_reduce(Range&& range, Result init, const ReduceFunc& reduce)
{
	return async::parallel_reduce(::async::default_scheduler(), range, init, reduce);
}
template<typename Sched, typename T, typename Result, typename ReduceFunc>
Result parallel_reduce(Sched& sched, std::initializer_list<T> range, Result init, const ReduceFunc& reduce)
{
	return async::parallel_reduce(sched, async::make_range(range.begin(), range.end()), init, reduce);
}
template<typename T, typename Result, typename ReduceFunc>
Result parallel_reduce(std::initializer_list<T> range, Result init, const ReduceFunc& reduce)
{
	return async::parallel_reduce(async::make_range(range.begin(), range.end()), init, reduce);
}

} // namespace async

#ifndef LIBASYNC_STATIC
#if defined(__GNUC__) && !defined(_WIN32)
# pragma GCC visibility pop
#endif
#endif

#endif

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <numeric>
#include <random>
#include <thread>
#include <type_traits>
#include <vector>

// We don't make use of dynamic TLS initialization/destruction so we can just
// use the legacy TLS attributes.
#ifdef __GNUC__
# define  THREAD_LOCAL __thread
#elif defined (_MSC_VER)
# define THREAD_LOCAL __declspec(thread)
#else
# define THREAD_LOCAL thread_local
#endif

// GCC, Clang and the Linux version of the Intel compiler and MSVC 2015 support
// thread-safe initialization of function-scope static variables.
#ifdef __GNUC__
# define HAVE_THREAD_SAFE_STATIC
#elif _MSC_VER >= 1900 && !defined(__INTEL_COMPILER)
# define HAVE_THREAD_SAFE_STATIC
#endif

// MSVC deadlocks when joining a thread from a static destructor. Use a
// workaround in that case to avoid the deadlock.
#if defined(_MSC_VER) && _MSC_VER < 1900
# define BROKEN_JOIN_IN_DESTRUCTOR
#endif

// Apple's iOS has no thread local support yet. They claim that they don't want to
// introduce a binary compatility issue when they got a better implementation available.
// Luckily, pthreads supports some kind of "emulation" for that. This detects if the we
// are compiling for iOS and enables the workaround accordingly.
// It is also possible enabling it forcibly by setting the EMULATE_PTHREAD_THREAD_LOCAL
// macro. Obviously, this will only works on platforms with pthread available.
#if __APPLE__
//# include "TargetConditionals.h"
# if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#  define EMULATE_PTHREAD_THREAD_LOCAL
# endif
#endif

// Force symbol visibility to hidden unless explicity exported
#ifndef LIBASYNC_STATIC
#if defined(__GNUC__) && !defined(_WIN32)
# pragma GCC visibility push(hidden)
#endif
#endif

// Include other internal headers

namespace async {
namespace detail {

// Thread-safe singleton wrapper class
#ifdef HAVE_THREAD_SAFE_STATIC
// C++11 guarantees thread safety for static initialization
template<typename T>
class singleton {
public:
	static T& get_instance()
	{
		static T instance;
		return instance;
	}
};
#else
// Some compilers don't support thread-safe static initialization, so emulate it
template<typename T>
class singleton {
	std::mutex lock;
	std::atomic<bool> init_flag;
	typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type storage;

	static singleton instance;

	// Use a destructor instead of atexit() because the latter does not work
	// properly when the singleton is in a library that is unloaded.
	~singleton()
	{
		if (init_flag.load(std::memory_order_acquire))
			reinterpret_cast<T*>(&storage)->~T();
	}

public:
	static T& get_instance()
	{
		T* ptr = reinterpret_cast<T*>(&instance.storage);
		if (!instance.init_flag.load(std::memory_order_acquire)) {
			std::lock_guard<std::mutex> locked(instance.lock);
			if (!instance.init_flag.load(std::memory_order_relaxed)) {
				new(ptr) T;
				instance.init_flag.store(true, std::memory_order_release);
			}
		}
		return *ptr;
	}
};

template<typename T> singleton<T> singleton<T>::instance;
#endif

} // namespace detail
} // namespace async

namespace async {
namespace detail {

// Set of events that an task_wait_event can hold
enum wait_type {
	// The task that is being waited on has completed
	task_finished = 1,

	// A task is available to execute from the scheduler
	task_available = 2
};

// OS-supported event object which can be used to wait for either a task to
// finish or for the scheduler to have more work for the current thread.
//
// The event object is lazily initialized to avoid unnecessary API calls.
class task_wait_event {
	std::aligned_storage<sizeof(std::mutex), std::alignment_of<std::mutex>::value>::type m;
	std::aligned_storage<sizeof(std::condition_variable), std::alignment_of<std::condition_variable>::value>::type c;
	int event_mask;
	bool initialized;

	std::mutex& mutex()
	{
		return *reinterpret_cast<std::mutex*>(&m);
	}
	std::condition_variable& cond()
	{
		return *reinterpret_cast<std::condition_variable*>(&c);
	}

public:
	task_wait_event()
		: event_mask(0), initialized(false) {}

	~task_wait_event()
	{
		if (initialized) {
			mutex().~mutex();
			cond().~condition_variable();
		}
	}

	// Initialize the event, must be done before any other functions are called.
	void init()
	{
		if (!initialized) {
			new(&m) std::mutex;
			new(&c) std::condition_variable;
			initialized = true;
		}
	}

	// Wait for an event to occur. Returns the event(s) that occurred. This also
	// clears any pending events afterwards.
	int wait()
	{
		std::unique_lock<std::mutex> lock(mutex());
		while (event_mask == 0)
			cond().wait(lock);
		int result = event_mask;
		event_mask = 0;
		return result;
	}

	// Check if a specific event is ready
	bool try_wait(int event)
	{
		std::lock_guard<std::mutex> lock(mutex());
		int result = event_mask & event;
		event_mask &= ~event;
		return result != 0;
	}

	// Signal an event and wake up a sleeping thread
	void signal(int event)
	{
		std::unique_lock<std::mutex> lock(mutex());
		event_mask |= event;

		// This must be done while holding the lock otherwise we may end up with
		// a use-after-free due to a race with wait().
		cond().notify_one();
		lock.unlock();
	}
};

} // namespace detail
} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Queue which holds tasks in FIFO order. Note that this queue is not
// thread-safe and must be protected by a lock.
class fifo_queue {
	detail::aligned_array<void*, LIBASYNC_CACHELINE_SIZE> items;
	std::size_t head, tail;

public:
	fifo_queue()
		: items(32), head(0), tail(0) {}
	~fifo_queue()
	{
		// Free any unexecuted tasks
		for (std::size_t i = head; i != tail; i = (i + 1) & (items.size() - 1))
			task_run_handle::from_void_ptr(items[i]);
	}

	// Push a task to the end of the queue
	void push(task_run_handle t)
	{
		// Resize queue if it is full
		if (head == ((tail + 1) & (items.size() - 1))) {
			detail::aligned_array<void*, LIBASYNC_CACHELINE_SIZE> new_items(items.size() * 2);
			for (std::size_t i = 0; i != items.size(); i++)
				new_items[i] = items[(i + head) & (items.size() - 1)];
			head = 0;
			tail = items.size() - 1;
			items = std::move(new_items);
		}

		// Push the item
		items[tail] = t.to_void_ptr();
		tail = (tail + 1) & (items.size() - 1);
	}

	// Pop a task from the front of the queue
	task_run_handle pop()
	{
		// See if an item is available
		if (head == tail)
			return task_run_handle();
		else {
			void* x = items[head];
			head = (head + 1) & (items.size() - 1);
			return task_run_handle::from_void_ptr(x);
		}
	}
};

} // namespace detail
} // namespace async

#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Chase-Lev work stealing deque
//
// Dynamic Circular Work-Stealing Deque
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.170.1097&rep=rep1&type=pdf
//
// Correct and Efﬁcient Work-Stealing for Weak Memory Models
// http://www.di.ens.fr/~zappa/readings/ppopp13.pdf
class work_steal_queue {
	// Circular array of void*
	class circular_array {
		detail::aligned_array<void*, LIBASYNC_CACHELINE_SIZE> items;
		std::unique_ptr<circular_array> previous;

	public:
		circular_array(std::size_t n)
			: items(n) {}

		std::size_t size() const
		{
			return items.size();
		}

		void* get(std::size_t index)
		{
			return items[index & (size() - 1)];
		}

		void put(std::size_t index, void* x)
		{
			items[index & (size() - 1)] = x;
		}

		// Growing the array returns a new circular_array object and keeps a
		// linked list of all previous arrays. This is done because other threads
		// could still be accessing elements from the smaller arrays.
		circular_array* grow(std::size_t top, std::size_t bottom)
		{
			circular_array* new_array = new circular_array(size() * 2);
			new_array->previous.reset(this);
			for (std::size_t i = top; i != bottom; i++)
				new_array->put(i, get(i));
			return new_array;
		}
	};

	std::atomic<circular_array*> array;
	std::atomic<std::size_t> top, bottom;

	// Convert a 2's complement unsigned value to a signed value. We need to do
	// this because (b - t) may not always be positive.
	static std::ptrdiff_t to_signed(std::size_t x)
	{
		// Unsigned to signed conversion is implementation-defined if the value
		// doesn't fit, so we convert manually.
		static_assert(static_cast<std::size_t>(PTRDIFF_MAX) + 1 == static_cast<std::size_t>(PTRDIFF_MIN), "Wrong integer wrapping behavior");
		if (x > static_cast<std::size_t>(PTRDIFF_MAX))
			return static_cast<std::ptrdiff_t>(x - static_cast<std::size_t>(PTRDIFF_MIN)) + PTRDIFF_MIN;
		else
			return static_cast<std::ptrdiff_t>(x);
	}

public:
	work_steal_queue()
		: array(new circular_array(32)), top(0), bottom(0) {}
	~work_steal_queue()
	{
		// Free any unexecuted tasks
		std::size_t b = bottom.load(std::memory_order_relaxed);
		std::size_t t = top.load(std::memory_order_relaxed);
		circular_array* a = array.load(std::memory_order_relaxed);
		for (std::size_t i = t; i != b; i++)
			task_run_handle::from_void_ptr(a->get(i));
		delete a;
	}

	// Push a task to the bottom of this thread's queue
	void push(task_run_handle x)
	{
		std::size_t b = bottom.load(std::memory_order_relaxed);
		std::size_t t = top.load(std::memory_order_acquire);
		circular_array* a = array.load(std::memory_order_relaxed);

		// Grow the array if it is full
		if (to_signed(b - t) >= to_signed(a->size())) {
			a = a->grow(t, b);
			array.store(a, std::memory_order_release);
		}

		// Note that we only convert to void* here in case grow throws due to
		// lack of memory.
		a->put(b, x.to_void_ptr());
		std::atomic_thread_fence(std::memory_order_release);
		bottom.store(b + 1, std::memory_order_relaxed);
	}

	// Pop a task from the bottom of this thread's queue
	task_run_handle pop()
	{
		std::size_t b = bottom.load(std::memory_order_relaxed);

		// Early exit if queue is empty
		std::size_t t = top.load(std::memory_order_relaxed);
		if (to_signed(b - t) <= 0)
			return task_run_handle();

		// Make sure bottom is stored before top is read
		bottom.store(--b, std::memory_order_relaxed);
		std::atomic_thread_fence(std::memory_order_seq_cst);
		t = top.load(std::memory_order_relaxed);

		// If the queue is empty, restore bottom and exit
		if (to_signed(b - t) < 0) {
			bottom.store(b + 1, std::memory_order_relaxed);
			return task_run_handle();
		}

		// Fetch the element from the queue
		circular_array* a = array.load(std::memory_order_relaxed);
		void* x = a->get(b);

		// If this was the last element in the queue, check for races
		if (b == t) {
			if (!top.compare_exchange_strong(t, t + 1, std::memory_order_seq_cst, std::memory_order_relaxed)) {
				bottom.store(b + 1, std::memory_order_relaxed);
				return task_run_handle();
			}
			bottom.store(b + 1, std::memory_order_relaxed);
		}
		return task_run_handle::from_void_ptr(x);
	}

	// Steal a task from the top of this thread's queue
	task_run_handle steal()
	{
		// Loop while the compare_exchange fails. This is still lock-free because
		// a fail means that another thread has sucessfully stolen a task.
		while (true) {
			// Make sure top is read before bottom
			std::size_t t = top.load(std::memory_order_acquire);
			std::atomic_thread_fence(std::memory_order_seq_cst);
			std::size_t b = bottom.load(std::memory_order_acquire);

			// Exit if the queue is empty
			if (to_signed(b - t) <= 0)
				return task_run_handle();

			// Fetch the element from the queue
			circular_array* a = array.load(std::memory_order_consume);
			void* x = a->get(t);

			// Attempt to increment top
			if (top.compare_exchange_weak(t, t + 1, std::memory_order_seq_cst, std::memory_order_relaxed))
				return task_run_handle::from_void_ptr(x);
		}
	}
};

} // namespace detail
} // namespace async

// for pthread thread_local emulation
#if defined(EMULATE_PTHREAD_THREAD_LOCAL)
# include <pthread.h>
#endif

namespace async {
namespace detail {

static inline void* aligned_alloc(std::size_t size, std::size_t align)
{
#ifdef _WIN32
	void* ptr = _aligned_malloc(size, align);
	if (!ptr)
		LIBASYNC_THROW(std::bad_alloc());
	return ptr;
#else
	void* result;
	if (posix_memalign(&result, align, size))
		LIBASYNC_THROW(std::bad_alloc());
	else
		return result;
#endif
}

static inline void aligned_free(void* addr) LIBASYNC_NOEXCEPT
{
#ifdef _WIN32
	_aligned_free(addr);
#else
	free(addr);
#endif
}
}
}

#if defined(LIBASYNC_IMPLEMENTATION)
namespace async {
namespace detail {

// Wait for a task to complete (for threads outside thread pool)
static void generic_wait_handler(task_wait_handle wait_task)
{
	// Create an event to wait on
	task_wait_event event;
	event.init();

	// Create a continuation for the task we are waiting for
	wait_task.on_finish([&event] {
		// Just signal the thread event
		event.signal(wait_type::task_finished);
	});

	// Wait for the event to be set
	event.wait();
}

#if defined(EMULATE_PTHREAD_THREAD_LOCAL)
// Wait handler function, per-thread, defaults to generic version
struct pthread_emulation_thread_wait_handler_key_initializer {
	pthread_key_t key;
		
	pthread_emulation_thread_wait_handler_key_initializer()
	{
		pthread_key_create(&key, nullptr);
	}
		
	~pthread_emulation_thread_wait_handler_key_initializer()
	{
		pthread_key_delete(key);
	}
};
	
static pthread_key_t get_thread_wait_handler_key()
{
	static pthread_emulation_thread_wait_handler_key_initializer initializer;
	return initializer.key;
}

#else
static THREAD_LOCAL wait_handler thread_wait_handler = generic_wait_handler;
#endif

static void set_thread_wait_handler_(wait_handler handler)
{
#if defined(EMULATE_PTHREAD_THREAD_LOCAL)
	// we need to call this here, because the pthread initializer is lazy,
	// this means the it could be null and we need to set it before trying to
	// get or set it
	pthread_setspecific(get_thread_wait_handler_key(), reinterpret_cast<void*>(handler));
#else
	thread_wait_handler = handler;
#endif
}
	
static wait_handler get_thread_wait_handler()
{
#if defined(EMULATE_PTHREAD_THREAD_LOCAL)
	// we need to call this here, because the pthread initializer is lazy,
	// this means the it could be null and we need to set it before trying to
	// get or set it
	wait_handler handler = (wait_handler) pthread_getspecific(get_thread_wait_handler_key());
	if(handler == nullptr) {
		return generic_wait_handler;
	}
	return handler;
#else
	return thread_wait_handler;
#endif
}

// Wait for a task to complete
void wait_for_task(task_base* wait_task)
{
	// Dispatch to the current thread's wait handler
	wait_handler thread_wait_handler = get_thread_wait_handler();
	thread_wait_handler(task_wait_handle(wait_task));
}

// The default scheduler is just a thread pool which can be configured
// using environment variables.
class default_scheduler_impl: public threadpool_scheduler {
	static std::size_t get_num_threads()
	{
		// Get the requested number of threads from the environment
		// If that fails, use the number of CPUs in the system.
		std::size_t num_threads;
#ifdef _MSC_VER
		char* s;
# ifdef __cplusplus_winrt
		// Windows store applications do not support environment variables
		s = nullptr;
# else
		// MSVC gives an error when trying to use getenv, work around this
		// by using _dupenv_s instead.
		_dupenv_s(&s, nullptr, "LIBASYNC_NUM_THREADS");
# endif
#else
		const char *s = std::getenv("LIBASYNC_NUM_THREADS");
#endif
		if (s)
			num_threads = std::strtoul(s, nullptr, 10);
		else
			num_threads = hardware_concurrency();

#if defined(_MSC_VER) && !defined(__cplusplus_winrt)
		// Free the string allocated by _dupenv_s
		free(s);
#endif

		// Make sure the thread count is reasonable
		if (num_threads < 1)
			num_threads = 1;
		return num_threads;
	}

public:
	default_scheduler_impl()
		: threadpool_scheduler(get_num_threads()) {}
};

// Thread scheduler implementation
void thread_scheduler_impl::schedule(task_run_handle t)
{
	// A shared_ptr is used here because not all implementations of
	// std::thread support move-only objects.
	std::thread([](const std::shared_ptr<task_run_handle>& t) {
		t->run();
	}, std::make_shared<task_run_handle>(std::move(t))).detach();
}

} // namespace detail

threadpool_scheduler& default_threadpool_scheduler()
{
	return detail::singleton<detail::default_scheduler_impl>::get_instance();
}

// FIFO scheduler implementation
struct fifo_scheduler::internal_data {
	detail::fifo_queue queue;
	std::mutex lock;
};
fifo_scheduler::fifo_scheduler()
	: impl(new internal_data) {}
fifo_scheduler::~fifo_scheduler() {}
void fifo_scheduler::schedule(task_run_handle t)
{
	std::lock_guard<std::mutex> locked(impl->lock);
	impl->queue.push(std::move(t));
}
bool fifo_scheduler::try_run_one_task()
{
	task_run_handle t;
	{
		std::lock_guard<std::mutex> locked(impl->lock);
		t = impl->queue.pop();
	}
	if (t) {
		t.run();
		return true;
	}
	return false;
}
void fifo_scheduler::run_all_tasks()
{
	while (try_run_one_task()) {}
}

std::size_t hardware_concurrency() LIBASYNC_NOEXCEPT
{
	// Cache the value because calculating it may be expensive
	static std::size_t value = std::thread::hardware_concurrency();

	// Always return at least 1 core
	return value == 0 ? 1 : value;
}

wait_handler set_thread_wait_handler(wait_handler handler) LIBASYNC_NOEXCEPT
{
	wait_handler old = detail::get_thread_wait_handler();
	detail::set_thread_wait_handler_(handler);
	return old;
}

} // namespace async
#ifndef LIBASYNC_STATIC
#if defined(__GNUC__) && !defined(_WIN32)
# pragma GCC visibility pop
#endif
#endif

// For GetProcAddress and GetModuleHandle
#ifdef _WIN32
#include <windows.h>
#endif

// for pthread thread_local emulation
#if defined(EMULATE_PTHREAD_THREAD_LOCAL)
# include <pthread.h>
#endif

namespace async {
namespace detail {

// Per-thread data, aligned to cachelines to avoid false sharing
struct LIBASYNC_CACHELINE_ALIGN thread_data_t {
	work_steal_queue queue;
	std::minstd_rand rng;
	std::thread handle;
};

// Internal data used by threadpool_scheduler
struct threadpool_data {
	threadpool_data(std::size_t num_threads)
		: thread_data(num_threads), shutdown(false), num_waiters(0), waiters(new task_wait_event*[num_threads]) {}

    threadpool_data(std::size_t num_threads, std::function<void()>&& prerun_, std::function<void()>&& postrun_)
		: thread_data(num_threads), shutdown(false), num_waiters(0), waiters(new task_wait_event*[num_threads]),
          prerun(std::move(prerun_)), postrun(std::move(postrun_)) {}

	// Mutex protecting everything except thread_data
	std::mutex lock;

	// Array of per-thread data
	aligned_array<thread_data_t> thread_data;

	// Global queue for tasks from outside the pool
	fifo_queue public_queue;

	// Shutdown request indicator
	bool shutdown;

	// List of threads waiting for tasks to run. num_waiters needs to be atomic
	// because it is sometimes read outside the mutex.
	std::atomic<std::size_t> num_waiters;
	std::unique_ptr<task_wait_event*[]> waiters;

	// Pre/Post run functions.
    std::function<void()> prerun;
    std::function<void()> postrun;

#ifdef BROKEN_JOIN_IN_DESTRUCTOR
	// Shutdown complete event, used instead of thread::join()
	std::size_t shutdown_num_threads;
	std::condition_variable shutdown_complete_event;
#endif
};

// this wrapper encapsulates both the owning_threadpool pointer and the thread id.
// this is done to improve performance on the emulated thread_local reducing the number
// of calls to "pthread_getspecific"
struct threadpool_data_wrapper {
	threadpool_data* owning_threadpool;
	std::size_t thread_id;

	threadpool_data_wrapper(threadpool_data* owning_threadpool, std::size_t thread_id):
		owning_threadpool(owning_threadpool), thread_id(thread_id) { }
};

#if defined(EMULATE_PTHREAD_THREAD_LOCAL)
struct pthread_emulation_threadpool_data_initializer {
	pthread_key_t key;
	
	pthread_emulation_threadpool_data_initializer()
	{
		pthread_key_create(&key, [](void* wrapper_ptr) {
			threadpool_data_wrapper* wrapper = static_cast<threadpool_data_wrapper*>(wrapper_ptr);
			delete wrapper;
		});
	}
		
	~pthread_emulation_threadpool_data_initializer()
	{
		pthread_key_delete(key);
	}
};
	
static pthread_key_t get_local_threadpool_data_key()
{
	static pthread_emulation_threadpool_data_initializer initializer;
	return initializer.key;
}

#else
// Thread pool this thread belongs to, or null if not in pool
static THREAD_LOCAL threadpool_data* owning_threadpool = nullptr;

// Current thread's index in the pool
static THREAD_LOCAL std::size_t thread_id;
#endif
	
static void create_threadpool_data(threadpool_data* owning_threadpool_, std::size_t thread_id_)
{
#if defined(EMULATE_PTHREAD_THREAD_LOCAL)
	// the memory allocated here gets deallocated by the lambda declared on the key creation
	pthread_setspecific(get_local_threadpool_data_key(), new threadpool_data_wrapper(owning_threadpool_, thread_id_));
#else
	owning_threadpool = owning_threadpool_;
	thread_id = thread_id_;
#endif
}
	
static threadpool_data_wrapper get_threadpool_data_wrapper()
{
#if defined(EMULATE_PTHREAD_THREAD_LOCAL)
	threadpool_data_wrapper* wrapper = static_cast<threadpool_data_wrapper*>(pthread_getspecific(get_local_threadpool_data_key()));
	if(wrapper == nullptr) {
		// if, for some reason, the wrapper is not set, this won't cause a crash
		return threadpool_data_wrapper(nullptr, 0);
	}
	return *wrapper;
#else
	return threadpool_data_wrapper(owning_threadpool, thread_id);
#endif
}

// Try to steal a task from another thread's queue
static task_run_handle steal_task(threadpool_data* impl, std::size_t thread_id)
{
	// Make a list of victim thread ids and shuffle it
	std::vector<std::size_t> victims(impl->thread_data.size());
	std::iota(victims.begin(), victims.end(), 0);
	std::shuffle(victims.begin(), victims.end(), impl->thread_data[thread_id].rng);

	// Try to steal from another thread
	for (std::size_t i: victims) {
		// Don't try to steal from ourself
		if (i == thread_id)
			continue;

		if (task_run_handle t = impl->thread_data[i].queue.steal())
			return t;
	}

	// No tasks found, but we might have missed one if it was just added. In
	// practice this doesn't really matter since it will be handled by another
	// thread.
	return task_run_handle();
}

// Main task stealing loop which is used by worker threads when they have
// nothing to do.
static void thread_task_loop(threadpool_data* impl, std::size_t thread_id, task_wait_handle wait_task)
{
	// Get our thread's data
	thread_data_t& current_thread = impl->thread_data[thread_id];

	// Flag indicating if we have added a continuation to the task
	bool added_continuation = false;

	// Event to wait on
	task_wait_event event;

	// Loop while waiting for the task to complete
	while (true) {
		// Check if the task has finished. If we have added a continuation, we
		// need to make sure the event has been signaled, otherwise the other
		// thread may try to signal it after we have freed it.
		if (wait_task && (added_continuation ? event.try_wait(wait_type::task_finished) : wait_task.ready()))
			return;

		// Try to get a task from the local queue
		if (task_run_handle t = current_thread.queue.pop()) {
			t.run();
			continue;
		}

		// Stealing loop
		while (true) {
			// Try to steal a task
			if (task_run_handle t = steal_task(impl, thread_id)) {
				t.run();
				break;
			}

			// Try to fetch from the public queue
			std::unique_lock<std::mutex> locked(impl->lock);
			if (task_run_handle t = impl->public_queue.pop()) {
				// Don't hold the lock while running the task
				locked.unlock();
				t.run();
				break;
			}

			// If shutting down and we don't have a task to wait for, return.
			if (!wait_task && impl->shutdown) {
#ifdef BROKEN_JOIN_IN_DESTRUCTOR
				// Notify once all worker threads have exited
				if (--impl->shutdown_num_threads == 0)
					impl->shutdown_complete_event.notify_one();
#endif
				return;
			}

			// Initialize the event object
			event.init();

			// No tasks found, so sleep until something happens.
			// If a continuation has not been added yet, add it.
			if (wait_task && !added_continuation) {
				// Create a continuation for the task we are waiting for
				wait_task.on_finish([&event] {
					// Signal the thread's event
					event.signal(wait_type::task_finished);
				});
				added_continuation = true;
			}

			// Add our thread to the list of waiting threads
			size_t num_waiters_val = impl->num_waiters.load(std::memory_order_relaxed);
			impl->waiters[num_waiters_val] = &event;
			impl->num_waiters.store(num_waiters_val + 1, std::memory_order_relaxed);

			// Wait for our event to be signaled when a task is scheduled or
			// the task we are waiting for has completed.
			locked.unlock();
			int events = event.wait();
			locked.lock();

			// Remove our thread from the list of waiting threads
			num_waiters_val = impl->num_waiters.load(std::memory_order_relaxed);
			for (std::size_t i = 0; i < num_waiters_val; i++) {
				if (impl->waiters[i] == &event) {
					if (i != num_waiters_val - 1)
						std::swap(impl->waiters[i], impl->waiters[num_waiters_val - 1]);
					impl->num_waiters.store(num_waiters_val - 1, std::memory_order_relaxed);
					break;
				}
			}

			// Check again if the task has finished. We have added a
			// continuation at this point, so we need to check that the
			// continuation has finished signaling the event.
			if (wait_task && (events & wait_type::task_finished))
				return;
		}
	}
}

// Wait for a task to complete (for worker threads inside thread pool)
static void threadpool_wait_handler(task_wait_handle wait_task)
{
	threadpool_data_wrapper wrapper = get_threadpool_data_wrapper();
	thread_task_loop(wrapper.owning_threadpool, wrapper.thread_id, wait_task);
}

// Worker thread main loop
static void worker_thread(threadpool_data* owning_threadpool, std::size_t thread_id)
{
	// store on the local thread data
	create_threadpool_data(owning_threadpool, thread_id);

	// Set the wait handler so threads from the pool do useful work while
	// waiting for another task to finish.
	set_thread_wait_handler(threadpool_wait_handler);

	// Seed the random number generator with our id. This gives each thread a
	// different steal order.
	owning_threadpool->thread_data[thread_id].rng.seed(static_cast<std::minstd_rand::result_type>(thread_id));

    // Prerun hook
    if (owning_threadpool->prerun) owning_threadpool->prerun();

	// Main loop, runs until the shutdown signal is recieved
	thread_task_loop(owning_threadpool, thread_id, task_wait_handle());

    // Postrun hook
    if (owning_threadpool->postrun) owning_threadpool->postrun();
}

// Recursive function to spawn all worker threads in parallel
static void recursive_spawn_worker_thread(threadpool_data* impl, std::size_t index, std::size_t threads)
{
	// If we are down to one thread, go to the worker main loop
	if (threads == 1)
		worker_thread(impl, index);
	else {
		// Split thread range into 2 sub-ranges
		std::size_t mid = index + threads / 2;

		// Spawn a thread for half of the range
		impl->thread_data[mid].handle = std::thread(recursive_spawn_worker_thread, impl, mid, threads - threads / 2);
#ifdef BROKEN_JOIN_IN_DESTRUCTOR
		impl->thread_data[mid].handle.detach();
#endif

		// Tail-recurse to handle our half of the range
		recursive_spawn_worker_thread(impl, index, threads / 2);
	}
}

} // namespace detail

threadpool_scheduler::threadpool_scheduler(threadpool_scheduler&& other)
        : impl(std::move(other.impl)) {}

threadpool_scheduler::threadpool_scheduler(std::size_t num_threads)
	: impl(new detail::threadpool_data(num_threads))
{
	// Start worker threads
	impl->thread_data[0].handle = std::thread(detail::recursive_spawn_worker_thread, impl.get(), 0, num_threads);
#ifdef BROKEN_JOIN_IN_DESTRUCTOR
	impl->thread_data[0].handle.detach();
#endif
}

threadpool_scheduler::threadpool_scheduler(std::size_t num_threads,
                                           std::function<void()>&& prerun,
                                           std::function<void()>&& postrun)
    : impl(new detail::threadpool_data(num_threads, std::move(prerun), std::move(postrun)))
{
	// Start worker threads
	impl->thread_data[0].handle = std::thread(detail::recursive_spawn_worker_thread, impl.get(), 0, num_threads);
#ifdef BROKEN_JOIN_IN_DESTRUCTOR
	impl->thread_data[0].handle.detach();
#endif
}

// Wait for all currently running tasks to finish
threadpool_scheduler::~threadpool_scheduler()
{
    if (!impl) return;
#ifdef _WIN32
	// Windows kills all threads except one on process exit before calling
	// global destructors in DLLs. Waiting for dead threads to exit will likely
	// result in deadlocks, so we just exit early if we detect that the process
	// is exiting.
	auto RtlDllShutdownInProgress = reinterpret_cast<BOOLEAN(WINAPI *)()>((void *)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlDllShutdownInProgress"));
	if (RtlDllShutdownInProgress && RtlDllShutdownInProgress()) {
# ifndef BROKEN_JOIN_IN_DESTRUCTOR
		// We still need to detach the thread handles otherwise the std::thread
		// destructor will throw an exception.
		for (std::size_t i = 0; i < impl->thread_data.size(); i++) {
# ifndef LIBASYNC_NO_EXCEPTIONS
			try {
# endif
				impl->thread_data[i].handle.detach();
# ifndef LIBASYNC_NO_EXCEPTIONS
			} catch (...) {}
# endif
		}
# endif
		return;
	}
#endif

	{
		std::unique_lock<std::mutex> locked(impl->lock);

		// Signal shutdown
		impl->shutdown = true;

		// Wake up any sleeping threads
		size_t num_waiters_val = impl->num_waiters.load(std::memory_order_relaxed);
		for (std::size_t i = 0; i < num_waiters_val; i++)
			impl->waiters[i]->signal(detail::wait_type::task_available);
		impl->num_waiters.store(0, std::memory_order_relaxed);

#ifdef BROKEN_JOIN_IN_DESTRUCTOR
		// Wait for the threads to exit
		impl->shutdown_num_threads = impl->thread_data.size();
		impl->shutdown_complete_event.wait(locked);
#endif
	}

#ifndef BROKEN_JOIN_IN_DESTRUCTOR
	// Wait for the threads to exit
	for (std::size_t i = 0; i < impl->thread_data.size(); i++)
		impl->thread_data[i].handle.join();
#endif
}

// Schedule a task on the thread pool
void threadpool_scheduler::schedule(task_run_handle t)
{
	detail::threadpool_data_wrapper wrapper = detail::get_threadpool_data_wrapper();
	
	// Check if we are in the thread pool
	if (wrapper.owning_threadpool == impl.get()) {
		// Push the task onto our task queue
		impl->thread_data[wrapper.thread_id].queue.push(std::move(t));

		// If there are no sleeping threads, just return. We check outside the
		// lock to avoid locking overhead in the fast path.
		if (impl->num_waiters.load(std::memory_order_relaxed) == 0)
			return;

		// Get a thread to wake up from the list
		std::lock_guard<std::mutex> locked(impl->lock);

		// Check again if there are waiters
		size_t num_waiters_val = impl->num_waiters.load(std::memory_order_relaxed);
		if (num_waiters_val == 0)
			return;

		// Pop a thread from the list and wake it up
		impl->waiters[num_waiters_val - 1]->signal(detail::wait_type::task_available);
		impl->num_waiters.store(num_waiters_val - 1, std::memory_order_relaxed);
	} else {
		std::lock_guard<std::mutex> locked(impl->lock);

		// Push task onto the public queue
		impl->public_queue.push(std::move(t));

		// Wake up a sleeping thread
		size_t num_waiters_val = impl->num_waiters.load(std::memory_order_relaxed);
		if (num_waiters_val == 0)
			return;
		impl->waiters[num_waiters_val - 1]->signal(detail::wait_type::task_available);
		impl->num_waiters.store(num_waiters_val - 1, std::memory_order_relaxed);
	}
}

} // namespace async

#endif // LIBASYNC_IMPLEMENTATION

#ifndef LIBASYNC_STATIC
#if defined(__GNUC__) && !defined(_WIN32)
# pragma GCC visibility pop
#endif
#endif

