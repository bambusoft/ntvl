Summary: NTVL Nat Traversal LAN & peer-to-peer virtual private network system.
Name: ntvl
Version: 1.0.0
Release: 1
License: GPLv3
Vendor: bambusoft
Group: None
URL: http://ntvl.bambusoft.mx
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
NTVL is Nat Traversal LAN and a peer-to-peer virtual private network system.
NTVL uses the universal TUNTAP interface to create TAP network interfaces to
an encrypted virtual LAN. Members of a community share encryption keys which
allow exchange of data. The supernode is used for peer discovery and initial
packet relay before direct peer-to-peer exchange is established.  Once direct
packet exchange is established, the supernode is not required.

NTVL-1 introduces additional security features and multiple supernodes.

%prep

%setup -q

echo -e "\n *** Building ${RPM_PACKAGE_NAME}-${RPM_PACKAGE_VERSION}-${RPM_PACKAGE_RELEASE} ***\n"

%build
make

%install
make PREFIX=${RPM_BUILD_ROOT}/usr install

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
     /usr/sbin/supernode
     /usr/sbin/node
%doc /usr/share/man/man1/supernode.1.gz
%doc /usr/share/man/man8/node.8.gz
%doc /usr/share/man/man7/ntvl-v1.0.0.gz


%changelog
* Mon Jun 25 2012 Mario Rodriguez -
- First release for ntvl-1.0
* Sun Jan 10 2012 Mario Rodriguez -
- First release for ntvl-0.9
* Fri Oct 30 2009 Richard Andrews -
- First beta for n2n-2
* Sat May  3 2008 Richard Andrews - 
- Initial build.

