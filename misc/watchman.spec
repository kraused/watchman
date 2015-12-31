
Name:		watchman
Version:	0.1
Release:	1
Summary:	Watchman I/O manager
Group:		System Environment/Base
License:	GPLv2

Source:		%{name}-%{version}.tar.gz

%description
This package provides the watchman resource manager. Watchman
offers failsafe I/O forwarding for processes that need to
reliably write to (potentially unreliable) clustered filesystems.

%prep
%setup -q -n %{name}-%{version}

%install
install -D -m755 watchman.exe %buildroot/sbin/watchman.exe

%files
/sbin/watchman.exe

%changelog

