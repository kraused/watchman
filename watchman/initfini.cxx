
#include <dlfcn.h>

#include "initfini.hxx"
#include "compiler.hxx"
#include "error.hxx"
#include "watchman.hxx"
#include "alloc.hxx"
#include "plugin.hxx"

static int _load_plugin(Watchman *w, Watchman_Plugin **plu, const char *plugin, int argc, char **argv)
{
	void *h;
	void *p;
	int err;

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

	*plu = reinterpret_cast<Watchman_Plugin_Entry>(p)(h, w);
	if (unlikely(!(*plu))) {
		WATCHMAN_ERROR("entry() returned nullptr");
		return -1;
	}

	if (1 != (*plu)->version()) {
		WATCHMAN_WARN("Invalid plugin version %d", (*plu)->version());
	}

	err = (*plu)->init(w, argc, argv);
	if (unlikely(err)) {
		WATCHMAN_ERROR("Plugin init() function returned %d", err);
		return err;
	}

	return 0;
}

static int _unload_plugin(Watchman *w, Watchman_Plugin *plu)
{
	void *h;
	int err;

	err = plu->fini();
	if (unlikely(err)) {
		WATCHMAN_ERROR("Plugin fini() function returned %d", err);
		return err;
	}

	h = plu->handle();

	/* TODO We may need a virtual destructor for Watchman_Plugin.
	 */
	w->alloc()->destroy(plu);

	dlclose(h);

	return 0;
}

int initialize(Watchman *w, Watchman_Plugin **plu,
               const char *plugin, int argc, char **argv)
{
	int err;

	if (!plugin) {
		WATCHMAN_ERROR("No plugin specified on the command line.");
		return -1;
	}

	err = w->init_signal_handling();
	if (unlikely(err))
		return err;

	err = _load_plugin(w, plu, plugin, argc, argv);
	if (unlikely(err))
		return err;

	err = w->execute_children();
	if (unlikely(err))
		return err;

	return 0;
}

int finalize(Watchman *w, Watchman_Plugin *plu)
{
	int err;

	err = w->fini_signal_handling();
	if (unlikely(err))
		return err;

	err = _unload_plugin(w, plu);
	if (unlikely(err))
		return err;

	return 0;
}

