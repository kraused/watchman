
#include <dlfcn.h>

#include "initfini.hxx"
#include "compiler.hxx"
#include "error.hxx"
#include "watchman.hxx"
#include "plugin.hxx"

static int _load_plugin(Watchman *w, const char *plugin, int argc, char **argv)
{
	void *h;
	void *p;
	Watchman_Plugin *(*entry)();
	Watchman_Plugin *plu;

	dlerror();

	h = dlopen(plugin, RTLD_NOW | RTLD_LOCAL);
	if (unlikely(!h)) {
		WATCHMAN_ERROR("dlopen() failed: %s", dlerror());
		return -1;
	}

	p = dlsym(h, "entry");
	if (unlikely(!p)) {
		WATCHMAN_ERROR("The module does not provide the entry() function");
		/* Could call dlclose() at this point but since we are terminating
		 * anyway there seems to be little point in cleaning up.
		 */
		return -1;
	}

	entry = reinterpret_cast<Watchman_Plugin *(*)()>(p);

	plu = entry();
	if (unlikely(!plu)) {
		WATCHMAN_ERROR("entry() returned NULL");
		return -1;
	}

	if (1 != plu->version)
		WATCHMAN_WARN("Invalid plugin version %d", plu->version);

	return plu->init(w, argc, argv);
}

int initialize(Watchman *w, const char *plugin, int argc, char **argv)
{
	int err;

	if (!plugin) {
		WATCHMAN_ERROR("No plugin specified on the command line.");
		return -1;
	}

	err = w->init_signal_handling();
	if (unlikely(err))
		return err;

	err = _load_plugin(w, plugin, argc, argv);
	if (unlikely(err))
		return err;

	err = w->execute_children();
	if (unlikely(err))
		return err;

	return 0;
}

int finalize  (Watchman *w)
{
	int err;

	err = w->fini_signal_handling();
	if (unlikely(err))
		return err;

	return 0;
}

