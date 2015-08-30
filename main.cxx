
#include "compiler.hxx"
#include "watchman.hxx"
#include "initfini.hxx"
#include "error.hxx"

int main(int argc, char **argv)
{
	Watchman w;
	int err;

	err = initialize(&w, argv[1]);
	if (unlikely(err)) {
		WATCHMAN_ERROR("initialize() failed with exit code %d", err);
		return err;
	}

	err = w.loop();
	if (unlikely(err)) {
		finalize(&w);
		return err;
	}

	err = finalize(&w);
	if (unlikely(err)) {
		WATCHMAN_ERROR("finalize() failed with exit code %d", err);
	}

	return err;
}

