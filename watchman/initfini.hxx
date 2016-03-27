
#ifndef WATCHMAN_INITFINI_HXX_INCLUDED
#define WATCHMAN_INITFINI_HXX_INCLUDED 1

class Watchman;
class Watchman_Plugin;

/* Initialization and finalization routines. Dynamic memory
 * allocations are only allowed in initialize() and not going
 * further from this point.
 */

int initialize(Watchman *w, Watchman_Plugin **plu,
               const char *plugin, int argc, char **argv);
int finalize  (Watchman *w, Watchman_Plugin *plu);

#endif

