%define __provides_exclude_from ^%{_datadir}/%{name}/lib/.*$
%define __requires_exclude ^(libopenblas.*|libopenblas_d.*).*$

Name:       ru.kotdath.MLPackAurora
Summary:    Example of usage of mlpack for AuroraOS
Version:    0.1
Release:    1
License:    BSD-3-Clause
URL:        https://auroraos.ru
Source0:    %{name}-%{version}.tar.bz2

Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(auroraapp)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)

%description
Example of usage of mlpack for AuroraOS

%prep

%autosetup

%build

%cmake -GNinja
%ninja_build

%install

mkdir -p %{buildroot}/%{_datadir}/%{name}/lib/
cp %{_sourcedir}/../thirdparty/openblas/lib/*.so* %{buildroot}/%{_datadir}/%{name}/lib/
chmod 600 %{buildroot}/%{_datadir}/%{name}/lib/*.so*

echo "amo"
echo %{buildroot}/%{_infodir}

%ninja_install

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%defattr(644,root,root,-)
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
