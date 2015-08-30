
#include "compiler.hxx"
#include "watchman.hxx"
#include "initfini.hxx"

int main(int argc, char **argv)
{
	Watchman w;
	int err;

	err = initialize(&w, argv[1]);
	if (unlikely(err))
		return err;

	err = w.loop();
	if (unlikely(err)) {
		finalize(&w);
		return err;
	}

	return finalize(&w);
}

