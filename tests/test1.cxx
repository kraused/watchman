
#include "plugin.hxx"
#include "watchman.hxx"
#include "compiler.hxx"
#include "error.hxx"

class Test1_Child : Child
{

};

int _init(Watchman *w)
{
	return -1;
};

static Watchman_Plugin plu;

extern "C" Watchman_Plugin *entry()
{
	plu.version = 1;
	plu.init    = _init;

	return &plu;
};

