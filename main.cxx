
#include "compiler.hxx"
#include "libc_alloc.hxx"
#include "watchman.hxx"
#include "plugin.hxx"
#include "initfini.hxx"
#include "error.hxx"

int main(int argc, char **argv)
{
	Libc_Allocator alloc;
	Watchman w(&alloc);
	Watchman_Plugin *plu;
	int err;
	int tmp;

	err = initialize(&w, &plu, argv[1], argc - 2, &argv[2]);
	if (unlikely(err)) {
		WATCHMAN_ERROR("initialize() failed with exit code %d", err);
		return err;
	}

	err = w.loop();

	tmp = finalize(&w, plu);
	if (unlikely(tmp)) {
		WATCHMAN_ERROR("finalize() failed with exit code %d", tmp);
		if (0 == err) {
			err = tmp;
		}
	}

	return err;
}

