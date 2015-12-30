
#ifndef WATCHMAN_PLUGIN_HXX_INCLUDED
#define WATCHMAN_PLUGIN_HXX_INCLUDED 1

class Watchman;

/* Datastructure provided by DSO plugins to register
 * with the main application.
 */
struct Watchman_Plugin
{
	int	version;
	/* Initialization function. In contrast to executable programs
	 * argv[0] is the first argument for the plugin.
	 */
	int	(*init)(Watchman *w, int argc, char **argv);
};

#endif

