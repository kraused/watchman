
#include "plugin.hxx"
#include "watchman.hxx"
#include "compiler.hxx"
#include "error.hxx"

class Test1_Child : public Child
{

public:
	int		execute();

};

int Test1_Child::execute()
{
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

