
#include <unistd.h>

#include "watchman/plugin.hxx"
#include "watchman/alloc.hxx"
#include "watchman/child.hxx"
#include "watchman/buffer.hxx"
#include "watchman/file.hxx"
#include "watchman/watchman.hxx"
#include "watchman/compiler.hxx"
#include "watchman/error.hxx"

class Test1_Child : public Child
{

public:
	int		execute();
	int		terminate();
	int		kill();

};

class Test1_Plugin : public Watchman_Plugin
{

public:
			explicit Test1_Plugin(void *handle, int version);

public:
	int		init(Watchman *w, int argc, char **argv);
	int		fini();

private:
	Allocator	*_alloc;

private:
	Test1_Child	_proc;

private:
	Buffer		_buf;

private:
	File		*_fo;
	File		*_fe;
};

int Test1_Child::execute()
{
	WATCHMAN_DEBUG("function entry point");
	return 0;
}

int Test1_Child::terminate()
{
	WATCHMAN_DEBUG("function entry point");
	return 0;
}

int Test1_Child::kill()
{
	WATCHMAN_DEBUG("function entry point");
	return 0;
}

Test1_Plugin::Test1_Plugin(void *handle, int version)
: Watchman_Plugin(handle, version), _fo(nullptr), _fe(nullptr)
{
}

int Test1_Plugin::init(Watchman *w, int argc, char **argv)
{
	int err;

	_alloc = w->alloc();

	_fo = _alloc->create<File>(STDOUT_FILENO);
	_fe = _alloc->create<File>(STDERR_FILENO);

	err = w->add_child(&_proc, &_buf, _fo, _fe, nullptr);
	if (unlikely(err)) {
		WATCHMAN_ERROR("Failed to add children to list: %d", err);
		return err;
	}

	return 0;
}

int Test1_Plugin::fini()
{
	_fo = _alloc->destroy<File>(_fo);
	_fe = _alloc->destroy<File>(_fe);

	return 0;
}

extern "C" Watchman_Plugin *entry(void *handle, Watchman *w)
{
	Allocator *alloc;

	alloc = w->alloc();

	return alloc->create<Test1_Plugin>(handle, 1);
}

