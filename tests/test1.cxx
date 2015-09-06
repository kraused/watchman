
#include "plugin.hxx"
#include "child.hxx"
#include "watchman.hxx"
#include "compiler.hxx"
#include "error.hxx"

class Test1_Child : public Child
{

public:
	int		execute();
	int		terminate();
	int		kill();

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

static Watchman_Plugin plu;
static Test1_Child     proc;

int _init(Watchman *w)
{
	int err;

	err = w->add_child(&proc);
	if (unlikely(err)) {
		WATCHMAN_ERROR("Failed to add children to list: %d", err);
		return err;
	}

	return 0;
};

extern "C" Watchman_Plugin *entry()
{
	plu.version = 1;
	plu.init    = _init;

	return &plu;
};

