
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#include "child.hxx"
#include "compiler.hxx"
#include "error.hxx"

Child::Child()
: _pid(-1), _fo(-1), _fe(-1)
{
}

int Child::terminate()
{
	return _send_signal_to_child(SIGTERM);
}

int Child::kill()
{
	return _send_signal_to_child(SIGKILL);
}

int Child::wait()
{
	int       status;
	long long x;

	x = ::waitpid(_pid, &status, 0);

	/* FIXME Evaluate status and handle errors returned by
	 *       wait()
	 */

	fprintf(stderr, "wait(): %d\n", status);

	return x;
}

int Child::_send_signal_to_child(int signum)
{
	int err;

	if (likely(-1 != _pid)) {
		err = ::kill(_pid, signum);
		if (unlikely(err))
			WATCHMAN_ERROR("kill() failed with errno %d: %s", errno, strerror(errno));
	} else {
		WATCHMAN_WARN("pid is invalid: %s() is a no-op", __func__);
		err = 0;
	}

	return err;
}

