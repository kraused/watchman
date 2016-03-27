
#ifndef WATCHMAN_PLUGIN_HXX_INCLUDED
#define WATCHMAN_PLUGIN_HXX_INCLUDED 1

class Watchman;

/* Datastructure provided by DSO plugins to register
 * with the main application.
 */
class Watchman_Plugin
{

public:
			Watchman_Plugin(void *handle, int version);

public:
			/* Initialization function. In contrast to executable programs
			 * argv[0] is the first argument for the plugin.
			 */
	virtual int	init(Watchman *w, int argc, char **argv) = 0;
	virtual int	fini() = 0;

public:
			/* Return the DSO handle. Use dlclose() on this pointer to
			 * unload the shared library.
			 */
	void		*handle();

private:
	void		*_handle;

public:
	int		version() const;

private:
	int		_version;

};

extern "C" typedef Watchman_Plugin *(*Watchman_Plugin_Entry)(void *handle, Watchman *w);

#endif

