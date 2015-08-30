
#ifndef WATCHMAN_INITFINI_HXX_INCLUDED
#define WATCHMAN_INITFINI_HXX_INCLUDED 1

class Watchman;

/* Initialization and finalization routines. Dynamic memory
 * allocations are only allowed in initialize() and not going
 * further from this point.
 */

int initialize(Watchman *w, const char *plugin);
int finalize  (Watchman *w);

#endif

