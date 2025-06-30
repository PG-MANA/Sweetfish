# spec file for Sweetfish
#
# Copyright 2017 PG_MANA
#
# This software is Licensed under the Apache License Version 2.0
# See LICENSE.md
#

%define APP_NAME        sweetfish
%define APP_VERSION     0.8.0
%define APP_HOMEPAGE    https://soft.taprix.org/product/sweetfish.html
%define APP_LICENCE     Apache License, Version 2.0

Summary:Mastodon client with Qt
Name: %{APP_NAME}
Source0: %{APP_NAME}-%{APP_VERSION}.tar.gz
Version: %{APP_VERSION}
Release: 1
License: %{APP_LICENCE}
URL: %{APP_HOMEPAGE}
Group: Applications/Internet
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Prefix: %{_prefix}

BuildRequires:cmake >= 3.28.0

%define INSTALLDIR %{buildroot}/usr/local/bin

%description
The Mastodon client with Qt.

%prep
%setup -q

%build
%cmake ..
make %{?_smp_mflags}

%install
cd build
make install DESTDIR=%{buildroot}

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/bin/%{APP_NAME}
/usr/share/pixmaps/%{APP_NAME}.png
/usr/share/applications/%{APP_NAME}.desktop
/usr/share/%{APP_NAME}/translations/*.qm
