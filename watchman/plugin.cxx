
#include "plugin.hxx"

Watchman_Plugin::Watchman_Plugin(void *handle, int version)
: _handle(handle), _version(version)
{
}

int Watchman_Plugin::version() const
{
	return _version;
}

void *Watchman_Plugin::handle()
{
	return _handle;
}

