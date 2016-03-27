
%define with_failfs	1

# We follow the Fedora naming scheme:
# https://fedoraproject.org/wiki/Packaging:NamingGuidelines?rd=Packaging/NamingGuidelines#NonNumericRelease
%define checkout	20160327gitd2e59e6

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

%if %{with_failfs}
%package failfs
Summary:	Watchman I/O manager failfs test filesystem
Group:		System Environment/Base
Requires:	fuse
BuildRequires:	fuse-devel

%description failfs
Failfs test filesystem
%endif

%prep
%setup -q -n %{name}-%{version}-%{checkout}

%build
make watchman.exe
%if %{with_failfs}
make tests/failfs/failfs.exe
%endif

%install
make PREFIX=%{buildroot} install
%if %{with_failfs}
install -m0755 tests/failfs/failfs.exe		%{buildroot}/usr/sbin/failfs.exe
install -m0755 tests/failfs/failfs_cmd_util.exe	%{buildroot}/usr/sbin/failfs_cmd_util.exe
%endif

%files
%defattr(-,root,root)
/usr/sbin/watchman.exe

%files devel
%defattr(-,root,root)
%dir %attr(0755,root,root)
%dir /usr/include/watchman
/usr/include/watchman/alloc.hxx
/usr/include/watchman/buffer.hxx
/usr/include/watchman/child.hxx
/usr/include/watchman/compiler.hxx
/usr/include/watchman/config.hxx
/usr/include/watchman/error.hxx
/usr/include/watchman/file.hxx
/usr/include/watchman/named_clingy_file.hxx
/usr/include/watchman/named_file.hxx
/usr/include/watchman/program.hxx
/usr/include/watchman/watchman.hxx

%if %{with_failfs}
%files failfs
%defattr(-,root,root)
/usr/sbin/failfs.exe
/usr/sbin/failfs_cmd_util.exe
%endif

