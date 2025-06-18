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
BuildRequires: cmake
BuildRequires: ninja
BuildRequires: conan

%description
Example of usage of mlpack for AuroraOS

%prep

%autosetup

%build

CONAN_LIB_DIR="%{_builddir}/conan-libs/"
%{set_build_flags}
conan-install-if-modified --source-folder="%{_sourcedir}/.." --output-folder="$CONAN_LIB_DIR" -vwarning
PKG_CONFIG_PATH="$CONAN_LIB_DIR":$PKG_CONFIG_PATH
export PKG_CONFIG_PATH
%cmake -GNinja
%ninja_build


%install
%ninja_install

CONAN_LIB_DIR="%{_builddir}/conan-libs/"
SHARED_LIBRARIES="%{buildroot}/%{_datadir}/%{name}/lib"
mkdir -p "$SHARED_LIBRARIES"
conan-deploy-libraries "%{buildroot}/%{_bindir}/%{name}" "$CONAN_LIB_DIR" "$SHARED_LIBRARIES"
cp -rf %{_libdir}/libgomp* "$SHARED_LIBRARIES"

%define __provides_exclude_from ^%{_datadir}/%{name}/lib/.*$
%define __requires_exclude ^(libopenblas.*|libgomp.*|libz.*|libhdf5.*|libarmadillo.*)$

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%defattr(644,root,root,-)
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
