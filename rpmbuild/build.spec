# spec file for Sweetfish
#
# Copyright 2017 PG_MANA
#
# This software is Licensed under the Apache License Version 2.0
# See LICENSE.md
#

%define APP_NAME        sweetfish
%define APP_VERSION     0.0.3
%define APP_HOMEPAGE    https://soft.taprix.org/product/sweetfish.html
%define APP_LICENCE     Apache License, Version 2.0

Summary: Sweetfish for Linux
Summary(ja):Mastodon client for Linux
Name: %{APP_NAME}
Source0: %{APP_NAME}-%{APP_VERSION}.tar.gz
Version: %{APP_VERSION}
Release: 1
License: %{APP_LICENCE}
URL: %{APP_HOMEPAGE}
Group: Applications/Internet
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Prefix: %{_prefix}

#ビルド時に必要なもの
BuildRequires:cmake >= 3.1.0

%define INSTALLDIR %{buildroot}/usr/local/bin

%description
The Mastodon client for linux/X11.
This software uses qt.So it may be possible to work on macOS or Windows.

%prep
%setup -q

%build
%cmake -DCMAKE_CXX_FLAGS="-s" ..
make %{?_smp_mflags}

%install
mkdir -p %{buildroot}/usr/share/applications %{buildroot}/usr/share/pixmaps %{buildroot}/usr/lib/%{APP_NAME}/
cp src/Resources/icon/icon-normal.png %{buildroot}/usr/share/pixmaps/%{APP_NAME}.png
cp %{APP_NAME}.desktop %{buildroot}/usr/share/applications/%{APP_NAME}.desktop
cd build
cp -r locales %{buildroot}/usr/lib/%{APP_NAME}/
make install DESTDIR=%{buildroot}

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/bin/%{APP_NAME}
/usr/share/pixmaps/%{APP_NAME}.png
/usr/share/applications/%{APP_NAME}.desktop
/usr/lib/%{APP_NAME}/locales/*.qm
