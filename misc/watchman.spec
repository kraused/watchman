
# We follow the Fedora naming scheme:
# https://fedoraproject.org/wiki/Packaging:NamingGuidelines?rd=Packaging/NamingGuidelines#NonNumericRelease
%define checkout	20160327gitab2ae69

Name:		watchman
Version:	1.0
Release:	1.%{checkout}%{?dist}
Summary:	Watchman I/O manager
Group:		System Environment/Base
License:	GPLv2

Source:		%{name}-%{version}-%{checkout}.tar.gz

%description
This package provides the watchman resource manager. Watchman
offers failsafe I/O forwarding for processes that need to
reliably write to (potentially unreliable) clustered filesystems.

%package devel
Summary:	Watchman I/O manager development files
Group:		System Environment/Base
Requires:	watchman = %{version}

%description devel
Development headers for writing watchman plugins/wrapper for
applications.

%prep
%setup -q -n %{name}-%{version}-%{checkout}

%build
make watchman.exe

%install
install -D -m755 watchman.exe %buildroot/sbin/watchman.exe

%files
%defattr(-,root,root)
/sbin/watchman.exe

%files devel
%defattr(-,root,root)

