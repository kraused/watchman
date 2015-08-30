
#ifndef WATCHMAN_PLUGIN_HXX_INCLUDED
#define WATCHMAN_PLUGIN_HXX_INCLUDED 1

class Watchman;

/* Datastructure provided by DSO plugins to register
 * with the main application.
 */
struct Watchman_Plugin
{
	int	version;
	int	(*init)(Watchman *w);
};

#endif

